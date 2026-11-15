#include "animal/AniAttacks.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "stage/StageObjUtil.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "engine/CharaColli.h"
#include "engine/GenTex.h"
#include "object/TObjLauncher.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"
#include "camera/TCameraCtrl.h"
#include "engine/DrawState.h"
#include "ui/AeArc.h"
#include "core/MathUtil.h"
#include "engine/FrameTex.h"
#include "engine/TParticle.h"
#include "animal/TAniBubbleWave.h"
#include "animal/AniModel.h"
struct DrawStateSlot;
struct DrawState;
struct CharaColli;
struct CamView;

extern "C" {
extern const MotionEntry sBubbleWaveMotions[9] = {
	{ MOTION_LOOP, 0, -1, 0, 0.0f, 30.0f, 1.0f, 0.0f, 0 }, { MOTION_LOOP, 1, -1, 0, 0.0f, 10.0f, 1.0f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 10.0f, 1.0f, 0.0f, 0 }, { MOTION_LOOP, 3, -1, 0, 0.0f, 30.0f, 1.0f, 0.0f, 0 },
	{ MOTION_LOOP, 4, -1, 0, 0.0f, 30.0f, 1.0f, 0.0f, 0 }, { -1, 0, 0, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
	{ MOTION_LOOP, 0, -1, 0, 0.0f, 30.0f, 1.0f, 0.0f, 0 }, { MOTION_LOOP, 1, -1, 0, 0.0f, 10.0f, 1.0f, 0.0f, 0 },
	{ MOTION_CHAIN, 0, 0, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
};
extern const MotionEntry sBubbleWaveMotionB[1] = {
	{ MOTION_ONCE, 6, -1, 0, 0.0f, 20.0f, 0.4f, 0.0f, 0 },
};
extern const MotionEntry sBubbleWaveMotionA[1] = {
	{ MOTION_ONCE, 7, -1, 0, 0.0f, 20.0f, 0.4f, 0.0f, 0 },
};
extern const ColliShape sBubbleWaveShape[1] = {
	{ { 0.0f, 5.0f, 0.0f }, 0x0, 0, 0x0, 25.0f, 10.0f, 0.0f, 0x0, COLLISHAPE_CYLINDER, 0x0 },
};
}

#define WGPIPE_F32 (*(volatile f32*)0xCC008000)

struct WaveCamera {
	u8 _00[0x44];
	int mFov; // _44
	u8 _48[4];
	f32 mAspect; // _4C
};

const char* gTAniBubbleWaveName = "TAniBubbleWave";

// 0x801340C4
TAniBubbleWave::TAniBubbleWave(TEggCreatureTypeE* owner, const Vec3f* pos, u16 ownerId)
    : TObject(owner)
    , mPos(*pos)
{
	_00       = (u32)gTAniBubbleWaveName;
	mSize     = 0x13C;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	mObjectA  = owner->mMotion->mObject[3];
	mObjectB  = owner->mMotion->mObject[2];
	mTexList  = (NJS_TEXLIST*)*owner->mMotion->mTexList;
	mPlayerB.init(sBubbleWaveMotionB, owner->mMotion->mMotions);
	mPlayerB.play(0);
	mPlayerA.init(sBubbleWaveMotionA, owner->mMotion->mMotions);
	mPlayerA.play(0);
	mDrawState          = *getDefaultDrawState();
	mDrawState.mFlags   = 0x200;
	mDrawState.mAmbient = 0xFF;
	mScale.x            = 1.5f;
	mScale.y            = 1.0f;
	mScale.z            = 1.5f;
	mtxSetScaleV(mMatrix, &mScale);
	mRadius    = sBubbleWaveShape->mSizeX;
	mHeight    = sBubbleWaveShape->mSizeY;
	mFrameWait = 0;
	mRise      = 0;
	mTicks     = 0;
	mFrame     = 0.0f;
	mIsDisc    = 0;
	mWarp      = 1.0f;
	mDiscAngle = 0x8000;
	mColli     = colliCreate(1, 1);
	if (mColli != 0) {
		colliSetOwner(mColli, (u16)ownerId);
		colliSetMasks((CharaColli*)mColli, 0x20, 0xFF, 0xFF, 0x10);
		colliCopyShapes((CharaColli*)mColli, sBubbleWaveShape);
		colliPlaceShapes((CharaColli*)mColli, &mPos, sBubbleWaveShape);
		colliShapeSetBit8((CharaColli*)mColli, 0);
	}
}

// 0x80134308
TAniBubbleWave::~TAniBubbleWave()
{
	colliFree((CharaColli*)mColli);
	removeDrawTask(mDrawTask);
}

// 0x80134378
void TAniBubbleWave::draw()
{
	if (mRise < 0x8000) {
		mRise += 0x280;
		mHeight = 30.0f * njSin(mRise);
		mRadius += 1.17647f;
		f32 height                     = 10.0f;
		height                         = (mHeight < height) ? height : mHeight;
		f32 radius                     = mRadius;
		colliGetShapes(mColli)->mSizeX = radius;
		colliGetShapes(mColli)->mSizeY = height;
	}

	if (mIsDisc != 0) {
		mDrawTask->mFlags &= ~1;
		if (mDiscAngle < 0xC000) {
			mDiscAngle += 0x100;
		} else {
			mFlags |= 1;
		}
	} else {
		f32 wait = 1.0f + sBubbleWaveMotionA[0].mEnd;
		wait /= sBubbleWaveMotionA[0].mRate;
		if ((f32)mFrameWait++ > wait / 16.0f - 1.0f) {
			if (mFrame < 3.75f) {
				mFrame += 0.25f;
			}
			mFrameWait = 0;
		}
		if (mTicks++ == 7) {
			particleOnceByIdDefault(&mPos, 0, 0x9A);
		}
		if (mPlayerA.mFrame > 10.0f && fadeAmbient(&mDrawState, 0, 8) != 0) {
			mIsDisc = 1;
			moveDrawTask(mDrawTask, 0x2A);
			mDrawTask->mFlags |= 1;
		}
	}
	updateMatrix();
	mPlayerB.update();
	mPlayerA.update();
}

// 0x80134580
void TAniBubbleWave::vfn_10()
{
	if (mIsDisc != 0) {
		moveDrawTask(mDrawTask, 0x2A);
	}
	Vec3f view;
	if (cameraIsVisible(1000.0f, 50.0f, &mPos, &view) != 0) {
		if (mIsDisc == 0) {
			mDrawTask->mSortKey = drawTaskDepthKey(view.z);
		}
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x8013461C
void TAniBubbleWave::render()
{
	if (mIsDisc != 0) {
		renderDisc();
	} else {
		renderWave();
	}
}

// A texture coordinate of the disc: the unit circle point scaled into 0..1
static inline f32 discU(f32 x, f32 uv)
{
	x /= uv;
	x = 1.0f + x;
	x = 0.5f * x;
	return x;
}

// 0x80134650
void TAniBubbleWave::renderDisc()
{
	gjLoadPosMatrix(0);
	gjLoadNrmMatrix(0);
	GXSetCurrentMtx(GX_PNMTX0);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GXColor white = { 0xFF, 0xFF, 0xFF, 0xFF };
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
	GXSetChanMatColor(GX_COLOR0A0, white);

	f32 light[12];
	WaveCamera* camera = (WaveCamera*)camGetDrawView();
	C_MTXLightPerspective((float*)light, (f32)(0.75 * (0.0054931640625 * (f64)camera->mFov)), camera->mAspect, 0.5f, -0.5f, 0.5f, 0.5f);
	PSMTXConcat(light, (const f32*)_gj_matrix_, light);
	GXLoadTexMtxImm(light, GX_TEXMTX1, GX_MTX3x4);
	genTexLoadWrap(4, 1, 0);
	frameTexSetCur();
	gjLoadTextureNum(0, 0, 0, 0);
	GXSetNumTexGens(2);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3X4, GX_TG_POS, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
	GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2X4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
	GXSetNumTevStages(1);
	GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
	GXSetNumIndStages(1);
	GXSetIndTexOrder(GX_IND_TEX_STAGE_0, GX_TEXCOORD1, GX_TEXMAP1);
	GXSetIndTexCoordScale(GX_IND_TEX_STAGE_0, GX_ITS_1, GX_ITS_1);
	GXSetTevIndWarp(GX_TEVSTAGE0, GX_IND_TEX_STAGE_0, GX_TRUE, GX_FALSE, GX_ITM_0);

	mWarp = 0.8f;
	f32 warp[6];
	warp[0] = 0.0f;
	warp[1] = 0.0f;
	warp[2] = 0.0f;
	warp[3] = 0.0f;
	warp[4] = mWarp;
	warp[5] = 0.0f;
	GXSetIndTexMtx(GX_ITM_0, warp, -2);
	GXSetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_COPY);
	GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0xFF);

	f32 half;
	f32 y;
	f32 midZ;
	f32 midX;
	f32 uv;
	midX = mPos.x;
	y    = 0.5f + mPos.y;
	midZ = mPos.z;
	uv   = 1.1f + njCos(mDiscAngle);
	uv *= 3.0f;
	half = 0.5f;
	GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 0xE);
	GXPosition3f32(midX, y, midZ);
	GXTexCoord2f32(half, half);
	for (s16 i = 0; i < 0xD; i++) {
		int angle = (int)(182.04445f * (f32)(i * 30));
		f32 c     = njCos(angle);
		f32 s     = njSin(angle);
		GXPosition3f32(mRadius * c + mPos.x, y, mRadius * s + mPos.z);
		GXTexCoord2f32(discU(c, uv), discU(-s, uv));
	}
	GXSetNumIndStages(0);
	GXSetTevDirect(GX_TEVSTAGE0);
}

// 0x80134A50
void TAniBubbleWave::renderWave()
{
	DrawStateSlot* slot = getDrawStateSlot();
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	beginDrawState(slot, (const DrawState*)&mDrawState);
	beginScroll();
	if (mPlayerA.isBlending() != 0) {
		gjDrawMotionLink((NJS_OBJECT*)mObjectA, (void*)mPlayerA.getBlend(), mPlayerA.mBlendTime);
	} else {
		int cur            = mPlayerA.mCurrent;
		f32 frame          = mPlayerA.mFrame;
		NJS_MOTION* motion = mPlayerA.getMotion(cur);
		gjDrawMotion((NJS_OBJECT*)mObjectA, motion, frame);
	}
	endScroll();
	if (mPlayerB.isBlending() != 0) {
		gjDrawMotionLink((NJS_OBJECT*)mObjectB, (void*)mPlayerB.getBlend(), mPlayerB.mBlendTime);
	} else {
		int cur            = mPlayerB.mCurrent;
		f32 frame          = mPlayerB.mFrame;
		NJS_MOTION* motion = mPlayerB.getMotion(cur);
		gjDrawMotion((NJS_OBJECT*)mObjectB, motion, frame);
	}
	endDrawState((DrawStateSlot*)slot);
	njPopMatrixEx();
}

// 0x80134C54
static void scrollTexGen(int id, int func, int src)
{
	GXSetTexCoordGen2(id, func, src, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
}

// 0x80134B60
void TAniBubbleWave::beginScroll()
{
	njSetTexture(mTexList);
	gjLoadTextureNum(0, 1, 1, 0);
	f32 frame = mFrame;
	f32 m[12];
	PSMTXTrans(m, frame - (f32)(s16)(int)frame, 0.25f * (f32)(s16)(int)frame, 0.0f);
	GXLoadTexMtxImm(m, GX_TEXMTX0, GX_MTX2x4);
	u8 funcs[0x20];
	*(void**)(funcs + 0x10) = (void*)scrollTexGen;
	gjChangeDrawObjIF((void* const*)funcs, (u32)0x40);
}

// 0x80134C24
void TAniBubbleWave::endScroll()
{
	u8 funcs[0x20];
	*(void**)(funcs + 0x10) = 0;
	gjChangeDrawObjIF((void* const*)funcs, (u32)0x40);
}

// 0x80134C80
void TAniBubbleWave::updateMatrix()
{
	mtxSetTransV(mMatrix, &mPos);
	mtxScaleV(mMatrix, mMatrix, &mScale);
}
