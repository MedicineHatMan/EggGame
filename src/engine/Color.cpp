#include "engine/Color.h"

u32 gColorWhite = ColorValue(0xFFFFFFFF).mValue;
u32 gColorBlack = ColorValue(0x000000FF).mValue;
u32 gColorRed   = ColorValue(0xFF0000FF).mValue;
u32 gColorGreen = ColorValue(0x00FF00FF).mValue;
u32 gColorBlue  = ColorValue(0x0000FFFF).mValue;

static int addClamped(u8* channel, int step);
static void lerpColorPS(const u32* from, const u32* to, u32* out, f32 t);

// 0x80044E90
void lerpColor(f32 t, u32* out, const u32* from, const u32* to)
{
	lerpColorPS(from, to, out, t);
}

// 0x80044EC4
void setColorRed(u32* color, u8 value)
{
	*color = __rlwimi(*color, value, 24, 0, 7);
}

// 0x80044ED4
void setColorAlpha(u32* color, int value)
{
	*color = __rlwimi(*color, (u8)value, 0, 24, 31);
}

// 0x80044EE4
static int addClamped(u8* channel, int step)
{
	int value = *channel + step;
	if (value <= 0) {
		*channel = 0;
		return 1;
	}
	if (value >= 0xFF) {
		*channel = 0xFF;
		return 1;
	}
	*channel = value;
	return 0;
}

// 0x80044F24
int addColorRed(u32* color, int step)
{
	u8 c       = *color >> 24;
	int result = addClamped(&c, step);
	*color     = __rlwimi(*color, c, 24, 0, 7);
	return result;
}

// 0x80044F70
int addColorGreen(u32* color, int step)
{
	u8 c       = *color >> 16;
	int result = addClamped(&c, step);
	*color     = (*color & 0xFF00FFFF) | ((c << 16) & 0xFF0000);
	return result;
}

// 0x80044FBC
int addColorBlue(u32* color, int step)
{
	u8 c       = *color >> 8;
	int result = addClamped(&c, step);
	*color     = (*color & 0xFFFF00FF) | ((c << 8) & 0xFF00);
	return result;
}

// 0x80045008
int addColorAlpha(u32* color, int step)
{
	u8 c       = *color;
	int result = addClamped(&c, step);
	*color     = __rlwimi(*color, c, 0, 24, 31);
	return result;
}

// 0x80045050
// Each channel as a u8 through GQR1 out = from + to - from * t
// clang-format off
static asm void lerpColorPS(const u32* from, const u32* to, u32* out, f32 t)
{
	nofralloc
	li r0, 0x804
	oris r0, r0, 0x804
	mtspr 0x391, r0
	psq_l f3, 0x0(r3), 0, 1
	psq_l f4, 0x0(r4), 0, 1
	ps_sub f4, f4, f3
	psq_l f5, 0x2(r3), 0, 1
	ps_madds0 f4, f4, f1, f3
	psq_l f6, 0x2(r4), 0, 1
	ps_sub f6, f6, f5
	psq_st f4, 0x0(r5), 0, 1
	ps_madds0 f6, f6, f1, f5
	psq_st f6, 0x2(r5), 0, 1
	blr
}
// clang-format on
