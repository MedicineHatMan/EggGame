#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"
#include "Ninja/gjPrim.h"

// 0x80217A34
void njDrawTexture3DEx(const NJS_TEXTURE_VTX* v, s32 count, s32 trans)
{
	njDrawTexture3DExStart(trans);
	njDrawTexture3DExSetData(v, count);
	njDrawTexture3DExEnd();
}

// 0x80217A80
void njDrawTexture3DExEnd(void)
{
	gjEndVertex();
}

// 0x80217AA0
void njDrawTexture3DExSetData(const NJS_TEXTURE_VTX* v, s32 count)
{
	GXBegin(0x98, 4, count);
	while (count > 0) {
		gjPrimGXPosition3f32(v->x, v->y, v->z);
		gjPrimGXColor1u32(ARGB_TO_RGBA(v->col));
		gjPrimGXTexCoord2f32(v->u, v->v);
		v++;
		count--;
	}
	gjPrimGXEnd();
}

// 0x80217B34
void njDrawTexture3DExStart(s32 trans)
{
	gjStartVertex3D();
	if (trans != 0) {
		gjStartVertexTexTrans();
	} else {
		gjStartVertexTexOpaque();
	}
	gjSetTexture();
}

// 0x80217B78
void gjPrimGXEnd(void)
{
}

// 0x80217B7C
void gjPrimGXTexCoord2f32(f32 u, f32 v)
{
	GX_FIFO_F32 = u;
	GX_FIFO_F32 = v;
}

// 0x80217B8C
void gjPrimGXColor1u32(u32 col)
{
	GX_FIFO_U32 = col;
}

// 0x80217B98
void gjPrimGXPosition3f32(f32 x, f32 y, f32 z)
{
	GX_FIFO_F32 = x;
	GX_FIFO_F32 = y;
	GX_FIFO_F32 = z;
}
