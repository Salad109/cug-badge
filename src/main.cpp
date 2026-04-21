#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// Definiujemy TYLKO dwa piny, tak jak w dzialajacym przykladzie z Uno!
#define TFT_CS 7
#define TFT_DC 2

// Tworzymy obiekt ekranu BEZ pinu resetu (biblioteka zresetuje go softwarowo)
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

void setup() {
  Serial.begin(115200);

  // Mówimy ESP32, pod jakimi pinami są kable magistrali SPI (Zgodnie z naszym diagramem)
  SPI.begin(4, 5, 6, 7);

  // Uruchamiamy ekran
  tft.begin();

  // Ustawienia obrazu
  tft.setRotation(1); 
  tft.fillScreen(ILI9341_BLACK);

  // Twój napis
  tft.setCursor(60, 100);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(4);
  tft.println("COMARCH");
  
  Serial.println("Gotowe!");
}

void loop() { }