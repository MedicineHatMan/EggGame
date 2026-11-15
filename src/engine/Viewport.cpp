#include "engine/Viewport.h"
#include "Ninja/ninja.h"
#include "core/SdkCalls.h"

f32 gViewW; // 0x802C4C60
f32 gViewH; // 0x802C4C64
f32 gViewX; // 0x802C4C68
f32 gViewY; // 0x802C4C6C

// 0x8001542C
extern "C" void viewportInit()
{
}

// 0x80015430
extern "C" void viewportTerm()
{
}

// 0x80015434
extern "C" void setViewport(int split)
{
	f32 x       = 0.0f;
	f32 y       = x;
	f32 w       = 640.0f;
	u32 left    = 0;
	f32 h       = 480.0f;
	u32 top     = 0;
	u32 width   = 640;
	u32 height  = 480;
	int offsetX = 0;
	int offsetY = 0;

	switch (split) {
	case 0:
		break;
	case 1:
		width >>= 1;
		offsetX = 160;
		left    = 160;
		break;
	case 2:
		width >>= 1;
		left    = 160;
		offsetX = -160;
		break;
	case 3:
		height >>= 1;
		top     = 120;
		offsetY = 120;
		break;
	case 4:
		height >>= 1;
		top     = 120;
		offsetY = -120;
		break;
	case 5:
		width >>= 1;
		height >>= 1;
		w *= 0.5f;
		h *= 0.5f;
		break;
	case 6:
		width >>= 1;
		left = width;
		height >>= 1;
		w *= 0.5f;
		h *= 0.5f;
		x = w;
		break;
	case 7:
		height >>= 1;
		top = height;
		width >>= 1;
		h *= 0.5f;
		left = 0;
		w *= 0.5f;
		y = h;
		break;
	case 8:
		width >>= 1;
		height >>= 1;
		w *= 0.5f;
		left = width;
		h *= 0.5f;
		top = height;
		x   = w;
		y   = h;
		break;
	}

	if (_gj_rmode_->field_rendering) {
		GXSetViewportJitter(x, y, w, h, 0.0f, 1.0f, VIGetNextField());
	} else {
		GXSetViewport(x, y, w, h, 0.0f, 1.0f);
	}
	GXSetScissor(left, top, width, height);
	GXSetScissorBoxOffset(offsetX, offsetY);
	gViewX = x;
	gViewY = y;
	gViewW = w;
	gViewH = h;
}

// 0x80015620
extern "C" void getViewport(f32* x, f32* y, f32* w, f32* h)
{
	if (x != 0) {
		*x = gViewX;
	}
	if (y != 0) {
		*y = gViewY;
	}
	if (w != 0) {
		*w = gViewW;
	}
	if (h != 0) {
		*h = gViewH;
	}
}

// 0x80015664
// clang-format off
extern "C" asm f32 quatNormalize(register Quat* q)
{
	nofralloc
	psq_l f0, 0x0(q), 0, 0
	psq_l f1, 0x8(q), 0, 0
	ps_mul f2, f0, f0
	ps_madd f2, f1, f1, f2
	ps_sum0 f2, f2, f2, f2
	frsqrte f2, f2
	ps_muls0 f3, f0, f2
	ps_muls0 f4, f1, f2
	psq_st f3, 0x0(q), 0, 0
	fres f1, f2
	psq_st f4, 0x8(q), 0, 0
	blr
}
// clang-format on

// 0x80015694
// clang-format off
extern "C" asm void quatScale(register Quat* q, register f32 scale)
{
	nofralloc
	psq_l f2, 0x0(q), 0, 0
	psq_l f3, 0x8(q), 0, 0
	ps_muls0 f2, f2, scale
	ps_muls0 f3, f3, scale
	psq_st f2, 0x0(q), 0, 0
	psq_st f3, 0x8(q), 0, 0
	blr
}
// clang-format on

// 0x800156B0
// clang-format off
extern "C" asm void quatAdd(register Quat* q, register const Quat* add)
{
	nofralloc
	psq_l f0, 0x0(q), 0, 0
	psq_l f1, 0x8(q), 0, 0
	psq_l f2, 0x0(add), 0, 0
	psq_l f3, 0x8(add), 0, 0
	ps_add f0, f0, f2
	ps_add f1, f1, f3
	psq_st f0, 0x0(q), 0, 0
	psq_st f1, 0x8(q), 0, 0
	blr
}
// clang-format on
