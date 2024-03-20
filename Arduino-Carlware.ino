#include <LiquidCrystal.h>
#include <FastLED.h>

#define LED_PIN 11
#define NUM_LEDS 270
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

const int SW_pin = 13;
const int X_pin = 1;
const int Y_pin = 0;

int interval_inputHandler = 30;
int interval_actionDelay = 200;
int interval_LiquidCrystal = 50;
int interval_displayLED = 20;
int interval_Seed = 10000;

int Seed[2][3];
int Display[3];
int masterLED[2];
int leftRight;

// Tracking variables for the timings // maybe put these into a struc type
unsigned long previousTime_actionDelay;
unsigned long previousTime_inputHandler;
unsigned long previousTime_LiquidCrystal;
unsigned long previousTime_displayLED;
unsigned long previousTime_randomSeed;
unsigned long currentTime;

const char *mainMenu[] = {"Dream Mode    ",
                          "Manual RGB    ",
                          "Daylight Sim  ",
                          "Meditation    "};

const char *optionMenu[] = {"Red       ",
                            "Green     ",
                            "Blue      ",
                            "Brightness"};

CRGB leds[NUM_LEDS];
LiquidCrystal lcd(1, 2, 4, 5, 6, 7);

struct menu
{

  int indexNumber[2];
  int indexPosition[2];
  int indexLevel;

  bool exitOptions = false;

  int num_mainMenu = sizeof(mainMenu) / sizeof(mainMenu[0]);
  int num_optionMenu = sizeof(optionMenu) / sizeof(optionMenu[0]);

  int optionValue[4]; // should equal the number of options in option menu

} menuData;

struct flags
{

  bool upFlag = false;
  bool downFlag = false;
  bool rightFlag = false;
  bool leftFlag = false;
  bool clickFlag = false;

} analogFlag;

bool startUpFlag = true;

void displayMainMenu();
void displaySelectMenu();
void displayRGBOptionMenu();
void readInputs();
void inputHandler();

void advanceMainMenuUp();
void advanceMainMenuDown();

void advanceSelectMenuLeft();
void advanceSelectMenuRight();

void advanceRGBMenuUp();
void advanceRGBMenuDown();
void advanceRGBMenuLeft();
void advanceRGBMenuRight();

void clickFlagHandler();

void modeRGB();
void modeDefault();
void modeDream();
void modeDaylight();
void modeMeditation();

void setup()
{
  lcd.begin(16, 2);
  pinMode(SW_pin, INPUT);
  digitalWrite(SW_pin, HIGH);
  byte upArrow[8] = {
      B00100,
      B01110,
      B11111,
      B00100,
      B00100,
      B00100,
      B00100,
  };
  byte downArrow[8] = {
      B00100,
      B00100,
      B00100,
      B00100,
      B11111,
      B01110,
      B00100,
  };
  byte selectArrow[8] = {
      B00000,
      B11000,
      B01100,
      B00110,
      B01100,
      B11000,
      B00000,
  };
  lcd.createChar(1, upArrow);
  lcd.createChar(2, downArrow);
  lcd.createChar(3, selectArrow);
  delay(3000);                                                                                    // LED power-up delay, safety first 8^)
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip); // Init for FastLED

  menuData.optionValue[0] = 255; // Red
  menuData.optionValue[1] = 165; // Green
  menuData.optionValue[2] = 0;   // Blue
  menuData.optionValue[3] = 10;  // Brightness
}

void readInputs()
{
  currentTime = millis();
  if (currentTime - previousTime_actionDelay > interval_actionDelay)
  {

    int xAxis = map(analogRead(X_pin), 0, 1200, 100, 0);
    int offsetFromX = 58;                    // slop offset
    leftRight = ((xAxis - offsetFromX) / 6); // -43 to 0 Left, 0 to 43 is Right

    if (leftRight <= -1)
    {
      previousTime_actionDelay = currentTime;
      analogFlag.leftFlag = true;
    }
    if (leftRight >= 1)
    {
      previousTime_actionDelay = currentTime;
      analogFlag.rightFlag = true;
    }

    int yAxis = map(analogRead(Y_pin), 0, 1200, 100, 0);
    if (yAxis > 80)
    {
      previousTime_actionDelay = currentTime;
      analogFlag.upFlag = true;
    }
    if (yAxis < 30)
    {
      previousTime_actionDelay = currentTime;
      analogFlag.downFlag = true;
    }

    if (digitalRead(SW_pin) == LOW)
    {
      previousTime_actionDelay = currentTime;
      analogFlag.clickFlag = true;
    }
  }
}

