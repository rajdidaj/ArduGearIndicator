/*
**------------------------------------------------------------------------------
** Here be bitmaps
**------------------------------------------------------------------------------
*/

#ifndef _BITMAPS_H_
#define _BITMAPS_H_

#define DEGICON_WIDTH       4
#define DEGICON_HEIGHT      4
const uint8_t PROGMEM degIcon[] =
{
    0x6f,
    0x9f,
    0x9f,
    0x6f
};

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

#endif  /* _BITMAPS_H_ */
