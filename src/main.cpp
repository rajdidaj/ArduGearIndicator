/*
**------------------------------------------------------------------------------
** ArduGearIndicator:
**
** Keeps track of gear changes based on GPIO inputs.
** Displays the current gear on an Adafruit 1603 OLED
**------------------------------------------------------------------------------
*/
/*
**------------------------------------------------------------------------------
** References
**------------------------------------------------------------------------------
*/
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ADS1015.h>
#include <fonts/FreeSansBold24pt7b.h>
#include <stdint.h>
#include <bitmaps.h>

/*
**------------------------------------------------------------------------------
** Macros
**------------------------------------------------------------------------------
*/
#define PORTRAIT    0
#define LANDSCAPE   1


#define _SESSIONCOUNTER_
#define _THERMOMETER_
//#define _INVERTED_DISPLAY_
#define _ARROWINDICATORS_
#define ORIENTATION LANDSCAPE

/*
**------------------------------------------------------------------------------
** Types
**------------------------------------------------------------------------------
*/
typedef struct
{
    char name[8];
    uint8_t pin;
    uint8_t xOffset;
}indicator_t;

/*
**------------------------------------------------------------------------------
** Constants
**------------------------------------------------------------------------------
*/

#define SCREEN_WIDTH         128        // OLED display width, in pixels
#define SCREEN_HEIGHT        32         // OLED display height, in pixels

#if (ORIENTATION == PORTRAIT)
const indicator_t gears[7] =
{
    { "1", 2, 4 },
    { "N", 3, 0 },
    { "2", 4, 4 },
    { "3", 5, 4 },
    { "4", 6, 4 },
    { "5", 7, 4 },
    { "6", 8, 4 }
};
#elif (ORIENTATION == LANDSCAPE)
const indicator_t gears[7] =
{
    { "1", 2, 64 },
    { "N", 3, 60 },
    { "2", 4, 64 },
    { "3", 5, 64 },
    { "4", 6, 64 },
    { "5", 7, 64 },
    { "6", 8, 64 }
};
#endif

const int16_t ledPin = LED_BUILTIN;

#if (ORIENTATION == PORTRAIT)
const int16_t yBasePos = 60;
const int16_t tempYPos = 110;
const int16_t yTopTextPos = 17;
#elif (ORIENTATION == LANDSCAPE)
const int16_t yBasePos = 32;
const int16_t tempYPos = 20;
const int16_t yTopTextPos = 0;
#endif

#ifdef _THERMOMETER_
#define SAMPLEDELAY         100U
#if (ORIENTATION == PORTRAIT)
const int16_t degXPos = 0;
const int16_t degYPos = tempYPos - 2;
#elif (ORIENTATION == LANDSCAPE)
const int16_t degXPos = 51;
const int16_t degYPos = tempYPos - 2;
#endif
#endif

#ifdef _ARROWINDICATORS_

#if (ORIENTATION == PORTRAIT)
const int16_t upXPos = 16;
const int16_t upYPos = yBasePos + 16;
const int16_t dnXPos = 0;
const int16_t dnYPos = yBasePos + 16;
#elif (ORIENTATION == LANDSCAPE)
const int16_t upXPos = 100;
const int16_t upYPos = yBasePos - 33;
const int16_t dnXPos = 100;
const int16_t dnYPos = yBasePos - 15;
#endif
#endif

#define SLEEPDELAY          10000U

/*
**------------------------------------------------------------------------------
** Function prototypes
**------------------------------------------------------------------------------
*/
void drawGearInfo(int16_t);
float measureT(void);

/*
**------------------------------------------------------------------------------
** Locals
**------------------------------------------------------------------------------
*/
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1, 200000, 200000);

static uint32_t changeCounter = 0;
static float temperature = 12.34;
static int16_t firstRun = true;

/*
**------------------------------------------------------------------------------
** Functions
**------------------------------------------------------------------------------
*/
/*
**------------------------------------------------------------------------------
** sleepDisplay:
**
** Sets teh display into sleep mode
**------------------------------------------------------------------------------
*/
void sleepDisplay(Adafruit_SSD1306* display)
{
    display->ssd1306_command(SSD1306_DISPLAYOFF);
}

