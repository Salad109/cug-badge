#include "DisplayUI.h"
#include "Config.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <qrcode.h>

// Wygenerowane pliki z czcionkami
#include "Roboto_Condensed_Regular4pt7b.h"
#include "Roboto_Condensed_Regular8pt7b.h"
#include "Roboto_Condensed_Regular12pt7b.h"
#include "Roboto_Condensed_Regular16pt7b.h"
#include "Roboto_Condensed_Regular20pt7b.h"
#include "Roboto_Condensed_Regular24pt7b.h"
#include "logo.h"
#include "logo_small.h"

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

bool isMenuVisible = false;

struct ScheduleEvent {
  const char* title;
  const char* time;
};

void drawSchedule(int scrollOffset);
void drawQRCode(const char* url, int offset_x, int offset_y, int scale);
void printTruncated(String text, uint16_t maxWidth);

void initDisplay() {
  SPI.begin(TFT_CLK, -1, TFT_MOSI, -1);
  tft.begin();
  tft.setRotation(1);
}

void drawSplashScreen() {
  tft.fillScreen(COMARCH_NAVY); 
  // Rysowanie loga z przezroczystym tłem i korekcją antyaliasingu
  for(int y = 0; y < 31; y++) {
    for(int x = 0; x < 200; x++) {
      uint16_t color = pgm_read_word(&CMR_WA_BIG_565[y * 200 + x]);
      if(color != 0x0000) {
        uint8_t r = (color >> 11) & 0x1F;
        uint8_t g = (color >> 5) & 0x3F;
        uint8_t b = color & 0x1F;

        // Pomijamy piksele, które po zblendowaniu i tak byłyby kolorem tła
        if (r == 0 && g <= 8 && b <= 8) {
          continue;
        }

        // Podbijamy ciemne piksele na skrajach logo do poziomu jasności tła
        g = max((int)g, 8);
        b = max((int)b, 8);

        uint16_t smoothedColor = (r << 11) | (g << 5) | b;
        tft.drawPixel(60 + x, 104 + y, smoothedColor);
      }
    }
  }
}

void drawMainScreen(String fullName, String role, String company, String email, String phone, String qrUrl, int selectedMenu) {
  static int lastMenu = -1;
  static int lastMenuVisible = -1;

  // Przy pierwszym wywołaniu rysujemy całe tło oraz paski
  if (lastMenu == -1) {
    tft.fillScreen(COMARCH_NAVY); 
    drawTopBar();
    drawBottomBar();
  }

  // Sprawdzamy czy widok po lewej stronie (Profil <-> Harmonogram) wymaga przerysowania
  bool viewChanged = (lastMenu == -1) || (selectedMenu == 5 && lastMenu != 5) || (selectedMenu != 5 && lastMenu == 5);
  bool menuVisibilityChanged = (isMenuVisible != (lastMenuVisible == 1));

  // Jeśli menu zostało schowane, czyścimy jego obszar przywracając kolor tła
  // i wymuszamy odświeżenie aktywnego widoku, by odzyskać elementy z prawej strony
  if (menuVisibilityChanged && !isMenuVisible) {
    tft.fillRect(180, 33, 140, 187, COMARCH_NAVY);
    viewChanged = true;
  }

  if (viewChanged) {
    switch (selectedMenu) {
      case 5: // Schedule
        drawSchedule(0);
        break;
      default: // Dla wszystkich innych opcji pokazujemy profil
        drawProfile(fullName, role, company, email, phone, qrUrl);
        break;
    }
  }

  // Menu po prawej stronie odświeżamy tylko gdy jest widoczne i wymaga aktualizacji
  if (isMenuVisible && (selectedMenu != lastMenu || viewChanged || menuVisibilityChanged)) {
    drawMenu(selectedMenu);
  }

  lastMenu = selectedMenu;
  lastMenuVisible = isMenuVisible ? 1 : 0;
}

void drawTopBar() {
  tft.fillRect(0, 0, 320, 32, COMARCH_BLUE); 
  
  for(int y = 0; y < 18; y++) {
    for(int x = 0; x < 115; x++) {
      uint16_t color = pgm_read_word(&CMR_WA_SMALL_565[y * 115 + x]);
      if(color != 0x0000) {
        uint8_t r = (color >> 11) & 0x1F;
        uint8_t g = (color >> 5) & 0x3F;
        uint8_t b = color & 0x1F;

        int alpha = (b * 255) / 18;
        if (alpha > 255) alpha = 255;

        int r_bg = 0, g_bg = 23, b_bg = 21;
        uint8_t r_new = (alpha * r + (255 - alpha) * r_bg) / 255;
        uint8_t g_new = (alpha * g + (255 - alpha) * g_bg) / 255;
        uint8_t b_new = (alpha * b + (255 - alpha) * b_bg) / 255;

        tft.drawPixel(102 + x, 7 + y, (r_new << 11) | (g_new << 5) | b_new);
      }
    }
  }
  tft.drawFastHLine(0, 32, 320, COMARCH_CYAN);
}

void drawBottomBar() {
  tft.fillRect(0, 220, 320, 20, COMARCH_BLUE);
  tft.drawFastHLine(0, 220, 320, COMARCH_CYAN);
  
  tft.setFont(&Roboto_Condensed_Regular8pt7b);
  tft.setTextColor(ILI9341_WHITE);
  
  tft.setCursor(15, 235); 
  tft.print("[ BACK ]");
  
  tft.setCursor(125, 235); 
  tft.print("[ SELECT ]");
  
  tft.setCursor(255, 235); 
  tft.print("[ MENU ]");
}

