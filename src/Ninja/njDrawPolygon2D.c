#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"
#include "Ninja/gjPrim.h"

// 0x80217BAC
void njDrawPolygon2DExEnd(void)
{
	gjEndVertex();
}

// 0x80217BCC
void njDrawPolygon2DExSetData(const NJS_POLYGON_VTX* v, s32 count)
{
	GXBegin(0x98, 4, count);
	while (count != 0) {
		f32 w = -1.0f / v->z;
		gjPrimGXPosition3f32B(v->x * w, v->y * w, w);
		gjPrimGXColor1u32B(ARGB_TO_RGBA(v->col));
		v++;
		count--;
	}
	gjPrimGXEndB();
}

// 0x80217C74
void njDrawPolygon2DExStart(s32 trans)
{
	gjStartVertex2D();
	if (trans != 0) {
		gjStartVertexNonTexTrans();
	} else {
		gjStartVertexNonTexOpaque();
	}
}

// 0x80217CB4
void gjPrimGXEndB(void)
{
}

// 0x80217CB8
void gjPrimGXColor1u32B(u32 col)
{
	GX_FIFO_U32 = col;
}

// 0x80217CC4
void gjPrimGXPosition3f32B(f32 x, f32 y, f32 z)
{
	GX_FIFO_F32 = x;
	GX_FIFO_F32 = y;
	GX_FIFO_F32 = z;
}
