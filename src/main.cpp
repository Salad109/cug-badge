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

static const BadgeMessage kDemoMessages[] = {
    {1, "Organizer", "09:15", "Welcome to CUG Badge demo. Please proceed to the main hall.",
     MessageCategory::INFO},
    {2, "Sponsor", "10:42", "Visit our booth for networking and small gifts while supplies last.",
     MessageCategory::SPONSOR},
    {3, "System", "11:05", "Your schedule was updated. Open Schedule to see new workshop times.",
     MessageCategory::AGENDA_UPDATE},
    {4, "Staff", "12:20", "Lunch vouchers are available at the info desk until 13:30 today.",
     MessageCategory::WARNING},
};
static const int kDemoMessageCount = sizeof(kDemoMessages) / sizeof(kDemoMessages[0]);

static int gSelectedMessage = 0;
static bool gMessageDetailOpen = false;

static void drawDemoMain(int selectedMenu) {
  drawMainScreen("John Doe", "Chief Engineer", "Comarch SA", "john.doe@comarch.com",
                 "+48 123 456 789", "https://www.linkedin.com/company/comarch/", selectedMenu,
                 kDemoSchedule, kDemoScheduleCount, 0, kDemoMessages, kDemoMessageCount,
                 gSelectedMessage, 0, gMessageDetailOpen);
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
  gSelectedMessage = 0;
  gMessageDetailOpen = false;
  drawDemoMain(MENU_PROFILE);
  delay(2000);
}

/**
 * Demo: menu opens only to pick a tab (SELECT closes it). Content uses the full screen.
 */
void loop() {
  int selectedMenu = MENU_PROFILE;

  // MENU: open overlay, scroll to Messages, SELECT closes menu.
  isMenuVisible = true;
  drawDemoMain(selectedMenu);
  delay(800);

  selectedMenu = MENU_MESSAGES;
  drawDemoMain(selectedMenu);
  delay(800);

  isMenuVisible = false;
  gSelectedMessage = 0;
  gMessageDetailOpen = false;
  drawDemoMain(selectedMenu);
  delay(1000);

  // Browse inbox (full screen, menu hidden).
  for (int m = 1; m < kDemoMessageCount; m++) {
    gSelectedMessage = m;
    drawDemoMain(selectedMenu);
    delay(750);
  }

  gMessageDetailOpen = true;
  drawDemoMain(selectedMenu);
  delay(3500);

  gMessageDetailOpen = false;
  drawDemoMain(selectedMenu);
  delay(2000);

  // MENU: pick Schedule, SELECT → full-screen schedule.
  isMenuVisible = true;
  selectedMenu = MENU_SCHEDULE;
  drawDemoMain(selectedMenu);
  delay(800);

  isMenuVisible = false;
  drawDemoMain(selectedMenu);
  delay(3000);

  // MENU: scroll up to Profile, SELECT → full-screen profile.
  isMenuVisible = true;
  for (int i = MENU_SCHEDULE - 1; i >= MENU_PROFILE; i--) {
    selectedMenu = i;
    drawDemoMain(selectedMenu);
    delay(750);
  }

  isMenuVisible = false;
  drawDemoMain(selectedMenu);
  delay(3000);
}
