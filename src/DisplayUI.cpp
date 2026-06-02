#include "DisplayUI.h"
#include "Config.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <qrcode.h>

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

namespace {
constexpr int kScreenW = 320;
constexpr int kContentTop = 33;
constexpr int kContentH = 187;
constexpr int kMenuPanelLeft = 180;
constexpr int kMenuPanelW = 140;

int contentWidth() { return isMenuVisible ? 180 : kScreenW; }
}  // namespace

void drawSchedule(const ScheduleEvent* events, int eventCount, int scrollOffset);
void drawMessagesList(const BadgeMessage* messages, int messageCount, int selectedMessage,
                      int scrollOffset);
void drawMessageDetail(const BadgeMessage* message);
void drawQRCode(const char* url, int offset_x, int offset_y, int scale);
void printTruncated(String text, uint16_t maxWidth);
void printTruncatedAt(int16_t x, int16_t y, String text, uint16_t maxWidth);

/** Initializes the ILI9341 display. */
void initDisplay() {
  SPI.begin();
  tft.begin();
  tft.setRotation(1);
}

/**
 * Full-screen splash: Comarch logo from PROGMEM bitmap.
 * Dark edge pixels are lifted to match COMARCH_NAVY so anti-aliased fringes do not read as black halos.
 */
void drawSplashScreen() {
  tft.fillScreen(COMARCH_NAVY);
  for (int y = 0; y < 31; y++) {
    for (int x = 0; x < 200; x++) {
      uint16_t color = pgm_read_word(&CMR_WA_BIG_565[y * 200 + x]);
      if (color != 0x0000) {
        uint8_t r = (color >> 11) & 0x1F;
        uint8_t g = (color >> 5) & 0x3F;
        uint8_t b = color & 0x1F;

        if (r == 0 && g <= 8 && b <= 8) {
          continue;
        }

        g = max((int)g, 8);
        b = max((int)b, 8);

        uint16_t smoothedColor = (r << 11) | (g << 5) | b;
        tft.drawPixel(60 + x, 104 + y, smoothedColor);
      }
    }
  }
}

/**
 * Main UI frame: left content pane (profile or schedule) and optional right-hand menu.
 * Uses static state to redraw only what changed (view switch, menu visibility, selection).
 *
 * @param selectedMenu Index 0–5 into the side menu.
 * @param scheduleEvents Rows to show when Schedule is selected (may be nullptr).
 * @param messages Inbox rows when Messages is selected (may be nullptr).
 * @param messageDetailOpen When true, left pane shows full text of @p selectedMessage.
 */
void drawMainScreen(String fullName, String role, String company, String email, String phone,
                    String qrUrl, int selectedMenu, const ScheduleEvent* scheduleEvents,
                    int scheduleEventCount, int scheduleScrollOffset, const BadgeMessage* messages,
                    int messageCount, int selectedMessage, int messageScrollOffset,
                    bool messageDetailOpen) {
  static int lastMenu = -1;
  static int lastMenuVisible = -1;
  static int lastSelectedMessage = -1;
  static int lastMessageScrollOffset = -1;
  static bool lastMessageDetailOpen = false;

  if (lastMenu == -1) {
    tft.fillScreen(COMARCH_NAVY);
    drawTopBar();
    drawBottomBar();
  }

  bool menuVisibilityChanged = (isMenuVisible != (lastMenuVisible == 1));
  bool messagesPaneChanged =
      (selectedMenu == MENU_MESSAGES) &&
      (messageDetailOpen != lastMessageDetailOpen || selectedMessage != lastSelectedMessage ||
       messageScrollOffset != lastMessageScrollOffset);

  bool viewChanged = (lastMenu == -1) || (selectedMenu != lastMenu) || messagesPaneChanged ||
                     menuVisibilityChanged;

  // Side menu is a temporary overlay; content is drawn only when the menu is closed.
  if (isMenuVisible) {
    if (menuVisibilityChanged || selectedMenu != lastMenu) {
      drawMenu(selectedMenu);
    }
    lastMenu = selectedMenu;
    lastMenuVisible = 1;
    lastSelectedMessage = selectedMessage;
    lastMessageScrollOffset = messageScrollOffset;
    lastMessageDetailOpen = messageDetailOpen;
    return;
  }

  if (viewChanged) {
    tft.fillRect(0, kContentTop, kScreenW, kContentH, COMARCH_NAVY);
    switch (selectedMenu) {
      case MENU_MESSAGES:
        if (messageDetailOpen && messages != nullptr && selectedMessage >= 0 &&
            selectedMessage < messageCount) {
          drawMessageDetail(&messages[selectedMessage]);
        } else {
          drawMessagesList(messages, messageCount, selectedMessage, messageScrollOffset);
        }
        break;
      case MENU_SCHEDULE:
        drawSchedule(scheduleEvents, scheduleEventCount, scheduleScrollOffset);
        break;
      default:
        drawProfile(fullName, role, company, email, phone, qrUrl);
        break;
    }
  }

  lastMenu = selectedMenu;
  lastMenuVisible = 0;
  lastSelectedMessage = selectedMessage;
  lastMessageScrollOffset = messageScrollOffset;
  lastMessageDetailOpen = messageDetailOpen;
}

