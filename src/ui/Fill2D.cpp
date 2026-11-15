#include "core/types.h"
#include "Ninja/ninja.h"
#include "sdk/Dolphin/gx/GXEnum.h"

extern "C" {
u32 sFillTop;    // 0x802C4DF8 GX's RGBA
u32 sFillBottom; // 0x802C4DFC

// 0x80042BF4
void fill2DBegin(int trans)
{
	gjStartVertex2D();
	if (trans != 0) {
		gjStartVertexNonTexTrans();
	} else {
		gjStartVertexNonTexOpaque();
	}
}

// 0x80042C34
void fill2DEnd()
{
	gjEndVertex();
}

// 0x80042C54
// color is ARGB as the game keeps them
void fill2DSetColor(u32 color)
{
	sFillTop = sFillBottom = (color << 8) | (color >> 24);
}

// 0x80042C68
// rect is x0 y0 x1 y1 drawn at depth z
void fill2DRect(f32 z, const f32* rect, int)
{
	f32 x0, x1, y0, y1, w;
	w  = -1.0f / z;
	x0 = rect[0] * w;
	x1 = rect[2] * w;
	y0 = rect[1] * w;
	y1 = rect[3] * w;
	u32 top    = sFillTop;
	u32 bottom = sFillBottom;

	GXBegin(GX_QUADS, GX_VTXFMT4, 4);
	GX_FIFO_F32 = x0;
	GX_FIFO_F32 = y0;
	GX_FIFO_F32 = w;
	GX_FIFO_U32 = top;
	GX_FIFO_F32 = x1;
	GX_FIFO_F32 = y0;
	GX_FIFO_F32 = w;
	GX_FIFO_U32 = top;
	GX_FIFO_F32 = x1;
	GX_FIFO_F32 = y1;
	GX_FIFO_F32 = w;
	GX_FIFO_U32 = bottom;
	GX_FIFO_F32 = x0;
	GX_FIFO_F32 = y1;
	GX_FIFO_F32 = w;
	GX_FIFO_U32 = bottom;
}
}
