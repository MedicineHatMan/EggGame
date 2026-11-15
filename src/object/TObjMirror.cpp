#include "object/ObjAction.h"
#include "engine/DrawTask.h"
#include "stage/SetManager.h"
#include "egg/ObjEgg.h"
#include "camera/TCamera.h"
#include "engine/GenTex.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "Ninja/ninja.h"
#include "engine/FrameTex.h"
#include "stage/SetSlot.h"
#include "camera/TCameraCtrl.h"
#include "object/TObjMirror.h"
struct SetLink;
struct DrawTask;
struct CamView;

typedef char TObjMirror_size_check[(sizeof(TObjMirror) == 0xA4) ? 1 : -1];

// 0x80076060
extern "C" TObject* createTObjMirror(TObject* parent, SetDesc* desc)
{
	return new TObjMirror(parent, desc);
}

// 0x800760B4
void TObjMirror::reset()
{
	mAct = 2;
}

const char* gTObjMirrorName = "TObjMirror";

// The mirror quad top edge first
extern const Vec3f sMirrorCorners[4] = {
	{ -20.0f, 100.0f, 0.0f },
	{ 20.0f, 100.0f, 0.0f },
	{ 20.0f, 0.0f, 0.0f },
	{ -20.0f, 0.0f, 0.0f },
};

// 0x800760C0
TObjMirror::TObjMirror(TObject* parent, SetDesc* desc)
    : TObjAction(parent)
{
	setObjId(this, 0xFFFF);
	_00   = (u32)gTObjMirrorName;
	mSize = 0xA4;
	mLink.get()->attach(desc);
	initFromDesc(desc);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x2A, 0);
	reset();
}

// 0x8007618C
TObjMirror::~TObjMirror()
{
	removeDrawTask(mDrawTask);
	notifySetLinkDeath((SetLink*)&mLink, (void*)this);
	releaseObjId(this);
}

// 0x80076214
// The corners are placed once from the record's place and rotation
void TObjMirror::initFromDesc(SetDesc* desc)
{
	desc->getPos(&mPos);
	desc->getRot(&mRot);

	njUnitMatrix(0);
	njTranslateEx(&mPos);
	gjRotateY(mRot.y);
	gjRotateX(mRot.x);
	gjRotateZ(mRot.z);
	for (int i = 0; i < 4; i++) {
		Vec3f corner;
		corner = sMirrorCorners[i];
		njCalcPoint(0, &corner, (Vec3f*)&mCorners[i]);
	}

	mPattern     = desc->getInt(0) % 3;
	mScrollSpeed = 0.01f + desc->getFloat(3);
}

// Not in the DOL the linker strips it
f32 mirrorIntToFloat(int x)
{
	return (f32)x;
}

// 0x80076338
void TObjMirror::debugPrint(void*)
{
	njPrintC(0xA0013, ":TEV PATTERN");
	njPrintC(0xA001A, "+0.01:SCROLL SPEED(0.0~1.0)");
}

// 0x8007637C
void TObjMirror::draw()
{
	frameTexRequestCur();
}

// 0x8007639C
void TObjMirror::vfn_10()
{
}

// 0x800763A0
void TObjMirror::render()
{
	drawQuad();
}

// 0x800763C0
void TObjMirror::drawQuad()
{
	njPushMatrixEx();
	GXSetCullMode(GX_CULL_NONE);
	gjLoadPosMatrix(0);
	gjLoadNrmMatrix(0);
	GXSetCurrentMtx(GX_PNMTX0);

	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

	GXColor color = { 0xFF, 0xFF, 0xFF, 0xFF };
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
	GXSetChanMatColor(GX_COLOR0A0, color);
	GXSetNumTexGens(2);
	njPushMatrixEx();

	// Project the screen on to the quad with a field of view slightly narrower than the camera's
	CamView* camera = (CamView*)camGetView(0);
	f32 fovY   = 0.75f * (360.0f * ((f32)camera->mFov / 65536.0f));
	f32 proj[12];
	C_MTXLightPerspective((float*)proj, fovY, camera->mAspect, -0.5f, -0.5f, 0.5f, 0.5f);
	f32 texMtx[12];
	PSMTXConcat(proj, _gj_matrix_, texMtx);
	GXLoadTexMtxImm(texMtx, GX_TEXMTX1, GX_MTX3x4);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3X4, GX_TG_POS, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
	GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2X4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);

	genTexLoad(4, 1);
	frameTexSetCur();
	gjLoadTextureNum(0, 0, 0, 0);

	GXSetNumTevStages(1);
	GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

	// The indirect stage that makes the reflection ripple
	GXSetNumIndStages(1);
	GXSetIndTexOrder(GX_IND_TEX_STAGE_0, GX_TEXCOORD1, GX_TEXMAP1);
	GXSetIndTexCoordScale(GX_IND_TEX_STAGE_0, GX_ITS_1, GX_ITS_1);
	GXSetTevIndWarp(GX_TEVSTAGE0, GX_IND_TEX_STAGE_0, GX_TRUE, GX_FALSE, GX_ITM_0);
	f32 indMtx[6];
	indMtx[0] = 0.6f;
	indMtx[1] = 0.0f;
	indMtx[2] = 0.0f;
	indMtx[3] = 0.0f;
	indMtx[4] = 0.6f;
	indMtx[5] = 0.0f;
	GXSetIndTexMtx(GX_ITM_0, indMtx, -2);

	GXBegin(GX_QUADS, GX_VTXFMT0, 4);
	Vec3f* corners = (Vec3f*)mCorners;
	f32 lo = 0.0f;
	f32 hi = 1.0f;
	GXPosition3f32(corners[0].x, corners[0].y, corners[0].z);
	GXTexCoord2f32(lo, lo);
	GXPosition3f32(corners[1].x, corners[1].y, corners[1].z);
	GXTexCoord2f32(hi, lo);
	GXPosition3f32(corners[2].x, corners[2].y, corners[2].z);
	GXTexCoord2f32(hi, hi);
	GXPosition3f32(corners[3].x, corners[3].y, corners[3].z);
	GXTexCoord2f32(lo, hi);

	GXSetNumIndStages(0);
	GXSetTevDirect(GX_TEVSTAGE0);
	njPopMatrixEx();
}

// 0x800766CC
int TObjMirror::getKind()
{
	return 3;
}

// 0x800766D4
int TObjMirror::getRotY()
{
	return mRot.y;
}

// 0x800766DC
Vec3f* TObjMirror::getPos()
{
	return &mPos;
}

// 0x800766E4
SetLink* TObjMirror::getLink()
{
	return mLink.get();
}