void advanceMainMenuUp()
{
  if (menuData.indexPosition[0] == 0 && menuData.indexNumber[0] > 0)
  {
    menuData.indexNumber[0] = menuData.indexNumber[0] - 1;
  }
  else if (menuData.indexPosition[0] == 1)
  {
    menuData.indexPosition[0] = 0;
  }
}

void advanceMainMenuDown()
{
  if (menuData.indexPosition[0] == 1 && menuData.indexNumber[0] < (menuData.num_mainMenu - 2))
  {
    menuData.indexNumber[0] = menuData.indexNumber[0] + 1;
  }
  else if (menuData.indexPosition[0] == 0)
  {
    menuData.indexPosition[0] = 1;
  }
}

void advanceRGBMenuUp()
{
  if (menuData.indexPosition[1] == 0 && menuData.indexNumber[1] > 0)
  {
    menuData.indexNumber[1] = menuData.indexNumber[1] - 1;
  }
  else if (menuData.indexPosition[1] == 1)
  {
    menuData.indexPosition[1] = 0;
  }
}

void advanceRGBMenuDown()
{
  if (menuData.indexPosition[1] == 1 && menuData.indexNumber[1] < (menuData.num_mainMenu - 2))
  {
    menuData.indexNumber[1] = menuData.indexNumber[1] + 1;
  }
  else if (menuData.indexPosition[1] == 0)
  {
    menuData.indexPosition[1] = 1;
  }
}

void advanceSelectMenuLeft()
{
  if (menuData.exitOptions == true)
  {
    menuData.exitOptions = false;
  }
}

void advanceSelectMenuRight()
{
  if (menuData.exitOptions == false)
  {
    menuData.exitOptions = true;
  }
}

void advanceRGBMenuLeft()
{
  if (menuData.indexPosition[1] == 1)
  {
    if (menuData.optionValue[menuData.indexNumber[1] + 1] + leftRight > 0)
    {
      menuData.optionValue[menuData.indexNumber[1] + 1] += leftRight;
    }
    else
    {
      menuData.optionValue[menuData.indexNumber[1] + 1] = 0;
    }
  }
  else if (menuData.indexPosition[1] == 0)
  {
    if (menuData.optionValue[menuData.indexNumber[1]] + leftRight > 0)
    {
      menuData.optionValue[menuData.indexNumber[1]] += leftRight;
    }
    else
    {
      menuData.optionValue[menuData.indexNumber[1]] = 0;
    }
  }
}

void advanceRGBMenuRight()
{
  if (menuData.indexPosition[1] == 1)
  {
    if (menuData.optionValue[menuData.indexNumber[1] + 1] + 1 + leftRight < 255)
    {
      menuData.optionValue[menuData.indexNumber[1] + 1] += leftRight;
    }
    else
    {
      menuData.optionValue[menuData.indexNumber[1] + 1] = 255;
    }
  }
  else if (menuData.indexPosition[1] == 0)
  {
    if (menuData.optionValue[menuData.indexNumber[1]] + 1 + leftRight < 255)
    {
      menuData.optionValue[menuData.indexNumber[1]] += leftRight;
    }
    else
    {
      menuData.optionValue[menuData.indexNumber[1]] = 255;
    }
  }
}

void clickFlagHandler()
{
  if (menuData.indexLevel == 0)
  {
    menuData.indexLevel++;
    lcd.clear();
  }
  else if (menuData.indexLevel == 1 && menuData.exitOptions == false)
  {
    menuData.indexLevel = 0;
    lcd.clear();
  }
  else if (menuData.indexLevel == 1 && menuData.exitOptions == true)
  {
    menuData.indexLevel++;
    lcd.clear();
  }
  else
  {
    menuData.indexLevel = 1;
    menuData.exitOptions = false;
    lcd.clear();
  }
}

