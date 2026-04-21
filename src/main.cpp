#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "secrets.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

const int LED = 8;
bool ledState = false;

WebServer server(80);

void handleRoot() {
  String state = ledState ? "ON" : "OFF";
  String page =
    "<!DOCTYPE html><html><head>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<style>"
    "body{font-family:sans-serif;display:flex;flex-direction:column;align-items:center;justify-content:center;height:100vh;margin:0;background:#1a1a1a;color:#eee}"
    "h1{font-size:2rem;margin-bottom:1rem}"
    "button{padding:1rem 2.5rem;font-size:1.2rem;border:none;border-radius:8px;cursor:pointer;background:" + String(ledState ? "#4caf50" : "#555") + ";color:#fff}"
    "</style></head><body>"
    "<h1>LED is " + state + "</h1>"
    "<form action='/toggle' method='POST'><button>Toggle</button></form>"
    "</body></html>";
  server.send(200, "text/html", page);
}

void handleToggle() {
  ledState = !ledState;
  Serial.println("LED toggled " + String(ledState ? "ON" : "OFF") + " by " + server.client().remoteIP().toString());
  digitalWrite(LED, ledState ? LOW : HIGH);
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH); // LED off on boot (active-low)

  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nIP: " + WiFi.localIP().toString());

  server.on("/", handleRoot);
  server.on("/toggle", HTTP_POST, handleToggle);
  server.begin();
}

void loop() {
  server.handleClient();
}
