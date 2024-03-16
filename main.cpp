#include <Arduino.h>
#include <U8g2lib.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C

#define MENU_BTN_PREV 12
#define MENU_BTN_NEXT 10
#define MENU_BTN_SEL 11

#define DEBOUNCE_DELAY 100

#define MAIN_MENU_LENGTH 3
#define SUB_MENU_LENGTH 4

#define FONT_HEIGHT 12
#define FONT_WIDTH 6
#define BOX_HEIGHT FONT_HEIGHT + 2

#define ALPHA_START 65
#define NUMERAL_START 48

#define MAX_WORD_LEN 21
#define MAIN_FONT u8g2_font_spleen6x12_mr

void init_buttons(void);
void init_screen(void);

int8_t navBack(void);
bool buttonEvent(uint8_t);
int8_t menuNavHandler(void(*[]), char);

void mainMenu(void);
int8_t menuItem1(void);
int8_t menuItem2(void);
int8_t subMenu(void);

int8_t subMenuItem1(void);
int8_t subMenuItem2(void);
int8_t subMenuItem3(void);

void screenHeader(const char *);
void showMenu(const char(*[]), char, int8_t = 0);

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

long lastBounceTime = 0;
int8_t menuCursor = 0;
char menuItemsOnScreen = 4; // To handle menu scrolling

void setup(void)
{
  // for status
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  init_buttons();
  init_screen();
}

void loop(void)
{
  mainMenu();
}

int8_t (*main_funcs[MAIN_MENU_LENGTH])(void) =
    {
        menuItem1,
        menuItem2,
        subMenu};

int8_t (*subMenu_funcs[SUB_MENU_LENGTH])(void) =
    {
        subMenuItem1,
        subMenuItem2,
        subMenuItem3,
        navBack};

const char *mainMenuItems[MAIN_MENU_LENGTH] =
    {
        "Item 1",
        "Item 2",
        "Item 3 >"};

const char *subMenuItems[SUB_MENU_LENGTH] =
    {
        "sub-item 1",
        "sub-item 2",
        "sub-item 3",
        "Back"};