void inputHandler()
{
  currentTime = millis();
  if (currentTime - previousTime_inputHandler > interval_inputHandler)
  {
    previousTime_inputHandler = currentTime;

    if (analogFlag.upFlag)
    {
      analogFlag.upFlag = false;
      if (menuData.indexLevel == 0)
      {
        advanceMainMenuUp();
      }
      else if (menuData.indexLevel == 2)
      {
        advanceRGBMenuUp();
      }
    }
    else if (analogFlag.downFlag)
    {
      analogFlag.downFlag = false;
      if (menuData.indexLevel == 0)
      {
        advanceMainMenuDown();
      }
      else if (menuData.indexLevel == 2)
      {
        advanceRGBMenuDown();
      }
    }
    else if (analogFlag.leftFlag)
    {
      analogFlag.leftFlag = false;
      if (menuData.indexLevel == 1)
      {
        advanceSelectMenuLeft();
      }
      else if (menuData.indexLevel == 2)
      {
        advanceRGBMenuLeft();
      }
    }
    else if (analogFlag.rightFlag)
    {
      analogFlag.rightFlag = false;
      if (menuData.indexLevel == 1)
      {
        advanceSelectMenuRight();
      }
      else if (menuData.indexLevel == 2)
      {
        advanceRGBMenuRight();
      }
    }

    if (analogFlag.clickFlag)
    {
      analogFlag.clickFlag = false;
      clickFlagHandler();
    }
  }
}

void displayMainMenu()
{
  currentTime = millis();
  if (currentTime - previousTime_LiquidCrystal > interval_LiquidCrystal)
  {
    previousTime_LiquidCrystal = currentTime;

    if (menuData.indexPosition[0] == 0)
    {
      lcd.setCursor(0, 0);
      lcd.write(3); // selectArrow
      lcd.setCursor(0, 1);
      lcd.print(" ");
    }
    if (menuData.indexPosition[0] == 1)
    {
      lcd.setCursor(0, 1);
      lcd.write(3); // selectArrow
      lcd.setCursor(0, 0);
      lcd.print(" ");
    }
    if (menuData.indexNumber[0] == 0)
    {
      lcd.setCursor(15, 1);
      lcd.write(2); // downArrow
      lcd.setCursor(15, 0);
      lcd.print(" ");
    }

    if (menuData.indexNumber[0] > 0 && menuData.indexNumber[0] < menuData.num_mainMenu - 2)
    {
      lcd.setCursor(15, 0);
      lcd.write(1); // upArrow
      lcd.setCursor(15, 1);
      lcd.write(2); // downArrow
    }
    if (menuData.indexNumber[0] == menuData.num_mainMenu - 2)
    {
      lcd.setCursor(15, 0);
      lcd.write(1); // upArrow
      lcd.setCursor(15, 1);
      lcd.print(" ");
    }

    lcd.setCursor(1, 0);

    lcd.print(mainMenu[menuData.indexNumber[0]]);
    lcd.setCursor(1, 1);

    lcd.print(mainMenu[menuData.indexNumber[0] + 1]);
  }
}

void displaySelectMenu()
{
  currentTime = millis();
  if (currentTime - previousTime_LiquidCrystal > interval_LiquidCrystal)
  {
    previousTime_LiquidCrystal = currentTime;
    lcd.setCursor(1, 0);
    if (menuData.indexPosition[0] == 1)
    {
      lcd.print(mainMenu[menuData.indexNumber[0] + 1]);
    }
    else
    {
      lcd.print(mainMenu[menuData.indexNumber[0]]);
    }

    lcd.setCursor(1, 1);
    lcd.print("Exit");
    lcd.setCursor(8, 1);
    lcd.print("Options");

    if (menuData.exitOptions)
    {
      lcd.setCursor(7, 1);
      lcd.write(3);
      lcd.setCursor(0, 1);
      lcd.print(" ");
    }
    else
    {
      lcd.setCursor(0, 1);
      lcd.write(3);
      lcd.setCursor(7, 1);
      lcd.print(" ");
    }
  }
}

