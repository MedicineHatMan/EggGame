#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"

f32 gjViewport[6] = {
	0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f,
};

// 0x8020CBB4
void njInitCameraEx(void)
{
	njUnitMatrix(NULL);
}

// 0x8020CBD8
void njRotateXCameraEx(s32 ang)
{
	NJS_MATRIX r;
	PSMTXRotRad(r, 'x', 9.587378008291125e-05 * -ang);
	PSMTXConcat(r, _gj_matrix_, _gj_matrix_);
}

// 0x8020CC40
void njTranslateCameraEx(f32 x, f32 y, f32 z)
{
	f32* m = _gj_matrix_;
	m[3] -= x;
	m[7] -= y;
	m[11] -= z;
}

// 0x8020CC6C
void njCameraExLookAt(const NJS_POINT3* pos, const NJS_POINT3* at, const NJS_VECTOR* up)
{
	C_MTXLookAt(_gj_matrix_, pos, up, at);
}

// 0x8020CC9C
// Projects a point in view space
s32 njCalcScreen(const NJS_POINT3* p, f32* sx, f32* sy)
{
	NJS_MATRIX m;
	f32 proj[7];
	f32 sz;
	PSMTXIdentity(m);
	GXGetProjectionv(proj);
	GXProject(p->x, p->y, p->z, m, proj, gjViewport, sx, sy, &sz);
	if (sz < njProjection[1] || sz > njProjection[0] || *sx < njProjection[6] || *sx > njProjection[8] || *sy < njProjection[7]
	    || *sy > njProjection[9]) {
		return 0;
	}
	return 1;
}
