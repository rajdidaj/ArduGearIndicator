# Arduino Primo Core on PlatformIO
Been looking for a way to build an upload a project onto a Primo Core with
PlatformIO? Well search no more!

The following files will add a Primo board to your PlatformIO setup, provided
you have the Nordic Semiconductor nfr52 platform installed.

The projects will have to be built with the following settings in your platformio.ini:

[env:Arduino_Primo_Core]

platform = nordicnrf52

board = PRIMO_CORE

framework = arduino
