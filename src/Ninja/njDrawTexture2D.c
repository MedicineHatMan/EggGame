#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"
#include "Ninja/gjPrim.h"

// 0x80217CD8
void njDrawTexture2DExEnd(void)
{
	gjEndVertex();
}

// 0x80217CF8
void njDrawTexture2DExSetData(const NJS_TEXTURE_VTX* v, s32 count)
{
	GXBegin(0x98, 4, count);
	while (count > 0) {
		f32 w = -1.0f / v->z;
		gjPrimGXPosition3f32C(v->x * w, v->y * w, w);
		gjPrimGXColor1u32C(ARGB_TO_RGBA(v->col));
		gjPrimGXTexCoord2f32C(v->u, v->v);
		v++;
		count--;
	}
	gjPrimGXEndC();
}

// 0x80217DAC
void njDrawTexture2DExStart(s32 trans)
{
	gjStartVertex2D();
	if (trans != 0) {
		gjStartVertexTexTrans();
	} else {
		gjStartVertexTexOpaque();
	}
	gjSetTexture();
}

// 0x80217DF0
void gjPrimGXEndC(void)
{
}

// 0x80217DF4
void gjPrimGXTexCoord2f32C(f32 u, f32 v)
{
	GX_FIFO_F32 = u;
	GX_FIFO_F32 = v;
}

// 0x80217E04
void gjPrimGXColor1u32C(u32 col)
{
	GX_FIFO_U32 = col;
}

// 0x80217E10
void gjPrimGXPosition3f32C(f32 x, f32 y, f32 z)
{
	GX_FIFO_F32 = x;
	GX_FIFO_F32 = y;
	GX_FIFO_F32 = z;
}
