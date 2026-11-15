#ifndef _COLOR_H
#define _COLOR_H

#include "core/types.h"

struct ColorValue {
	ColorValue(u32 value) { mValue = value; }

	u32 mValue;
};

extern "C" {
extern u32 gColorWhite; // 0xFFFFFFFF
extern u32 gColorBlack; // 0x000000FF
extern u32 gColorRed;   // 0xFF0000FF
extern u32 gColorGreen; // 0x00FF00FF
extern u32 gColorBlue;  // 0x0000FFFF

void lerpColor(f32 t, u32* out, const u32* from, const u32* to); // 0x80044E90
void setColorRed(u32* color, u8 value);                       // 0x80044EC4
void setColorAlpha(u32* color, int value);                        // 0x80044ED4

int addColorRed(u32* color, int step);
int addColorGreen(u32* color, int step);
int addColorBlue(u32* color, int step);
int addColorAlpha(u32* color, int step);
}

#endif
