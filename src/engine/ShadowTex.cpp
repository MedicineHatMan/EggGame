#include "engine/ShadowTex.h"
#include "stage/Stage.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "engine/FrameTex.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "core/SdkCalls.h"

ShadowTex* sShadowTex; // 0x802C4DE0

// 0x80041E48
ShadowTex* shadowTexGet()
{
	return sShadowTex;
}

// 0x80041E50
void shadowTexCreate()
{
	if (sShadowTex == 0) {
		sShadowTex = new ShadowTex;
	}
}

// 0x80041E94
void shadowTexDestroy()
{
	if (sShadowTex != 0) {
		delete sShadowTex;
		sShadowTex = 0;
	}
}

// 0x80041ECC
ShadowTex::ShadowTex()
{
	frameTexCreate(7, 0x80, 0x80, 0);
	frameTexSetFormat(7, 0x20);
	frameTexSetClear(7, 1);
}

// 0x80041F24
ShadowTex::~ShadowTex()
{
	frameTexDelete(7);
}

// 0x80041F70
void ShadowTex::begin(Vec3f* at)
{
	f32 proj[4][4];

	C_MTXFrustum(proj, 0.8f, -0.8f, -0.8f, 0.8f, 150.0f, 10000.0f);
	GXSetProjection(proj[0], GX_PERSPECTIVE);
	njUnitMatrix(0);
	njTranslateCameraEx(at->x, at->y, at->z);
	njRotateXCameraEx(-0x2AAA);
	njTranslateCameraEx(0.0f, 0.0f, 3000.0f);
	njGetMatrix(mView);
	GXSetViewport(0.0f, 0.0f, 128.0f, 128.0f, 0.0f, 1.0f);
	GXSetScissor(2, 2, 0x7C, 0x7C);
}

// 0x80042030
void ShadowTex::end()
{
	GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_ENABLE);
	frameTexRequest(7);
	frameTexUpdate(7);
	GXPixModeSync();
}

// 0x80042070
void ShadowTex::load(int map)
{
	frameTexSet(7);
	gjLoadTextureNum(0, 0, 0, map);
}

// 0x800420B4
void ShadowTex::loadMtx(int id)
{
	f32 light[3][4];
	f32 mtx[3][4];

	C_MTXLightFrustum(light, -0.8f, 0.8f, -0.8f, 0.8f, 150.0f, 0.5f, 0.5f, 0.5f, 0.5f);
	PSMTXConcat(light[0], mView, mtx[0]);
	GXLoadTexMtxImm(mtx[0], id, GX_MTX3x4);
}