void drawProfile(String fullName, String role, String company, String email, String phone, String qrUrl) {
  // Czyścimy lewą część ekranu przed narysowaniem danych profilu
  tft.fillRect(0, 33, 180, 187, COMARCH_NAVY);

  uint16_t maxTextWidth = 180; 

  // Nagłówek z imieniem i nazwiskiem
  tft.setFont(&Roboto_Condensed_Regular16pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(15, 75);
  tft.print(fullName);

  // Blok danych z rolą, firmą, emailem i telefonem
  tft.setFont(&Roboto_Condensed_Regular8pt7b);
  tft.setTextColor(COMARCH_CYAN); 
  tft.setCursor(15, 115); 
  printTruncated(role, maxTextWidth);

  tft.setTextColor(ILI9341_LIGHTGREY);
  tft.setCursor(15, 135);
  printTruncated(company, maxTextWidth);

  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(15, 160);
  printTruncated(email, maxTextWidth);

  tft.setCursor(15, 180);
  printTruncated(phone, maxTextWidth);

  if (!isMenuVisible) {
    drawQRCode(qrUrl.c_str(), 210, 97, 3);
  }
}

void drawMenu(int selectedMenu) {
  tft.fillRect(180, 33, 140, 187, COMARCH_BG);
  tft.drawFastVLine(180, 33, 187, COMARCH_BLUE);

  String menuItems[] = {"Profile", "NFC Access", "Messages", "Games", "Schedule", "Settings"};
  int menuCount = 6;
  
  int maxVisibleItems = 5;
  int startIndex = 0;
  
  if (selectedMenu >= maxVisibleItems) {
    startIndex = selectedMenu - maxVisibleItems + 1;
  }

  tft.setFont(&Roboto_Condensed_Regular8pt7b);
  for(int i = startIndex; i < min(menuCount, startIndex + maxVisibleItems); i++) {
    int displayIndex = i - startIndex;
    int boxY = 48 + (displayIndex * 32); 
    
    if(i == selectedMenu) {
      tft.fillRoundRect(185, boxY, 130, 26, 4, COMARCH_BLUE); 
      tft.setTextColor(ILI9341_WHITE);
    } else {
      tft.setTextColor(ILI9341_LIGHTGREY);
    }
    
    tft.setCursor(192, boxY + 18); 
    tft.print(menuItems[i]);
  }

  // Wyśrodkowany trójkąt wskazujący na wybrany element menu
  int selectedDisplayIndex = selectedMenu - startIndex;
  int selectedBoxY = 48 + (selectedDisplayIndex * 32);
  int centerY = selectedBoxY + 13; // 13 to połowa wysokości prostokąta podświetlenia (26 / 2)
  tft.fillTriangle(315, centerY - 5, 310, centerY, 315, centerY + 5, COMARCH_CYAN);
}

void drawSchedule(int scrollOffset) {
  // Wyczyść obszar zawartości po lewej stronie
  tft.fillRect(0, 33, 180, 187, COMARCH_NAVY);

  // Przykładowe dane harmonogramu, które w przyszłości będą pobierane z zewnątrz
  ScheduleEvent events[] = {
    {"Opening Ceremony", "09:00-09:30"},
    {"Keynote Speech",   "09:30-10:30"},
    {"Coffee Break",     "10:30-11:00"},
    {"Workshop A",       "11:00-12:30"},
    {"Lunch",            "12:30-13:30"},
    {"Workshop B",       "13:30-15:00"},
    {"Panel Discussion", "15:00-16:00"},
    {"Closing Remarks",  "16:00-16:30"}
  };
  int eventCount = sizeof(events) / sizeof(events[0]);

  // Narysuj tytuł widoku
  tft.setFont(&Roboto_Condensed_Regular12pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(15, 55);
  tft.print("Schedule");

  // Narysuj listę wydarzeń
  tft.setFont(&Roboto_Condensed_Regular8pt7b);
  int yPos = 80;
  int lineHeight = 20;
  int maxVisibleItems = (220 - yPos) / lineHeight; 

  for (int i = scrollOffset; i < eventCount && (i - scrollOffset) < maxVisibleItems; i++) {
    int currentY = yPos + (i - scrollOffset) * lineHeight;

    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(15, currentY);
    tft.print(events[i].title);

    if (!isMenuVisible) {
      tft.setTextColor(COMARCH_CYAN);
      tft.setCursor(230, currentY);
      tft.print(events[i].time);
    }
  }
}

void drawQRCode(const char* url, int offset_x, int offset_y, int scale) {
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, url);

  int padding = 2 * scale; 
  int qrSize = qrcode.size * scale;
  tft.fillRect(offset_x, offset_y, qrSize + 2 * padding, qrSize + 2 * padding, ILI9341_WHITE);

  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        tft.fillRect(offset_x + padding + x * scale, offset_y + padding + y * scale, scale, scale, ILI9341_BLACK);
      }
    }
  }
}

void printTruncated(String text, uint16_t maxWidth) {
  int16_t x1, y1;
  uint16_t w, h;
  
  tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  
  if (w <= maxWidth) {
    tft.print(text);
  } else {
    String truncated = text;
    while (truncated.length() > 0) {
      truncated.remove(truncated.length() - 1);
      tft.getTextBounds(truncated + "...", 0, 0, &x1, &y1, &w, &h);
      if (w <= maxWidth) {
        tft.print(truncated + "...");
        break;
      }
    }
  }
}