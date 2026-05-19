#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <LoRa.h>
#include <ESPAsyncWebServer.h>
#include "secrets.h"

// SX1276 wiring — matches the badge BOM pinout (shared SPI bus).
// Change if the AP board is wired differently.
constexpr int  LORA_SCK  = 4;
constexpr int  LORA_MISO = 6;
constexpr int  LORA_MOSI = 7;
constexpr int  LORA_CS   = 3;
constexpr int  LORA_RST  = -1;   // soft reset via SPI
constexpr int  LORA_DIO0 = 8;    // shares the C3 Super Mini onboard LED — input only, fine
constexpr long LORA_FREQ = 868E6;

constexpr size_t MAX_LORA_PAYLOAD = 240;

struct RxPacket {
  uint32_t at_ms;
  int16_t  rssi;
  float    snr;
  uint8_t  len;
  char     data[MAX_LORA_PAYLOAD];
};

QueueHandle_t rxQueue;

constexpr size_t RX_LOG_SIZE = 16;
RxPacket rxLog[RX_LOG_SIZE];
size_t   rxLogHead  = 0;
size_t   rxLogCount = 0;

bool     loraReady = false;
uint32_t rxTotal   = 0;
uint32_t txTotal   = 0;

AsyncWebServer server(80);

void onLoRaReceive(int packetSize) {
  if (packetSize <= 0 || packetSize > (int)MAX_LORA_PAYLOAD) return;
  RxPacket p{};
  p.at_ms = millis();
  p.rssi  = LoRa.packetRssi();
  p.snr   = LoRa.packetSnr();
  p.len   = packetSize;
  for (int i = 0; i < packetSize; i++) p.data[i] = (char)LoRa.read();
  xQueueSend(rxQueue, &p, 0);
}

// Extracts a JSON string value: returns the substring between the quotes
// after `"key":"`. Returns empty String if not found. No escape handling —
// fine for the backend's MessageRequest shape (ASCII content, no quotes inside).
String jsonString(const String& body, const char* key) {
  String needle = String("\"") + key + "\"";
  int k = body.indexOf(needle);
  if (k < 0) return String();
  int colon = body.indexOf(':', k);
  if (colon < 0) return String();
  int q1 = body.indexOf('"', colon);
  if (q1 < 0) return String();
  int q2 = body.indexOf('"', q1 + 1);
  if (q2 < 0) return String();
  return body.substring(q1 + 1, q2);
}

// Accumulates POST body chunks into a heap-allocated String stashed on the
// request. The main handler reads it back and deletes it.
void bodyAccumulator(AsyncWebServerRequest* req, uint8_t* data, size_t len,
                     size_t index, size_t total) {
  if (index == 0) {
    auto* s = new String();
    s->reserve(total);
    req->_tempObject = s;
  }
  auto* s = static_cast<String*>(req->_tempObject);
  if (!s) return;
  for (size_t i = 0; i < len; i++) (*s) += (char)data[i];
}