void displayRGBOptionMenu()
{
  currentTime = millis();
  if (currentTime - previousTime_LiquidCrystal > interval_LiquidCrystal)
  {
    previousTime_LiquidCrystal = currentTime;

    if (menuData.indexPosition[1] == 0)
    {
      lcd.setCursor(0, 0);
      lcd.write(3); // selectArrow
      lcd.setCursor(0, 1);
      lcd.print(" ");
    }
    if (menuData.indexPosition[1] == 1)
    {
      lcd.setCursor(0, 1);
      lcd.write(3); // selectArrow
      lcd.setCursor(0, 0);
      lcd.print(" ");
    }
    if (menuData.indexNumber[1] == 0)
    {
      lcd.setCursor(15, 1);
      lcd.write(2); // downArrow
      lcd.setCursor(15, 0);
      lcd.print(" ");
    }

    if (menuData.indexNumber[1] > 0 && menuData.indexNumber[1] < menuData.num_optionMenu - 2)
    {
      lcd.setCursor(15, 0);
      lcd.write(1); // upArrow
      lcd.setCursor(15, 1);
      lcd.write(2); // downArrow
    }
    if (menuData.indexNumber[1] == menuData.num_optionMenu - 2)
    {
      lcd.setCursor(15, 0);
      lcd.write(1); // upArrow
      lcd.setCursor(15, 1);
      lcd.print(" ");
    }

    lcd.setCursor(1, 0);
    lcd.print(optionMenu[menuData.indexNumber[1]]);

    lcd.setCursor(12, 0);
    if (menuData.optionValue[menuData.indexNumber[1]] < 10)
    {
      lcd.print("  ");
      lcd.print(menuData.optionValue[menuData.indexNumber[1]]);
    }
    else if (menuData.optionValue[menuData.indexNumber[1]] > 99)
    {
      lcd.print(menuData.optionValue[menuData.indexNumber[1]]);
    }
    else
    {
      lcd.print(" ");
      lcd.print(menuData.optionValue[menuData.indexNumber[1]]);
    }

    lcd.setCursor(1, 1);
    lcd.print(optionMenu[menuData.indexNumber[1] + 1]);

    lcd.setCursor(12, 1);
    if (menuData.optionValue[menuData.indexNumber[1] + 1] < 10)
    {
      lcd.print("  ");
      lcd.print(menuData.optionValue[menuData.indexNumber[1] + 1]);
    }
    else if (menuData.optionValue[menuData.indexNumber[1] + 1] > 99)
    {
      lcd.print(menuData.optionValue[menuData.indexNumber[1] + 1]);
    }
    else
    {
      lcd.print(" ");
      lcd.print(menuData.optionValue[menuData.indexNumber[1] + 1]);
    }
  }
}

