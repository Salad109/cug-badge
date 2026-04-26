#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

#define TEST_PIN 8
#define BOOT_BUTTON_PIN 9 // Standardowy pin przycisku BOOT w ESP32-C3

// Adres rozgloszeniowy - wysyla do wszystkich w zasiegu
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Struktura danych do komunikacji
typedef struct struct_message {
    char msg[32];
    int val;
    uint32_t timeStamp;
} struct_message;

// Dane do wyslania
struct_message myData;       
// Dane odebrane
struct_message incomingData; 

esp_now_peer_info_t peerInfo;
// Flaga do mrugania dioda poza callbackiem
volatile bool shouldBlink = false;

// Callback wywolywany przy wysylaniu
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nStatus wysylki: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sukces" : "Blad");
}

// Callback wywolywany przy odbieraniu
void OnDataRecv(const uint8_t * mac, const uint8_t *incoming, int len) {
  // Sprawdzamy, czy ten peer jest juz na liscie, jesli nie - dodajemy go
  if (!esp_now_is_peer_exist(mac)) {
    Serial.println("Wykryto nowe urzadzenie! Dodaje do listy partnerow...");
    esp_now_peer_info_t newPeer;
    memcpy(newPeer.peer_addr, mac, 6);
    newPeer.channel = 0; // Uzyj aktualnego kanalu
    newPeer.encrypt = false;

    if (esp_now_add_peer(&newPeer) != ESP_OK) {
      Serial.println("Nie udalo sie dodac nowego partnera");
    } else {
      // Po dodaniu mozemy od teraz wysylac bezposrednio do tego MAC
      Serial.printf("Dodano: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
  }

  memcpy(&incomingData, incoming, sizeof(incomingData));
  Serial.print("\n--- ODEBRANO DANE ---");
  Serial.print("\r\nWiadomosc: "); Serial.println(incomingData.msg);
  Serial.print("Wartosc: "); Serial.println(incomingData.val);
  
  // Ustawienie flagi mrugania zamiast uzywania delay()
  shouldBlink = true;
}

void sendData() {
  // Przygotowanie danych do wysylki
  strcpy(myData.msg, "Wyzwalanie reczne!");
  myData.val = random(0, 100);
  myData.timeStamp = millis();

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  if (result == ESP_OK) Serial.println("Wyslano zadanie.");
}

void setup() {
  Serial.begin(115200);
  
  unsigned long start = millis();
  while (!Serial && (millis() - start < 5000));

  Serial.println("\n--- ESP-NOW TRANSCEIVER START ---");

  pinMode(TEST_PIN, OUTPUT);
  digitalWrite(TEST_PIN, LOW); // Dioda wyłączona od początku (stan niski)
  
  // Ustawienie Wi-Fi w trybie Station (wymagane dla ESP-NOW)
  WiFi.mode(WIFI_STA);
  Serial.print("Adres MAC tego urzadzenia: ");
  Serial.println(WiFi.macAddress());

  // Konfiguracja przycisku BOOT
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

  // Inicjalizacja ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Blad inicjalizacji ESP-NOW");
    return;
  }

  // Rejestracja funkcji zwrotnych
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  
  // Rejestracja partnera (peera)
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Nie udalo sie dodac partnera");
    return;
  }

  Serial.println("System gotowy. Przycisk BOOT lub Serial wyzwala wysylke.");
}

void loop() {
  // Obsluga mrugania dioda (bezpieczna dla systemu)
  if (shouldBlink) {
    digitalWrite(TEST_PIN, !digitalRead(TEST_PIN)); // Przełącz stan: jeśli LOW -> HIGH, jeśli HIGH -> LOW
    shouldBlink = false;
  }

  // 1. Sprawdzanie przycisku fizycznego (BOOT)
  if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
    Serial.println("Przycisk BOOT nacisniety - Wysylam...");
    sendData();
    delay(250); // Prosty debouncing bez blokowania petli while
  }

  // 2. Sprawdzanie komendy z Serial Monitora
  if (Serial.available() > 0) {
    char c = Serial.read();
    while(Serial.available() > 0) Serial.read(); // Wyczysc bufor

    if (c == 's' || c == 'S') {
      Serial.println("\n[SYMULACJA] Generowanie sztucznego pakietu przychodzacego...");
      struct_message testData;
      strcpy(testData.msg, "Test lokalny!");
      testData.val = 123;
      uint8_t fakeMac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02};
      
      // Reczne wywolanie funkcji odbioru
      OnDataRecv(fakeMac, (uint8_t *)&testData, sizeof(testData));
    } else {
      Serial.println("Odebrano sygnal z Seriala - Wysylam realny pakiet...");
      sendData();
    }
  }
}