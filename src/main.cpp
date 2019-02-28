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
#include <fonts/FreeSansBold24pt7b.h>
#include <stdint.h>

/*
**------------------------------------------------------------------------------
** Macros
**------------------------------------------------------------------------------
*/
#define _SESSIONCOUNTER_
//#define _LIFECOUNTER_
#define _THERMOMETER_
//#define _INVERTED_DISPLAY_
#define _ARROWINDICATORS_

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

const int16_t ledPin = 13;
const int16_t yBasePos = 60;
const int16_t yBtmTextPos = 110;
const int16_t yTopTextPos = 17;

#ifdef _THERMOMETER_
#define SAMPLEDELAY         100U
const int16_t degXPos = 0;
const int16_t degYPos = yBtmTextPos - 2;
#define DEGICON_WIDTH       4
#define DEGICON_HEIGHT      4
const uint8_t PROGMEM degIcon[] =
{
    0x6f,
    0x9f,
    0x9f,
    0x6f
};
#endif

#ifdef _ARROWINDICATORS_
const int16_t upXPos = 16;
const int16_t upYPos = yBasePos + 16;
const int16_t dnXPos = 0;
const int16_t dnYPos = yBasePos + 16;

#define SLEEPDELAY          1000U
#define ARROWICON_WIDTH     16
#define ARROWICON_HEIGHT    16
const uint8_t PROGMEM upIcon[] =
{
    0x01, 0x80,
    0x03, 0xc0,
    0x07, 0xe0,
    0x0f, 0xf0,
    0x1f, 0xf8,
    0x3f, 0xfc,
    0x7f, 0xfe,
    0x7f, 0xfe,
    0x0f, 0xf0,
    0x0f, 0xf0,
    0x0f, 0xf0,
    0x0f, 0xf0,
    0x0f, 0xf0,
    0x0f, 0xf0,
    0x0f, 0xf0,
    0x07, 0xe0
};

const uint8_t PROGMEM dnIcon[] =
{
    0x07, 0xe0,
    0x0f, 0xf0,
    0x0f, 0xf0,
    0x0f, 0xf0,
    0x0f, 0xf0,
    0x0f, 0xf0,
    0x0f, 0xf0,
    0x0f, 0xf0,
    0x7f, 0xfe,
    0x7f, 0xfe,
    0x3f, 0xfc,
    0x1f, 0xf8,
    0x0f, 0xf0,
    0x07, 0xe0,
    0x03, 0xc0,
    0x01, 0x80
};
#endif



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
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1, 600000, 400000);
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
    temperature = measureT();

    Serial.begin(19200);
    Wire.begin();

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
    display.setRotation(1);
    display.cp437(true);
    #ifdef _INVERTED_DISPLAY_
    display.invertDisplay(true);
    #endif

    wakeDisplay(&display);
    drawGearInfo(1);
}

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
    display.setCursor(gears[gear].xOffset, yBasePos);
    display.setFont(&FreeSansBold24pt7b);
    sprintf(str, "%s", gears[gear].name);
    display.print(str);

    #ifdef _SESSIONCOUNTER_
    //Current session counter
    display.writeLine(0, yTopTextPos + 4, 31, yTopTextPos + 4, WHITE);

    display.setCursor(0, yTopTextPos);
    display.setFont();
    sprintf(str, "% 5lu", changeCounter);
    display.print(str);
    #endif

    #ifdef _LIFECOUNTER_
    //Total lifetime counter
    display.setCursor(0, yBtmTextPos);
    display.setFont();
    sprintf(str, "% 5lu", changeCounter);
    display.print(str);
    #endif

    #ifdef _THERMOMETER_
    //Temperature
    display.drawBitmap(degXPos, degYPos, degIcon, DEGICON_WIDTH, DEGICON_HEIGHT, WHITE);
    display.setFont();
    display.writeLine(0, yBtmTextPos - 4, 31, yBtmTextPos - 4, WHITE);

    int16_t t = temperature;
    uint16_t dec = (temperature * 10);
    dec %= 10;
    sprintf(str, "% 2d.%1u", t, dec);
    display.setCursor(0, yBtmTextPos);
    display.print(str);
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
#define CALVALUE 4.92

    float retVal = (float) analogRead(A0);
    retVal = (retVal*CALVALUE)/1024.0;

    retVal /= 0.01;
    retVal -= 273.15;

    return retVal;
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

    if(sampleTimer++ > SAMPLEDELAY)
    {
        sampleTimer = 0;
        temperature = measureT();
    }
    delay(10);
}
