#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"

typedef struct NJS_POLYGON_VTX {
	f32 x, y, z;
	u32 col;
} NJS_POLYGON_VTX;

void njMiscGXEnd2(void);
void njMiscGXColor1u32B(u32 col);
void njMiscGXPosition3f32B(f32 x, f32 y, f32 z);

// 0x80216798
void njDrawPolygon3DExEnd(void)
{
	gjEndVertex();
}

// 0x802167B8
void njDrawPolygon3DExSetData(const NJS_POLYGON_VTX* v, s32 count)
{
	GXBegin(0x98, 4, count);
	while (count != 0) {
		njMiscGXPosition3f32B(v->x, v->y, v->z);
		njMiscGXColor1u32B(ARGB_TO_RGBA(v->col));
		v++;
		count--;
	}
	njMiscGXEnd2();
}

// 0x80216840
void njDrawPolygon3DExStart(s32 trans)
{
	gjStartVertex3D();
	if (trans != 0) {
		gjStartVertexNonTexTrans();
	} else {
		gjStartVertexNonTexOpaque();
	}
}

// 0x80216880
void njMiscGXEnd2(void)
{
}

// 0x80216884
void njMiscGXColor1u32B(u32 col)
{
	GX_FIFO_U32 = col;
}

// 0x80216890
void njMiscGXPosition3f32B(f32 x, f32 y, f32 z)
{
	GX_FIFO_F32 = x;
	GX_FIFO_F32 = y;
	GX_FIFO_F32 = z;
}