/*
**------------------------------------------------------------------------------
** wakeDisplay:
**
** Wakes the display from sleep mode
**------------------------------------------------------------------------------
*/
void wakeDisplay(Adafruit_SSD1306* display)
{
    display->ssd1306_command(SSD1306_DISPLAYON);
}

/*
**------------------------------------------------------------------------------
** setup:
**
** See name
**------------------------------------------------------------------------------
*/
void setup(void)
{
    uint16_t i;

    pinMode(ledPin, OUTPUT);

    Serial.begin(19200);
    Wire.begin();

    temperature = measureT();

    for(i = 0 ; i < sizeof(gears)/sizeof(indicator_t) ; i++)
    {
        pinMode(gears[i].pin, INPUT_PULLUP);
    }

    //Set up the display
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c))  // Address 0x3D for 128x64, 0x3c for 128x32
    {
        for (;;)
        Serial.println(F("Channel display allocation failed")); // Don't proceed, loop forever
    }

    display.clearDisplay();
    display.setTextSize(1);      // Normal 1:1 pixel scale
    display.setTextColor(WHITE); // Draw white text
#if (ORIENTATION == PORTRAIT)
    display.setRotation(1);
#elif (ORIENTATION == LANDSCAPE)
    display.setRotation(0);
#endif
    display.cp437(true);
    #ifdef _INVERTED_DISPLAY_
    display.invertDisplay(true);
    #endif

    wakeDisplay(&display);
    drawGearInfo(1);
}

#ifdef _SESSIONCOUNTER_
void drawSessionCounter(void)
{
    static char str[10];

    //Current session counter
#if (ORIENTATION == PORTRAIT)
    display.writeLine(0, yTopTextPos + 4, 31, yTopTextPos + 4, WHITE);
    display.setCursor(0, yTopTextPos);
#elif (ORIENTATION == LANDSCAPE)
    display.writeLine(0, yTopTextPos + 15, 58, yTopTextPos + 15, WHITE);    //horizontal line
    display.writeLine(58, 0, 58, SCREEN_HEIGHT, WHITE);                     //vertical line
    display.setCursor(26, yTopTextPos+10);
#endif
    display.setFont();
    sprintf(str, "% 5lu", changeCounter);
    display.print(str);
}
#endif

#ifdef _THERMOMETER_
void drawTemperature(void)
{
    static char str[10];

    //Clear the screen area
#if (ORIENTATION == PORTRAIT)
    display.fillRect(0, tempYPos - 4, SCREEN_HEIGHT, SCREEN_WIDTH - (tempYPos - 4), BLACK);
    display.drawBitmap(degXPos, degYPos, degIcon, DEGICON_WIDTH, DEGICON_HEIGHT, WHITE);
    display.setFont();
    display.writeLine(0, tempYPos - 4, 31, tempYPos - 4, WHITE);
#elif (ORIENTATION == LANDSCAPE)
    display.fillRect(0, tempYPos - 4, 57, SCREEN_HEIGHT - tempYPos, BLACK);
    display.drawBitmap(degXPos, degYPos, degIcon, DEGICON_WIDTH, DEGICON_HEIGHT, WHITE);
    display.setFont();
    display.writeLine(0, tempYPos - 5, 58, tempYPos - 5, WHITE);            //horizontal line
    display.writeLine(58, 0, 58, SCREEN_HEIGHT, WHITE);                     //vertical line
#endif

    //Temperature
    int16_t t = temperature;
    uint16_t dec = (temperature * 10);
    dec %= 10;
    sprintf(str, "% 2d.%1u", t, dec);
#if (ORIENTATION == PORTRAIT)
    display.setCursor(0, tempYPos);
#elif (ORIENTATION == LANDSCAPE)
    display.setCursor(20, tempYPos);
#endif
    display.print(str);
}
#endif

