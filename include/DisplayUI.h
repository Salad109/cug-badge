#ifndef DISPLAY_UI_H
#define DISPLAY_UI_H

#include <Arduino.h>

/** Mirrors backend MessageRequest.CategoryEnum (LoRa / admin messaging). */
enum class MessageCategory : uint8_t {
  INFO = 0,
  WARNING,
  SPONSOR,
  ALERT,
  AGENDA_UPDATE,
  AGENDA_SYNC,
};

/** Inbox row; maps from backend push payload after parsing. */
struct BadgeMessage {
  uint32_t id;
  const char* sender;
  const char* receivedAt;
  const char* content;
  MessageCategory category;
};

struct ScheduleEvent {
  const char* title;
  const char* time;
};

enum MenuIndex : int {
  MENU_PROFILE = 0,
  MENU_NFC = 1,
  MENU_MESSAGES = 2,
  MENU_GAMES = 3,
  MENU_SCHEDULE = 4,
  MENU_SETTINGS = 5,
};

void initDisplay();
void drawSplashScreen();
void drawMainScreen(String fullName, String role, String company, String email, String phone,
                    String qrUrl, int selectedMenu = 0, const ScheduleEvent* scheduleEvents = nullptr,
                    int scheduleEventCount = 0, int scheduleScrollOffset = 0,
                    const BadgeMessage* messages = nullptr, int messageCount = 0,
                    int selectedMessage = 0, int messageScrollOffset = 0, bool messageDetailOpen = false);

void drawTopBar();
void drawBottomBar();
void drawProfile(String fullName, String role, String company, String email, String phone,
                 String qrUrl);
void drawMenu(int selectedMenu);
void drawSchedule(const ScheduleEvent* events, int eventCount, int scrollOffset = 0);
void drawMessagesList(const BadgeMessage* messages, int messageCount, int selectedMessage,
                      int scrollOffset);
void drawMessageDetail(const BadgeMessage* message);
void drawRegistrationScreen(String macAddress);

#endif