String takeBody(AsyncWebServerRequest* req) {
  auto* s = static_cast<String*>(req->_tempObject);
  String out = s ? *s : String();
  delete s;
  req->_tempObject = nullptr;
  return out;
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n=== CUG Badge AP ===");

  rxQueue = xQueueCreate(8, sizeof(RxPacket));

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.printf("wifi: connecting to %s", WIFI_SSID);
  for (int i = 0; i < 40 && WiFi.status() != WL_CONNECTED; i++) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("wifi: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("mac:  %s\n", WiFi.macAddress().c_str());
    Serial.printf("register this AP as a sector with gatewayUrl=http://%s/downlink\n",
                  WiFi.localIP().toString().c_str());
  } else {
    Serial.println("wifi: not connected (will keep trying in background)");
  }

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);
  if (LoRa.begin(LORA_FREQ)) {
    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(5);
    LoRa.enableCrc();
    LoRa.onReceive(onLoRaReceive);
    LoRa.receive();
    loraReady = true;
    Serial.println("lora: ok @ 868 MHz, SF7/BW125");
  } else {
    Serial.println("lora: init FAILED (no SX1276 attached?)");
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
    String s;
    s.reserve(2048);
    s += "CUG Badge AP\n";
    s += "wifi: ";
    s += (WiFi.status() == WL_CONNECTED ? "OK " : "no ");
    s += WiFi.localIP().toString() + "\n";
    s += "lora: ";
    s += (loraReady ? "OK 868MHz SF7/BW125" : "FAIL");
    s += "\n";
    s += "rx total: " + String(rxTotal) + "\n";
    s += "tx total: " + String(txTotal) + "\n";
    s += "\nrecent rx:\n";
    for (size_t i = 0; i < rxLogCount; i++) {
      size_t idx = (rxLogHead + RX_LOG_SIZE - rxLogCount + i) % RX_LOG_SIZE;
      const RxPacket& p = rxLog[idx];
      s += "  [" + String(p.at_ms) + "] rssi=" + String(p.rssi)
         + " snr=" + String(p.snr, 1) + " : ";
      for (uint8_t j = 0; j < p.len; j++) {
        char c = p.data[j];
        s += (c >= 0x20 && (uint8_t)c < 0x7f) ? c : '.';
      }
      s += "\n";
    }
    s += "\nPOST /tx (body = payload, max 240 bytes) to transmit.\n";
    req->send(200, "text/plain", s);
  });

  // POST /downlink — Spring backend's MessagingService forwards MessageRequest here.
  // Body: {"targetType":"BROADCAST|SECTOR|MAC","targetId":"...","content":"...","category":"INFO|..."}
  // We transmit a single text LoRa frame: "<targetType>|<targetId>|<category>|<content>"
  // (badges filter by targetType/targetId themselves).
  server.on("/downlink", HTTP_POST,
    [](AsyncWebServerRequest* req) {
      String body = takeBody(req);
      Serial.printf("[dl] hit. %u bytes body\n", body.length());
      if (body.length() == 0) { req->send(400, "text/plain", "empty body\n"); return; }

      String targetType = jsonString(body, "targetType");
      String targetId   = jsonString(body, "targetId");
      String content    = jsonString(body, "content");
      String category   = jsonString(body, "category");
      if (targetType.length() == 0 || content.length() == 0) {
        req->send(400, "text/plain", "missing targetType or content\n");
        return;
      }
      if (category.length() == 0) category = "INFO";
      if (targetId.length()  == 0) targetId  = "-";

      String frame = targetType + "|" + targetId + "|" + category + "|" + content;
      Serial.printf("[dl] frame (%u bytes): %s\n", frame.length(), frame.c_str());
      if (frame.length() > MAX_LORA_PAYLOAD) {
        req->send(413, "text/plain", "frame > 240 bytes after framing\n");
        return;
      }
      if (!loraReady) {
        req->send(503, "text/plain", "LoRa not initialized (frame parsed OK)\n");
        return;
      }
      LoRa.beginPacket();
      LoRa.write((const uint8_t*)frame.c_str(), frame.length());
      LoRa.endPacket();
      LoRa.receive();
      txTotal++;
      Serial.printf("[dl] transmitted\n");
      req->send(202, "text/plain", "accepted\n");
    },
    nullptr,
    bodyAccumulator);

  // POST /tx — raw passthrough for manual testing (curl etc.). Body becomes the
  // LoRa payload verbatim, no framing.
  server.on("/tx", HTTP_POST,
    [](AsyncWebServerRequest* req) {
      String body = takeBody(req);
      if (body.length() == 0 || body.length() > MAX_LORA_PAYLOAD) {
        req->send(400, "text/plain", "payload must be 1..240 bytes\n");
        return;
      }
      if (!loraReady) {
        req->send(503, "text/plain", "LoRa not initialized\n");
        return;
      }
      LoRa.beginPacket();
      LoRa.write((const uint8_t*)body.c_str(), body.length());
      LoRa.endPacket();
      LoRa.receive();
      txTotal++;
      Serial.printf("[tx] %u bytes: %s\n", body.length(), body.c_str());
      req->send(200, "text/plain", String("sent ") + body.length() + " bytes\n");
    },
    nullptr,
    bodyAccumulator);

  server.begin();
  Serial.println("http: listening on :80");
}

void loop() {
  RxPacket p;
  while (xQueueReceive(rxQueue, &p, 0) == pdTRUE) {
    rxTotal++;
    rxLog[rxLogHead] = p;
    rxLogHead = (rxLogHead + 1) % RX_LOG_SIZE;
    if (rxLogCount < RX_LOG_SIZE) rxLogCount++;

    Serial.printf("[rx] %u bytes rssi=%d snr=%.1f : ", p.len, p.rssi, p.snr);
    for (uint8_t i = 0; i < p.len; i++) {
      char c = p.data[i];
      Serial.print((c >= 0x20 && (uint8_t)c < 0x7f) ? c : '.');
    }
    Serial.println();
  }
  delay(10);
}
