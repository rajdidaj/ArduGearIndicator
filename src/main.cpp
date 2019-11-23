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
#include <stdarg.h>
#include <bitmaps.h>

/*
**------------------------------------------------------------------------------
** Macros
**------------------------------------------------------------------------------
*/
#define PORTRAIT 0
#define LANDSCAPE 1

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
} indicator_t;

typedef union {
    uint8_t u8[2];
    uint16_t u16;
    int16_t s16;
} b16_t;

/*
**------------------------------------------------------------------------------
** Constants
**------------------------------------------------------------------------------
*/

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#if (ORIENTATION == PORTRAIT)
const indicator_t gears[7] =
    {
        {"1", 2, 4},
        {"N", 3, 0},
        {"2", 4, 4},
        {"3", 5, 4},
        {"4", 6, 4},
        {"5", 7, 4},
        {"6", 8, 4}};
#elif (ORIENTATION == LANDSCAPE)
const indicator_t gears[7] =
    {
        {"1", 2, 64},
        {"N", 0, 60},
        {"2", 4, 64},
        {"3", 5, 64},
        {"4", 6, 64},
        {"5", 7, 64},
        {"6", 8, 64}};
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
#define SAMPLEDELAY 100U
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

#define SLEEPDELAY 10000U

/*
**------------------------------------------------------------------------------
** Function prototypes
**------------------------------------------------------------------------------
*/
void drawGearInfo(int16_t);
float measureT(void);
void swoprintf(const char *, ...);

/*
**------------------------------------------------------------------------------
** Locals
**------------------------------------------------------------------------------
*/
TwoWire Wire0(NRF_TWIM0, NRF_TWIS0, SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQn, PIN_WIRE_SDA, PIN_WIRE_SCL);
TwoWire Wire1(NRF_TWIM1, NRF_TWIS1, SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQn, PIN_WIRE_SDA1, PIN_WIRE_SCL1);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire0, -1, 200000, 200000);

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
void sleepDisplay(Adafruit_SSD1306 *display)
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
void wakeDisplay(Adafruit_SSD1306 *display)
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

#define ENABLE_SWO
    // Enable SWO
    NRF_CLOCK->TRACECONFIG = (CLOCK_TRACECONFIG_TRACEPORTSPEED_4MHz << CLOCK_TRACECONFIG_TRACEPORTSPEED_Pos);

    NRF_CLOCK->TRACECONFIG |= CLOCK_TRACECONFIG_TRACEMUX_Serial << CLOCK_TRACECONFIG_TRACEMUX_Pos;

    ITM->TCR |= 1;
    ITM->TER |= 1;

    Serial.begin(19200);
    Wire0.begin();
    Wire1.begin();

    temperature = measureT();

    for (i = 0; i < sizeof(gears) / sizeof(indicator_t); i++)
    {
        pinMode(gears[i].pin, INPUT_PULLUP);
    }

    //Set up the display
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c)) // Address 0x3D for 128x64, 0x3c for 128x32
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
    display.writeLine(0, yTopTextPos + 15, 58, yTopTextPos + 15, WHITE); //horizontal line
    display.writeLine(58, 0, 58, SCREEN_HEIGHT, WHITE);                  //vertical line
    display.setCursor(26, yTopTextPos + 10);
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
    display.writeLine(0, tempYPos - 5, 58, tempYPos - 5, WHITE); //horizontal line
    display.writeLine(58, 0, 58, SCREEN_HEIGHT, WHITE);          //vertical line
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
    if (gear == 0)
    {
        //Only up
        display.drawBitmap(upXPos, upYPos, upIcon, ARROWICON_WIDTH, ARROWICON_HEIGHT, WHITE);
    }
    else if (gear == sizeof(gears) / sizeof(indicator_t) - 1)
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
    display.setCursor(gears[gear].xOffset, yBasePos - 6);
#else
    display.setCursor(gears[gear].xOffset, yBasePos);
