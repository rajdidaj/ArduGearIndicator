# ArduGearIndicator
This is a small Arduino based gear indicator.
It relies on Adafruit_SSD1306 and Adafruit_GFX libraries to display text and
images on a small Adafruit SSD1306 OLED display.

It is small enough to fit in a ATMega 328P (RAM-wise, the display buffer is a bit
of a memory hog).

# Gear detection
This program is made for a machine that is capable of indicating the selected gear
by pulling an input low; one input per gear.

The gears are remappable per input pin.

# Optional thermometer
This program can also use a Adafruit ADS1015 ADC to read a thermometer. I've
used a LM355 thermometer for this.

# Arduino Primo Core
I've planned using a Primo Core to run this program, which is bit of a mess right now.
The device is very small and power efficient, has all the required inputs, outputs
and memory (and then some).

I've included the required files to make this project viable in PlatformIO.
