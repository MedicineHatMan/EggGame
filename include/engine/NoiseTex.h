#ifndef _NOISETEX_H
#define _NOISETEX_H

#include "Ninja/ninja.h"

/**
 * @size 0x30
 */
struct NoiseTex {
	NoiseTex(int width, int height, s32 wrapS, s32 wrapT); // 0x80147D88
	~NoiseTex();                                           // 0x80147E30
	void flush();                                          // 0x80147E7C
	void fill(f32 freq, f32 x, f32 y, f32 amp);            // 0x80147EA8
	void load(s32 map);                                    // 0x80147FF0

	GXTexObj mTexObj; // _00
	u8* mData;        // _20
	u32 mWidth;       // _24
	u32 mHeight;      // _28
	u32 mSize;        // _2C
};

extern "C" {
void noiseInit();             // 0x801477A4
f32 noise2(f32 x, f32 y);     // 0x80147C14
}

#endif