/**
 * Top chrome bar with Comarch branding.
 * Small logo uses per-pixel alpha derived from the blue channel and blended onto COMARCH_BLUE.
 */
void drawTopBar() {
  tft.fillRect(0, 0, 320, 32, COMARCH_BLUE);

  for (int y = 0; y < 18; y++) {
    for (int x = 0; x < 115; x++) {
      uint16_t color = pgm_read_word(&CMR_WA_SMALL_565[y * 115 + x]);
      if (color != 0x0000) {
        uint8_t r = (color >> 11) & 0x1F;
        uint8_t g = (color >> 5) & 0x3F;
        uint8_t b = color & 0x1F;

        int alpha = (b * 255) / 18;
        if (alpha > 255) {
          alpha = 255;
        }

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

/** Bottom navigation hints (hardware button labels). */
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

/**
 * Digital business card in the left pane.
 * QR code is drawn only when the side menu is hidden so it is not covered by the panel.
 */
void drawProfile(String fullName, String role, String company, String email, String phone,
                 String qrUrl) {
  const int w = contentWidth();
  tft.fillRect(0, kContentTop, w, kContentH, COMARCH_NAVY);

  uint16_t maxTextWidth = w - 30;

  tft.setFont(&Roboto_Condensed_Regular16pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(15, 75);
  tft.print(fullName);

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

  drawQRCode(qrUrl.c_str(), w - 110, 97, 3);
}

/**
 * Scrollable side menu (max five visible rows).
 * startIndex advances when the selection moves past the visible window.
 */
void drawMenu(int selectedMenu) {
  tft.fillRect(kMenuPanelLeft, kContentTop, kMenuPanelW, kContentH, COMARCH_BG);
  tft.drawFastVLine(kMenuPanelLeft, kContentTop, kContentH, COMARCH_BLUE);

  String menuItems[] = {"Profile", "NFC Access", "Messages", "Games", "Schedule", "Settings"};
  int menuCount = 6;

  int maxVisibleItems = 5;
  int startIndex = 0;

  if (selectedMenu >= maxVisibleItems) {
    startIndex = selectedMenu - maxVisibleItems + 1;
  }

  tft.setFont(&Roboto_Condensed_Regular8pt7b);
  for (int i = startIndex; i < min(menuCount, startIndex + maxVisibleItems); i++) {
    int displayIndex = i - startIndex;
    int boxY = 48 + (displayIndex * 32);

    if (i == selectedMenu) {
      tft.fillRoundRect(185, boxY, 130, 26, 4, COMARCH_BLUE);
      tft.setTextColor(ILI9341_WHITE);
    } else {
      tft.setTextColor(ILI9341_LIGHTGREY);
    }

    tft.setCursor(192, boxY + 18);
    tft.print(menuItems[i]);
  }

  int selectedDisplayIndex = selectedMenu - startIndex;
  int selectedBoxY = 48 + (selectedDisplayIndex * 32);
  int centerY = selectedBoxY + 13;
  tft.fillTriangle(315, centerY - 5, 310, centerY, 315, centerY + 5, COMARCH_CYAN);
}

/**
 * Conference schedule list.
 * Event times are omitted while the menu is open to avoid overlap with the right pane.
 *
 * @param events Event rows to render (ignored when @p eventCount is 0).
 * @param scrollOffset Index of the first event row to render.
 */
void drawSchedule(const ScheduleEvent* events, int eventCount, int scrollOffset) {
  const int w = contentWidth();
  tft.fillRect(0, kContentTop, w, kContentH, COMARCH_NAVY);

  tft.setFont(&Roboto_Condensed_Regular12pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(15, 55);
  tft.print("Schedule");

  tft.setFont(&Roboto_Condensed_Regular8pt7b);
  int yPos = 80;
  int lineHeight = 20;
  int maxVisibleItems = (220 - yPos) / lineHeight;

  if (events == nullptr) {
    eventCount = 0;
  }

  for (int i = scrollOffset; i < eventCount && (i - scrollOffset) < maxVisibleItems; i++) {
    int currentY = yPos + (i - scrollOffset) * lineHeight;

    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(15, currentY);
    tft.print(events[i].title);

    tft.setTextColor(COMARCH_CYAN);
    tft.setCursor(w - 90, currentY);
    tft.print(events[i].time);
  }
}

/**
 * Renders a QR code (version 3) scaled by integer pixel blocks.
 */
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
        tft.fillRect(offset_x + padding + x * scale, offset_y + padding + y * scale, scale, scale,
                     ILI9341_BLACK);
      }
    }
  }
}

/** Prints @p text, appending "..." until the rendered width fits @p maxWidth. */
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

void printTruncatedAt(int16_t x, int16_t y, String text, uint16_t maxWidth) {
  tft.setCursor(x, y);
  printTruncated(text, maxWidth);
}

/**
 * Inbox list with menu-style selection (one highlighted row).
 * Each row: sender + time header, single-line preview via printTruncated.
 */
void drawMessagesList(const BadgeMessage* messages, int messageCount, int selectedMessage,
                      int scrollOffset) {
  const int w = contentWidth();
  tft.fillRect(0, kContentTop, w, kContentH, COMARCH_NAVY);

  tft.setFont(&Roboto_Condensed_Regular12pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(15, 55);
  tft.print("Messages");

  if (messages == nullptr) {
    messageCount = 0;
  }

  if (messageCount == 0) {
    tft.setFont(&Roboto_Condensed_Regular8pt7b);
    tft.setTextColor(ILI9341_LIGHTGREY);
    tft.setCursor(15, 85);
    tft.print("No messages");
    return;
  }

  const int rowHeight = 36;
  const int yStart = 68;
  const int maxVisibleRows = (220 - yStart) / rowHeight;
  const uint16_t previewWidth = w - 40;
  const int rowWidth = w - 16;

  int startIndex = scrollOffset;
  if (selectedMessage >= startIndex + maxVisibleRows) {
    startIndex = selectedMessage - maxVisibleRows + 1;
  }
  if (selectedMessage < startIndex) {
    startIndex = selectedMessage;
  }

  tft.setFont(&Roboto_Condensed_Regular8pt7b);
  for (int i = startIndex; i < messageCount && (i - startIndex) < maxVisibleRows; i++) {
    int rowY = yStart + (i - startIndex) * rowHeight;
    const BadgeMessage& msg = messages[i];

    if (i == selectedMessage) {
      tft.fillRoundRect(8, rowY, rowWidth, rowHeight - 4, 4, COMARCH_BLUE);
      tft.setTextColor(ILI9341_WHITE);
    } else {
      tft.setTextColor(ILI9341_LIGHTGREY);
    }

    tft.setCursor(14, rowY + 12);
    if (msg.sender != nullptr) {
      printTruncated(String(msg.sender), 90);
    }

    tft.setTextColor(i == selectedMessage ? COMARCH_CYAN : ILI9341_DARKGREY);
    tft.setCursor(w - 55, rowY + 12);
    if (msg.receivedAt != nullptr) {
      tft.print(msg.receivedAt);
    }

    tft.setTextColor(i == selectedMessage ? ILI9341_WHITE : ILI9341_LIGHTGREY);
    printTruncatedAt(14, rowY + 26, msg.content != nullptr ? String(msg.content) : String(),
                     previewWidth);
  }

  if (messageCount > 0) {
    int selectedRow = selectedMessage - startIndex;
    int boxY = yStart + selectedRow * rowHeight;
    int centerY = boxY + (rowHeight - 4) / 2;
    int arrowX = w - 8;
    tft.fillTriangle(arrowX, centerY - 5, arrowX - 5, centerY, arrowX, centerY + 5, COMARCH_CYAN);
  }
}

/** Full message body (opened with SELECT from the inbox list). */
void drawMessageDetail(const BadgeMessage* message) {
  const int w = contentWidth();
  tft.fillRect(0, kContentTop, w, kContentH, COMARCH_NAVY);

  if (message == nullptr) {
    return;
  }

  tft.setFont(&Roboto_Condensed_Regular12pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(15, 55);
  tft.print("Message");

  tft.setFont(&Roboto_Condensed_Regular8pt7b);
  tft.setTextColor(COMARCH_CYAN);
  tft.setCursor(15, 78);
  if (message->sender != nullptr) {
    printTruncated(String(message->sender), w - 30);
  }

  tft.setTextColor(ILI9341_LIGHTGREY);
  tft.setCursor(15, 94);
  if (message->receivedAt != nullptr) {
    tft.print(message->receivedAt);
  }

  tft.setTextColor(ILI9341_WHITE);
  int y = 118;
  const int lineHeight = 16;
  const int maxY = 210;
  const uint16_t lineWidth = w - 30;

  if (message->content == nullptr) {
    return;
  }

  String remaining = message->content;
  while (remaining.length() > 0 && y <= maxY) {
    tft.setCursor(15, y);
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(remaining, 0, 0, &x1, &y1, &w, &h);

    if (w <= lineWidth) {
      tft.print(remaining);
      break;
    }

    int cut = remaining.length();
    while (cut > 0) {
      String chunk = remaining.substring(0, cut);
      tft.getTextBounds(chunk, 0, 0, &x1, &y1, &w, &h);
      if (w <= lineWidth) {
        int lastSpace = chunk.lastIndexOf(' ');
        if (lastSpace > 0 && cut < (int)remaining.length()) {
          chunk = remaining.substring(0, lastSpace);
          remaining = remaining.substring(lastSpace + 1);
        } else {
          remaining = remaining.substring(cut);
        }
        tft.print(chunk);
        y += lineHeight;
        break;
      }
      cut--;
    }

    if (cut == 0) {
      printTruncated(remaining, lineWidth);
      break;
    }
  }
}

/**
 * First-run pairing screen: activation URL encodes the device MAC for the backend.
 */
void drawRegistrationScreen(String macAddress) {
  tft.fillScreen(COMARCH_NAVY);
  drawTopBar();
  drawBottomBar();

  tft.setFont(&Roboto_Condensed_Regular16pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(15, 100);
  tft.print("Badge Setup");

  tft.setFont(&Roboto_Condensed_Regular8pt7b);
  tft.setTextColor(COMARCH_CYAN);
  tft.setCursor(15, 135);
  tft.print("Scan QR code to activate");

  tft.setTextColor(ILI9341_LIGHTGREY);
  tft.setCursor(15, 155);
  tft.print("Device MAC:");
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(15, 175);
  tft.print(macAddress);

  String url = "http://localhost:6769/?mac=" + macAddress;
  drawQRCode(url.c_str(), 190, 75, 3);
}