void modeRGB()
{
  currentTime = millis();
  if (currentTime - previousTime_displayLED > interval_displayLED)
  {
    previousTime_displayLED = currentTime;

    int RED = menuData.optionValue[0];
    int GREEN = menuData.optionValue[1];
    int BLUE = menuData.optionValue[2];
    int BRIGHTNESS = menuData.optionValue[3];

    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds[i].setRGB(RED, GREEN, BLUE);
    }

    FastLED.setBrightness(BRIGHTNESS);
    FastLED.show();
  }
}
void modeDefault()
{
  currentTime = millis();
  if (currentTime - previousTime_displayLED > interval_displayLED)
  {
    previousTime_displayLED = currentTime;

    if (masterLED[1] < 84 && masterLED[0] == 0)
    {
      masterLED[1] = masterLED[1] + 1;
    }
    else
    {
      masterLED[0] = 1;
    }
    if (masterLED[1] > 0 && masterLED[0] == 1)
    {
      masterLED[1] = masterLED[1] - 1;
    }
    else
    {
      masterLED[0] = 0;
    }

    int topSide = map(masterLED[1], 0, 84, 224, 140);

    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds[i].setRGB(15, 15, 15);
    }

    leds[topSide - 1].setRGB(255, 0, 0);
    leds[topSide].setRGB(255, 0, 0);
    leds[topSide + 1].setRGB(255, 0, 0);

    leds[masterLED[1] - 1].setRGB(255, 0, 0);
    leds[masterLED[1]].setRGB(255, 0, 0);
    leds[masterLED[1] + 1].setRGB(255, 0, 0);

    int BRIGHTNESS = menuData.optionValue[3];

    FastLED.setBrightness(BRIGHTNESS); // change to BRIGHTNESS \ 2 if too bright
    FastLED.show();
  }
}
void modeDream()
{
  if (startUpFlag == true)
  {
    for (int i = 0; i < 3; i++)
    {
      Seed[0][i] = random(10, 255);
      Seed[1][i] = random(10, 255);
    }
    startUpFlag = false;
  }
  currentTime = millis();
  if (currentTime - previousTime_displayLED > interval_displayLED)
  {

    previousTime_displayLED = currentTime;

    float z = currentTime - previousTime_randomSeed;
    float y = z / interval_Seed; // What percetage of the seed interval is completed
    int a = y * 100;             // converts to whole number percentage

    for (int i = 0; i < 3; i++)
    {
      Display[i] = map(a, 0, 100, Seed[0][i], Seed[1][i]); // maps the output of display[x] to the interval percentage of between the two seed values
    }

    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds[i].setRGB(Display[0], Display[1], Display[2]); // sets the entire LED array to the output of Display[x]
    }

    int BRIGHTNESS = menuData.optionValue[3];

    FastLED.setBrightness(BRIGHTNESS);
    FastLED.show();
  }
  if (currentTime - previousTime_randomSeed > interval_Seed)
  {

    for (int i = 0; i < 3; i++)
    {
      Seed[0][i] = Seed[1][i];
      Seed[1][i] = random(0, 255);
    }

    previousTime_randomSeed = currentTime;
  }
}
void modeMeditation() {
  currentTime = millis();

  if (currentTime - previousTime_displayLED > interval_displayLED) {
    previousTime_displayLED = currentTime;

    // Background color pulsing
    static uint8_t backgroundHue = 128; // Start with a different hue for contrast
    uint8_t backgroundBrightness = sin8(millis() / 20); // Slow pulsing effect

    // Cycle through colors for the wave effect
    static uint8_t waveHue = 0;
    waveHue++; // Slowly change the hue for a rainbow wave effect

    for (int i = 0; i < NUM_LEDS; i++) {
      // Calculate the wave pattern
      uint8_t waveValue = sin8(((currentTime / 10) % 255) + (i * 10));

      // Combine the wave pattern with the pulsing background
      CRGB waveColor = CHSV(waveHue, 255, waveValue);
      CRGB backgroundColor = CHSV(backgroundHue, 255, backgroundBrightness);

      // Blend wave color with background color, giving preference to the brighter of the two
      leds[i] = blend(backgroundColor, waveColor, max(waveValue, backgroundBrightness));

    }

    int BRIGHTNESS = menuData.optionValue[3];
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.show();
  }
}

void modeDaylight()
{
  currentTime = millis();
  if (currentTime - previousTime_displayLED > interval_displayLED)
  {
    previousTime_displayLED = currentTime;
    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds[i].setRGB(200, 200, 200);
    }
    FastLED.setBrightness(200);
    FastLED.show();
  }
}
void loop()
{

  readInputs();
  inputHandler();

  int indexActual;
  if (menuData.indexPosition[0] == 0)
  {
    indexActual = menuData.indexNumber[0];
  }
  else if (menuData.indexPosition[0] == 1)
  {
    indexActual = menuData.indexNumber[0] + 1;
  }

  if (menuData.indexLevel == 0)
  {
    displayMainMenu();
  }
  else if (menuData.indexLevel == 1)
  {
    displaySelectMenu();
  }
  else if (menuData.indexLevel == 2)
  {
    displayRGBOptionMenu();
  }

  if (indexActual == 0 && menuData.indexLevel > 0)
  {
    modeDream();
  }
  else if (indexActual == 1 && menuData.indexLevel > 0)
  {
    modeRGB();
  }
  else if (indexActual == 2 && menuData.indexLevel > 0)
  {
    modeDaylight();
  }
  else if (indexActual == 3 && menuData.indexLevel > 0)
  {
    modeMeditation();
  }
  else
  {
    modeDefault();
    if (startUpFlag == false)
    {
      startUpFlag = true;
    }
  }
}
