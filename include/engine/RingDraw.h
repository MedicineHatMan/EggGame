#ifndef _RINGDRAW_H
#define _RINGDRAW_H

#include "core/types.h"

/**
 * @size 0x10
 */
struct RingDraw {
	RingDraw();  // 0x800F322C
	~RingDraw(); // 0x800F324C

	static f32 getWidth(); // 0x800F3288 how thick the hoop is
	void draw();           // 0x800F3290
	void drawHoop();       // 0x800F3414
	void drawShadow();     // 0x800F36C4
	void drawShadowHoop(); // 0x800F37C0

	f32 mRadius;     // _00
	int mLod;        // _04 0 near 1 middle 2 far
	int mShadowLod;  // _08
	u32 mColor;      // _0C RGBA
};

extern "C" {
// A tube of n sides along z from mtx from z0 to z1 radius wide
void drawTube(const f32* mtx, u8 n, const u32* color, f32 z0, f32 z1, f32 radius);
void ringDrawInit(); // 0x800F2B3C
void ringDrawTerm(); // 0x800F2B5C
}

#endif
