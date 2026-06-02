#ifndef DISPLAY_UI_H
#define DISPLAY_UI_H

#include <Arduino.h>

struct ScheduleEvent {
  const char* title;
  const char* time;
};

void initDisplay();
void drawSplashScreen();
void drawMainScreen(String fullName, String role, String company, String email, String phone,
                    String qrUrl, int selectedMenu = 0, const ScheduleEvent* scheduleEvents = nullptr,
                    int scheduleEventCount = 0, int scheduleScrollOffset = 0);

void drawTopBar();
void drawBottomBar();
void drawProfile(String fullName, String role, String company, String email, String phone,
                 String qrUrl);
void drawMenu(int selectedMenu);
void drawSchedule(const ScheduleEvent* events, int eventCount, int scrollOffset = 0);
void drawRegistrationScreen(String macAddress);

#endif
