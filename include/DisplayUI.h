#ifndef DISPLAY_UI_H
#define DISPLAY_UI_H

#include <Arduino.h>

void initDisplay();
void drawSplashScreen();
void drawMainScreen(String fullName, String role, String company, String email, String phone, String qrUrl, int selectedMenu = 0);

void drawTopBar();
void drawBottomBar();
void drawProfile(String fullName, String role, String company, String email, String phone, String qrUrl);
void drawMenu(int selectedMenu);

#endif