#endif
    display.setFont(&FreeSansBold24pt7b);
    sprintf(str, "%s", gears[gear].name);
    display.print(str);
    swoprintf("New gear: %s\n", gears[gear].name);

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
    if (!digitalRead(gears[gear].pin) && (gears[gear].pin != lastGear))
    {
        if (firstRun)
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
** readLM75:
**
** Reads a LM75 thermometer return degrees Celsius
**------------------------------------------------------------------------------
*/
float readLM75(void)
{
    const uint8_t addr = 0x4f;
    const uint8_t T_REG = 0x00;
    const uint8_t T_SIZE = 2;

    b16_t b16 = {0};
    uint8_t *p = &b16.u8[T_SIZE - 1];

    Wire0.beginTransmission(addr);
    Wire0.write(byte(T_REG)); //read temperature register
    Wire0.endTransmission();

    Wire0.requestFrom(addr, T_SIZE);
    while (Wire0.available()) // slave may send less than requested
    {
        *p = Wire0.read(); // receive a byte as character
        p--;
    }

    return b16.s16 / 256.0;
}

/*
**------------------------------------------------------------------------------
** readHTS221:
**
** Reads a readHTS221 thermometer return degrees Celsius
**------------------------------------------------------------------------------
*/
float readHTS221(void)
{
    const uint8_t addr = 0x5f;
    const uint8_t T_REG = 0x2a + 0x80;
    const uint8_t T_SIZE = 2;
    const uint8_t C_REG = 0x30 + 0x80;
    const uint8_t C_SIZE = 16;
    
    static bool startup = true;

    static struct 
    {
        uint8_t H0_rH_x2;
        uint8_t H1_rH_x2;
        uint8_t T0_degC_x8;
        uint8_t T1_degC_x8;
        uint8_t : 8;
        uint8_t T1_T0msb;
        int16_t H0_T0_OUT;
        uint16_t : 16;
        int16_t H1_T0_OUT;
        int16_t T0_OUT;
        int16_t T1_OUT;
    }cal;

    uint8_t *p = NULL;
    int16_t T_OUT = 0;

    if(startup)
    {
        startup = false;
        
        // Read the calibration registers
        p = (uint8_t*)&cal;
        Wire1.beginTransmission(addr);
        Wire1.write(byte(C_REG)); //read temperature register
        Wire1.endTransmission();
        Wire1.requestFrom(addr, C_SIZE);
        while (Wire1.available()) // slave may send less than requested
        {
            *p = Wire1.read();
            p++;
        }
        swoprintf("T0_O: %d, T1_O: %d\n", cal.T0_OUT, cal.T1_OUT);
    }
    p = (uint8_t*)&T_OUT;

    Wire1.beginTransmission(addr);
    Wire1.write(byte(T_REG)); //read temperature register
    Wire1.endTransmission();
    Wire1.requestFrom(addr, T_SIZE);
    while (Wire1.available()) // slave may send less than requested
    {
        *p = Wire1.read();
        p++;
    }

    float T0_degC = (cal.T0_degC_x8 + (1 << 8) * (cal.T1_T0msb & 0x03)) / 8.0; 
    float T1_degC = (cal.T1_degC_x8 + (1 << 6) * (cal.T1_T0msb & 0x0C)) / 8.0; // Value is in 3rd and fourth bit, so we only need to shift this value 6 more bits.
    float T_DegC = (T0_degC + (T_OUT - cal.T0_OUT) * (T1_degC - T0_degC) / (cal.T1_OUT - cal.T0_OUT)); 

    return T_DegC;
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

    // External sensor
    float Te = readLM75();
    swoprintf("Te: %d\n", (int32_t)Te);

    // Internal sensor
    // float Ti = readHTS221();
    // swoprintf("Ti: %d\n", (int32_t)Ti);  
    // return (Ti + Te) / 2.0;

    return Te;
}

/*
**------------------------------------------------------------------------------
** swoprintf:
**
** Like printf but for swo
**------------------------------------------------------------------------------
*/
void swoprintf(const char *format, ...)
{
    static char tmpStr[128] = {0};

    va_list argptr;
    va_start(argptr, format);
    vsprintf(tmpStr, format, argptr);
    va_end(argptr);

    for (uint32_t i = 0; i < strlen(tmpStr) && i < sizeof(tmpStr); i++)
    {
        ITM_SendChar(tmpStr[i]);
    }
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

    if (gearChanged(checkGear, lastGear))
    {
        lastGear = gears[checkGear].pin;

        wakeDisplay(&display);
        sleepTimer = 0;
        drawGearInfo(checkGear);
    }

    checkGear++;
    if (checkGear >= sizeof(gears) / sizeof(indicator_t))
    {
        checkGear = 0;
    }

    if (sleepTimer < SLEEPDELAY)
    {
        sleepTimer++;
    }
    else if (sleepTimer == SLEEPDELAY)
    {
        sleepTimer++;
        sleepDisplay(&display);
    }

#ifdef _THERMOMETER_
    if (sampleTimer++ > SAMPLEDELAY)
    {
        sampleTimer = 0;
        temperature = measureT();
        drawTemperature();
        display.display();
    }
#endif
    delay(10);
}
