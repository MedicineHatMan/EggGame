#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"

typedef struct NJS_QUAD_TEXTURE_EX {
	f32 x, y, z;  // _00
	f32 vx1, vy1; // _0C
	f32 vx2, vy2; // _14
	f32 _1C;      // _1C
	f32 u, v;     // _20
	f32 vu1, vv1; // _28
	f32 vu2, vv2; // _30
} NJS_QUAD_TEXTURE_EX;

// Not defined here the DOL keeps the .bss pair ahead of njTexLoad.c's and the .sbss ones on both sides of syLib.c's so
// data-only files own them
extern f32 gjIdentityMatrix[12]; // the first matrix on the stack
extern GXLightObj gjLightObjs[8];
extern s32 lbl_802C6208;
extern s32 lbl_802C620C;
extern f32 gjLight0Dist;   // how far away light 0 is
extern u32 gjQuadSpecular; // specular for quads
extern u32 gjQuadColor;    // colour for quads
void njMiscGXTexCoord2f32(f32 u, f32 v);
void njMiscGXColor1u32(u32 col);
void njMiscGXPosition3f32(f32 x, f32 y, f32 z);
void njMiscGXEnd(void);

// 0x80215E70
// The stack lives in the locked cache at 0xE0000000
void gjInitMatrix(s32 unused, s32 depth)
{
	gjMatrixStack    = (f32*)0xE0000000;
	_gj_matrix_      = (f32*)0xE0000000;
	gjMatrixStackEnd = (f32*)((depth - 1) * sizeof(NJS_MATRIX) + 0xE0000000);
	PSMTXIdentity(gjIdentityMatrix == NULL ? (f32*)0xE0000000 : gjIdentityMatrix);
	syMemCopy(gjMatrixStack, gjIdentityMatrix, sizeof(NJS_MATRIX));
}

// 0x80215ED4
void njMiscInit(void)
{
	_nj_control_3d_flag_      = 0;
	lbl_802C620C              = -1;
	lbl_802C6208              = 0;
	_nj_constant_material_[0] = 255.0f;
	_nj_constant_material_[1] = 0.0f;
	_nj_constant_material_[2] = 0.0f;
	_nj_constant_material_[3] = 0.0f;
	gjLightsOn                = 0;
	gjLight0Dist              = 100000000.0f;
	GXInitLightAttnK(gjLightObjs, 1.0f, 0.0f, 0.0f);
}

// 0x80215F44
void njSetConstantMaterial(const f32* c)
{
	_nj_constant_material_[0] = c[0];
	_nj_constant_material_[1] = c[1];
	_nj_constant_material_[2] = c[2];
	_nj_constant_material_[3] = c[3];
}

// 0x80215F70
void njQuadTextureStart(s32 trans)
{
	gjStartVertex2D();
	if (trans != 0) {
		gjStartVertexTexTransEx();
	} else {
		gjStartVertexTexOpaqueEx();
	}
}

// 0x80215FB0
void njQuadTextureEnd(void)
{
	gjEndVertex();
}

// 0x80215FD0
void njQuadVertex(f32 x, f32 y, f32 z, u32 col, u32 spec, f32 u, f32 v)
{
	njMiscGXPosition3f32(x, y, z);
	njMiscGXColor1u32(col);
	njMiscGXColor1u32(spec);
	njMiscGXTexCoord2f32(u, v);
}

// 0x8021603C
void njDrawQuadTexture(const NJS_QUAD_TEXTURE* q, f32 z)
{
	u32 col  = gjQuadColor;
	u32 spec = gjQuadSpecular;
	f32 w    = -1.0f / z;
	f32 x1   = q->x1 * w;
	f32 x2   = q->x2 * w;
	f32 y1   = q->y1 * w;
	f32 y2   = q->y2 * w;

	GXBegin(0x80, 4, 4);
	njQuadVertex(x1, y1, w, col, spec, q->u1, q->v1);
	njQuadVertex(x2, y1, w, col, spec, q->u2, q->v1);
	njQuadVertex(x2, y2, w, col, spec, q->u2, q->v2);
	njQuadVertex(x1, y2, w, col, spec, q->u1, q->v2);
	njMiscGXEnd();
}

// 0x80216188
void njDrawQuadTextureEx(const NJS_QUAD_TEXTURE_EX* q)
{
	f32 w    = -1.0f / q->z;
	u32 col  = gjQuadColor;
	u32 spec = gjQuadSpecular;
	f32 x;
	f32 y;
	f32 u;
	f32 v;
	f32 cx;
	f32 cy;
	f32 cu;
	f32 cv;

	GXBegin(0x80, 4, 4);
	x = q->x * w;
	y = q->y * w;
	u = q->u;
	v = q->v;
	njQuadVertex(x, y, w, col, spec, u, v);
	cx = x + q->vx2 * w;
	cy = y + q->vy2 * w;
	cu = u + q->vu2;
	cv = v + q->vv2;
	njQuadVertex(cx, cy, w, col, spec, cu, cv);
	cx += q->vx1 * w;
	cy += q->vy1 * w;
	cu += q->vu1;
	cv += q->vv1;
	njQuadVertex(cx, cy, w, col, spec, cu, cv);
	njQuadVertex(x + q->vx1 * w, y + q->vy1 * w, w, col, spec, u + q->vu1, v + q->vv1);
	njMiscGXEnd();
}

// 0x80216378
void njSetQuadTexture(s32 num, u32 col)
{
	gjLoadTextureNum(num, 0, 0, 0);
	gjQuadColor    = ARGB_TO_RGBA(col);
	gjQuadSpecular = 0;
}

// 0x802163D0
void njSetQuadTextureColor(u32 col)
{
	gjQuadColor    = ARGB_TO_RGBA(col);
	gjQuadSpecular = 0;
}

// 0x802163F4
void njMiscGXTexCoord2f32(f32 u, f32 v)
{
	GX_FIFO_F32 = u;
	GX_FIFO_F32 = v;
}

// 0x80216404
void njMiscGXColor1u32(u32 col)
{
	GX_FIFO_U32 = col;
}

// 0x80216410
void njMiscGXPosition3f32(f32 x, f32 y, f32 z)
{
	GX_FIFO_F32 = x;
	GX_FIFO_F32 = y;
	GX_FIFO_F32 = z;
}

// 0x80216424
void njMiscGXEnd(void)
{
}
