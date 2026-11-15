#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"
#include "Ninja/gjPrim.h"

u32 gjLineColor; // line colour
s32 gjLineMode;  // what the lines were last set up for 1 2D 2 3D

// 0x80217E24
void gjLineModeSet2D(void)
{
	if ((u32)gjLineMode ^ 1) {
		gjLineMode = 1;
		gjStartVertex2D();
	}
}

// 0x80217E5C
void gjLineModeSet3D(void)
{
	if ((u32)gjLineMode ^ 2) {
		gjLineMode = 2;
		gjStartVertex3D();
	}
}

// 0x80217E94
void njDrawLine2DExSetList(const f32* p, f32 z, s32 count)
{
	u32 col = gjLineColor;
	gjLineModeSet2D();
	GXBegin(0xA8, 4, count);
	count >>= 1;
	z = -1.0f / z;
	while (count != 0) {
		gjPrimGXPosition3f32D(p[0] * z, p[1] * z, z);
		gjPrimGXColor1u32D(col);
		gjPrimGXPosition3f32D(p[2] * z, p[3] * z, z);
		gjPrimGXColor1u32D(col);
		p += 4;
		count--;
	}
	gjPrimGXEndD();
}

// 0x80217F60
void njDrawLine3DExSetList(const f32* p, s32 count)
{
	u32 col = gjLineColor;
	gjLineModeSet3D();
	GXBegin(0xA8, 4, count);
	count >>= 1;
	while (count != 0) {
		gjPrimGXPosition3f32D(p[0], p[1], p[2]);
		gjPrimGXColor1u32D(col);
		gjPrimGXPosition3f32D(p[3], p[4], p[5]);
		gjPrimGXColor1u32D(col);
		p += 6;
		count--;
	}
	gjPrimGXEndD();
}

// 0x80218000
void njDrawLine3DExSetStrip(const NJS_POINT3* p, s32 count)
{
	u32 col = gjLineColor;
	gjLineModeSet3D();
	GXBegin(0xB0, 4, count);
	while (count != 0) {
		gjPrimGXPosition3f32D(p->x, p->y, p->z);
		gjPrimGXColor1u32D(col);
		p++;
		count--;
	}
	gjPrimGXEndD();
}

// 0x80218084
void njDrawLineExEnd(void)
{
	gjEndVertex();
}

// 0x802180A4
void njDrawLineExStart(f32 width, u32 col, s32 trans)
{
	u32 w;
	gjLineMode = 0;
	if (trans != 0) {
		gjStartVertexNonTexTrans();
	} else {
		gjStartVertexNonTexOpaque();
	}
	gjLineColor = ARGB_TO_RGBA(col);
	w           = 2.0f * (6.0f * width);
	if (w > 0xFF) {
		w = 0xFF;
	}
	GXSetLineWidth((u8)w, 0);
}

// 0x80218140
void gjPrimGXEndD(void)
{
}

// 0x80218144
void gjPrimGXColor1u32D(u32 col)
{
	GX_FIFO_U32 = col;
}

// 0x80218150
void gjPrimGXPosition3f32D(f32 x, f32 y, f32 z)
{
	GX_FIFO_F32 = x;
	GX_FIFO_F32 = y;
	GX_FIFO_F32 = z;
}