void init_buttons(void)
{
  pinMode(MENU_BTN_PREV, INPUT_PULLUP);
  pinMode(MENU_BTN_NEXT, INPUT_PULLUP);
  pinMode(MENU_BTN_SEL, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
}

void init_screen(void)
{
  if (!u8g2.begin())
  {
    // display failed to init
  }
  u8g2.clearBuffer();
  u8g2.setFontMode(1); // Transparent background
  u8g2.setFont(MAIN_FONT);
  u8g2.sendBuffer();
}

int8_t navBack(void)
{
  return -1;
}

int8_t menuNavHandler(int8_t (*funcptr[])(), char menu_len)
{
  if ((millis() - lastBounceTime) > DEBOUNCE_DELAY)
  {
    if (!digitalRead(MENU_BTN_NEXT))
    {
      menuCursor++;
      if (menuCursor >= menu_len)
        menuCursor = menu_len - 1;
    }

    if (!digitalRead(MENU_BTN_PREV))
    {
      menuCursor--;
      if (menuCursor < 0)
        menuCursor = 0;
    }

    if (!digitalRead(MENU_BTN_SEL))
    {
      return funcptr[menuCursor]();
    }

    lastBounceTime = millis();
  }
  return -2;
}

/// @brief Check to see if a button has been pushed.
/// @param button Input pin of the button to check.
/// @return true if the button has been pushed.
bool buttonEvent(uint8_t button)
{
  if ((millis() - lastBounceTime) > DEBOUNCE_DELAY)
  {
    if (digitalRead(button) == LOW)
    {
      lastBounceTime = millis();
      return true;
    }
  }
  return false;
}

void showMenu(const char *menu[], char menu_len, int8_t offset_y)
{
  for (char m = 0; m <= menu_len - 1; m++)
  {
    if (menuCursor < menuItemsOnScreen)
    {
      int8_t _y = offset_y + (m + 1) * FONT_HEIGHT;
      if (menuCursor == m)
      {
        u8g2.setFont(u8g2_font_unifont_t_symbols);
        u8g2.drawGlyph(0, _y + 2, 0x23F5);
        u8g2.setFont(MAIN_FONT);
        /*
        u8g2.drawRFrame(0,
                          offset_y + (m * FONT_HEIGHT) + 1,
                          u8g2.getDisplayWidth() - 4,
                          BOX_HEIGHT, 2);
                          */
      }
      u8g2.setCursor(2 * FONT_WIDTH, _y);
      u8g2.print(menu[m]);
    }
    else
    { // If the menu is longer than the screen
      // continue to scroll the menu
      int8_t _y = (m - (1 + menuCursor - menuItemsOnScreen)) * FONT_HEIGHT;
      if (menuCursor == m)
      {
        u8g2.setFont(u8g2_font_unifont_t_symbols);
        u8g2.drawGlyph(0, _y + 2, 0x23F5);
        u8g2.setFont(MAIN_FONT);
        /*u8g2.drawRFrame(0,
                           (_y) + 1 - offset_y,
                           u8g2.getDisplayWidth() - 4,
                           BOX_HEIGHT, 2);*/
      }
      if (_y > offset_y)
      {
        u8g2.setCursor(2 * FONT_WIDTH,
                         _y);
        u8g2.print(menu[m]);
      }
    }
  }
  u8g2.sendBuffer();
}

/// @brief Print a header for a screen with a H-line divider
/// @param s Title string to be shown in the header
void screenHeader(const char *s)
{
  u8g2.clearBuffer();
  u8g2.setFontMode(0);
  u8g2.setCursor(0, FONT_HEIGHT);
  u8g2.print(s);
  u8g2.drawHLine(0, FONT_HEIGHT + 1, SCREEN_WIDTH);
  u8g2.setCursor(0, 2 * FONT_HEIGHT);
  u8g2.setFontMode(1);
}

/// @brief Display the main menu
void mainMenu(void)
{
  screenHeader("Main Menu");
  menuNavHandler(main_funcs, MAIN_MENU_LENGTH);
  showMenu(mainMenuItems, MAIN_MENU_LENGTH, FONT_HEIGHT);
  u8g2.clearBuffer();
}

int8_t menuItem1(void)
{
  screenHeader("Menu Item 1");
  u8g2.sendBuffer();

  delay(DEBOUNCE_DELAY);
  while (buttonEvent(MENU_BTN_SEL) == false)
  {
    if (buttonEvent(MENU_BTN_PREV)) {}
    if (buttonEvent(MENU_BTN_NEXT)) {}
  }
  delay(DEBOUNCE_DELAY);

  u8g2.clearBuffer();
  return menuCursor;
}

int8_t menuItem2(void)
{
  screenHeader("Menu Item 2");

  delay(DEBOUNCE_DELAY);
  while (buttonEvent(MENU_BTN_SEL) == false)
  {
    if (buttonEvent(MENU_BTN_PREV)) {}
    if (buttonEvent(MENU_BTN_NEXT)) {}
  }
  delay(DEBOUNCE_DELAY);

  u8g2.clearBuffer();
  return menuCursor;
}

int8_t subMenu(void)
{
  delay(DEBOUNCE_DELAY);
  char oldCursor = menuCursor;
  menuCursor = 0;

  u8g2.clearBuffer();
  while (menuNavHandler(subMenu_funcs, SUB_MENU_LENGTH) != -1)
  {
    screenHeader("Sub Menu");
    showMenu(subMenuItems, SUB_MENU_LENGTH, FONT_HEIGHT);
  }

  menuCursor = oldCursor;
  delay(DEBOUNCE_DELAY);

  u8g2.clearBuffer();
  return menuCursor;
}

int8_t subMenuItem1(void)
{
  bool refresh = true;
  delay(DEBOUNCE_DELAY);
  u8g2.clearBuffer();
  screenHeader("Sub menu 1");
  u8g2.setFontMode(0);
  uint8_t sel = 0;
  const char *options[3] = {"Option A", "Option B", "Option C"};

  while (buttonEvent(MENU_BTN_SEL) == false)
  {
    if (buttonEvent(MENU_BTN_NEXT))
    {
      sel += 1;
      if (sel > 2)
        sel = 2;
      refresh = true;
    }
    else if (buttonEvent(MENU_BTN_PREV))
    {
      sel -= 1;
      if (sel < 0)
        sel = 0;
      refresh = true;
    }

    if (refresh)
    {
      u8g2.setCursor((SCREEN_WIDTH / 2) - (12 * FONT_WIDTH) / 2,
                       (SCREEN_HEIGHT / 2) + (FONT_HEIGHT / 2));
      u8g2.print(F("< "));
      u8g2.print(options[sel]);
      u8g2.print(F(" >"));
      u8g2.sendBuffer();

      refresh = false;
      delay(DEBOUNCE_DELAY);
    }
  }

  u8g2.setFontMode(1);

  return menuCursor;
}

int8_t subMenuItem2(void)
{
  bool refresh = true;
  delay(DEBOUNCE_DELAY);
  u8g2.clearBuffer();
  screenHeader("Sub Menu 2");
  u8g2.setFontMode(0);
  uint16_t val = 255;

  while (buttonEvent(MENU_BTN_SEL) == false)
  {
    if (buttonEvent(MENU_BTN_NEXT))
    {
      val += 5;
      if (val > 600)
        val = 600;
      refresh = true;
    }
    else if (buttonEvent(MENU_BTN_PREV))
    {
      val -= 5;
      if (val < 0)
        val = 0;
      refresh = true;
    }
    if (refresh)
    {
      u8g2.setCursor((SCREEN_WIDTH / 2) - (9 * FONT_WIDTH) / 2,
                       (SCREEN_HEIGHT / 2) + (FONT_HEIGHT / 2));
      u8g2.print(F("< "));
      u8g2.print(val);
      u8g2.print(F(" >"));
      u8g2.sendBuffer();
      refresh = false;
      delay(DEBOUNCE_DELAY);
    }
  }
  u8g2.setFontMode(1);

  return menuCursor;
}

int8_t subMenuItem3(void)
{
  bool refresh = true;
  delay(DEBOUNCE_DELAY);

  u8g2.clearBuffer();
  screenHeader("Sub Menu 3");
  u8g2.setFontMode(0);
  while (buttonEvent(MENU_BTN_SEL) == false)
  {
    if (buttonEvent(MENU_BTN_NEXT))
    {
    
      refresh = true;
    }
    else if (buttonEvent(MENU_BTN_PREV))
    {
      refresh = true;
    }
    if (refresh)
    {
      u8g2.setCursor((SCREEN_WIDTH / 2) - (10 * FONT_WIDTH) / 2,
                       (SCREEN_HEIGHT / 2) + (FONT_HEIGHT / 2));
      u8g2.print(F("< "));
      u8g2.print("Some Value");
      u8g2.print(F(" >"));
      u8g2.sendBuffer();

      refresh = false;
      delay(100);
    }
  }
  u8g2.setFontMode(1);

  return menuCursor;
}
