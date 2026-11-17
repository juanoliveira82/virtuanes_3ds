
#ifndef _PALETTE_H_
#define _PALETTE_H_

// How many palettes there are to choose from (see palettelist.cpp).
#define NES_PALETTE_COUNT   8

#include "3ds.h"

typedef struct
{
    u8      red;
    u8      green;
    u8      blue;
} SNesPalette;

extern SNesPalette nesPaletteChoices[NES_PALETTE_COUNT][64];
extern u16         rgbaPalette[16][64];

void nespalInitialize(int choice);

#endif