/*
**------------------------------------------------------------------------------
** drawGearInfo:
**
** Draws everything about hte current gear
**------------------------------------------------------------------------------
*/
void drawGearInfo(int16_t gear)
{
    static char str[10];

    display.clearDisplay();

#ifdef _ARROWINDICATORS_
    //Possible changes
    if(gear == 0)
    {
        //Only up
        display.drawBitmap(upXPos, upYPos, upIcon, ARROWICON_WIDTH, ARROWICON_HEIGHT, WHITE);
    }
    else if (gear == sizeof(gears)/sizeof(indicator_t) - 1)
    {
        //Only down
        display.drawBitmap(dnXPos, dnYPos, dnIcon, ARROWICON_WIDTH, ARROWICON_HEIGHT, WHITE);
    }
    else
    {
        //Both up and down
        display.drawBitmap(upXPos, upYPos, upIcon, ARROWICON_WIDTH, ARROWICON_HEIGHT, WHITE);
        display.drawBitmap(dnXPos, dnYPos, dnIcon, ARROWICON_WIDTH, ARROWICON_HEIGHT, WHITE);
    }
#endif

    //Current gear number
#if (defined(_SESSIONCOUNTER_) || defined(_THERMOMETER_)) && (ORIENTATION == LANDSCAPE)
    //fixes a weird bug where graphic fonts are offset if mixed with the default font
    display.setCursor(gears[gear].xOffset, yBasePos-6);
#else
    display.setCursor(gears[gear].xOffset, yBasePos);
#endif
    display.setFont(&FreeSansBold24pt7b);
    sprintf(str, "%s", gears[gear].name);
    display.print(str);

#ifdef _SESSIONCOUNTER_
    drawSessionCounter();
#endif

#ifdef _THERMOMETER_
    drawTemperature();
#endif

    display.display();
}

/*
**------------------------------------------------------------------------------
** gearChanged:
**
** Checks for gear changes and keeps count of the gear changes so far
**------------------------------------------------------------------------------
*/
int16_t gearChanged(int16_t gear, int16_t lastGear)
{
    if(!digitalRead(gears[gear].pin) && (gears[gear].pin != lastGear))
    {
        if(firstRun)
        {
            firstRun = false;
        }
        else
        {
            changeCounter++;
        }
        return true;
    }
    return false;
}

/*
**------------------------------------------------------------------------------
** measureT:
**
** Reads a LM335 thermometer on A0 ands returns degrees Celsius
**------------------------------------------------------------------------------
*/
float measureT(void)
{
    uint8_t T[2];
    uint8_t *tp = T;
    const uint8_t lm = 0x4f;

    Wire.beginTransmission(lm);
    Wire.write(byte(0x00)); //read temperature register
    Wire.endTransmission();

    Wire.requestFrom(lm, 2);
    while(Wire.available())    // slave may send less than requested
    {
        *tp = Wire.read(); // receive a byte as character
        tp++;
    }
    float Tf = (T[0] * 256 + T[1]) >> 7;
    Tf /= 2.0;

    return Tf;
}
/*
**------------------------------------------------------------------------------
** loop:
**
** See name
**------------------------------------------------------------------------------
*/
void loop(void)
{
    static uint16_t checkGear = 0;
    static uint16_t lastGear = 0;
    static uint16_t sleepTimer = 0;
    static uint16_t sampleTimer = 0;
    static int sensorValue = 0;

    if(gearChanged(checkGear, lastGear))
    {
        lastGear = gears[checkGear].pin;

        wakeDisplay(&display);
        sleepTimer = 0;
        drawGearInfo(checkGear);
    }

    checkGear++;
    if( checkGear >= sizeof(gears)/sizeof(indicator_t))
    {
        checkGear = 0;
    }

    if(sleepTimer < SLEEPDELAY)
    {
        sleepTimer++;
    }
    else if(sleepTimer == SLEEPDELAY)
    {
        sleepTimer++;
        sleepDisplay(&display);
    }

    #ifdef _THERMOMETER_
    if(sampleTimer++ > SAMPLEDELAY)
    {
        sampleTimer = 0;
        temperature = measureT();
        drawTemperature();
        display.display();
    }
    #endif
    delay(10);
}
