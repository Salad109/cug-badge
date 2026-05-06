#include <Arduino.h>
#include "DisplayUI.h"
#include "Config.h" 

void setup() {
  Serial.begin(115200);
  
  initDisplay();
  
  drawSplashScreen();
  delay(2000);

  drawMainScreen("John Doe", "Chief Engineer", "Comarch SA", "john.doe@comarch.com", "+48 123 456 789", "https://www.linkedin.com/company/comarch/");
}

void loop() {
}