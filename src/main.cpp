#include <Arduino.h>
#include "DisplayUI.h"
#include "Config.h" 

extern bool isMenuVisible;

void setup() {
  Serial.begin(115200);
  
  initDisplay();
  
  drawSplashScreen();
  delay(2000);

  isMenuVisible = false;
  drawMainScreen("John Doe", "Chief Engineer", "Comarch SA", "john.doe@comarch.com", "+48 123 456 789", "https://www.linkedin.com/company/comarch/", 0);
  delay(2000);
}

void loop() {
  int selectedMenu = 0;
  const int menuCount = 6;

  // Otwarcie menu
  isMenuVisible = true;
  drawMainScreen("John Doe", "Chief Engineer", "Comarch SA", "john.doe@comarch.com", "+48 123 456 789", "https://www.linkedin.com/company/comarch/", selectedMenu);
  delay(1000);

  // Przewijanie w dół do zakładki "Schedule"
  for (int i = 1; i < menuCount; i++) {
    selectedMenu = i;
    drawMainScreen("John Doe", "Chief Engineer", "Comarch SA", "john.doe@comarch.com", "+48 123 456 789", "https://www.linkedin.com/company/comarch/", selectedMenu);
    delay(750);
  }

  // Wybranie "Schedule"
  isMenuVisible = false;
  drawMainScreen("John Doe", "Chief Engineer", "Comarch SA", "john.doe@comarch.com", "+48 123 456 789", "https://www.linkedin.com/company/comarch/", selectedMenu);
  delay(3000);

  // Ponowne otwarcie menu
  isMenuVisible = true;
  drawMainScreen("John Doe", "Chief Engineer", "Comarch SA", "john.doe@comarch.com", "+48 123 456 789", "https://www.linkedin.com/company/comarch/", selectedMenu);
  delay(1000);

  // Przewijanie w górę z powrotem do "Profile"
  for (int i = menuCount - 2; i >= 0; i--) {
    selectedMenu = i;
    drawMainScreen("John Doe", "Chief Engineer", "Comarch SA", "john.doe@comarch.com", "+48 123 456 789", "https://www.linkedin.com/company/comarch/", selectedMenu);
    delay(750);
  }
  
  // Wybranie "Profile" - menu znika
  isMenuVisible = false;
  drawMainScreen("John Doe", "Chief Engineer", "Comarch SA", "john.doe@comarch.com", "+48 123 456 789", "https://www.linkedin.com/company/comarch/", selectedMenu);
  delay(3000);
}