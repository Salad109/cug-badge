#include <Arduino.h>
#include "DisplayUI.h"
#include "Config.h"
#include <WiFi.h>

extern bool isMenuVisible;

static const ScheduleEvent kDemoSchedule[] = {
    {"Opening Ceremony", "09:00-09:30"},
    {"Keynote Speech", "09:30-10:30"},
    {"Coffee Break", "10:30-11:00"},
    {"Workshop A", "11:00-12:30"},
    {"Lunch", "12:30-13:30"},
    {"Workshop B", "13:30-15:00"},
    {"Panel Discussion", "15:00-16:00"},
    {"Closing Remarks", "16:00-16:30"},
};
static const int kDemoScheduleCount = sizeof(kDemoSchedule) / sizeof(kDemoSchedule[0]);

static void drawDemoMain(int selectedMenu) {
  drawMainScreen("John Doe", "Chief Engineer", "Comarch SA", "john.doe@comarch.com",
                 "+48 123 456 789", "https://www.linkedin.com/company/comarch/", selectedMenu,
                 kDemoSchedule, kDemoScheduleCount);
}

/**
 * Boot sequence: splash → device registration (MAC + QR) → default profile screen.
 */
void setup() {
  Serial.begin(115200);

  initDisplay();

  drawSplashScreen();
  delay(2000);

  String macAddress = WiFi.macAddress();
  drawRegistrationScreen(macAddress);
  delay(5000);

  isMenuVisible = false;
  drawDemoMain(0);
  delay(2000);
}

/**
 * Automated menu demo for display testing (no physical buttons yet).
 * Cycles: open menu → scroll down → confirm Schedule → reopen → scroll up → confirm Profile.
 */
void loop() {
  int selectedMenu = 0;
  const int menuCount = 6;

  // Open side menu on Profile.
  isMenuVisible = true;
  drawDemoMain(selectedMenu);
  delay(1000);

  // Scroll selection down through all menu items (ends on Schedule).
  for (int i = 1; i < menuCount; i++) {
    selectedMenu = i;
    drawDemoMain(selectedMenu);
    delay(750);
  }

  // Close menu and show Schedule view.
  isMenuVisible = false;
  drawDemoMain(selectedMenu);
  delay(3000);

  // Open menu again while still on Schedule.
  isMenuVisible = true;
  drawDemoMain(selectedMenu);
  delay(1000);

  // Scroll back up to Profile.
  for (int i = menuCount - 2; i >= 0; i--) {
    selectedMenu = i;
    drawDemoMain(selectedMenu);
    delay(750);
  }

  // Close menu and return to Profile view.
  isMenuVisible = false;
  drawDemoMain(selectedMenu);
  delay(3000);
}
