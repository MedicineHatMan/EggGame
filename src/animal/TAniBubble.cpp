#include "animal/AniAttacks.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "engine/GenTex.h"
#include "enemy/boss/EneBoss.h"
#include "Ninja/ninja.h"
#include "ui/AeArc.h"
#include "engine/FrameTex.h"
#include "camera/TCameraCtrl.h"
#include "animal/TAniBubble.h"
#include "animal/AniModel.h"

static const GXColor sBubbleTevColor = { 0, 0, 0, 0xC0 };
struct NJS_OBJECT;
struct CamView;

// The field of view and aspect of the current view
struct AniCamera {
	u8 _00[0x44];
	int mFov; // _44
	u8 _48[4];
	f32 mAspect; // _4C
};

const char* gTAniBubbleName = "TAniBubble";

// 0x80133B1C
TAniBubble::TAniBubble(TEggCreatureTypeE* owner)
    : TObject(owner)
{
	_00          = (u32)gTAniBubbleName;
	mSize        = 0x50;
	mDrawTask    = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x2A, 0);
	mPosRef      = 0;
	mRotRef      = 0;
	mSizeRef     = 0;
	mWobble.x    = 1.0f;
	mWobble.y    = 1.0f;
	mWobble.z    = 1.0f;
	mWobbleAngle = 0;
	mWarp        = 0.5f;
	mWarpAngle   = 0;
	mObject      = (NJS_OBJECT*)owner->mMotion->mObject[1];
	mTexList     = (NJS_TEXLIST*)*owner->mMotion->mTexList;
}

// 0x80133BD8
TAniBubble::~TAniBubble()
{
	removeDrawTask(mDrawTask);
}

// 0x80133C40
void TAniBubble::draw()
{
	mWobble.x = 0.2f * njSin(mWobbleAngle) + 1.0f;
	mWobble.y = 0.2f * njCos(mWobbleAngle) + 1.0f;
	mWobble.z = 0.2f * njSin(mWobbleAngle) + 1.0f;
	mWobbleAngle += 0x800;
	mWobbleAngle = (u16)mWobbleAngle;
}

// 0x80133CC8
void TAniBubble::vfn_10()
{
}

static void texGenCallback(u8 id);
static void tevCallback(u8 stage);

// 0x80133CCC
void TAniBubble::render()
{
	f32 size = *mSizeRef / 5.0f;
	gjLoadPosMatrix(0);
	gjLoadNrmMatrix(0);
	GXSetCurrentMtx(GX_PNMTX0);
	njPushMatrixEx();
	njTranslateEx(mPosRef);
	if (mRotRef != 0) {
		gjRotateY(mRotRef[1]);
		gjRotateX(mRotRef[0]);
	}
	njScale(0, size, size, size);
	njScale(0, mWobble.x, mWobble.y, mWobble.z);

	f32 light[12];
	f32 identity[12];
	AniCamera* camera = (AniCamera*)camGetDrawView();
	C_MTXLightPerspective((float*)light, (f32)(0.75 * (0.0054931640625 * (f64)camera->mFov)), camera->mAspect, -0.5f, -0.5f, 0.5f, 0.5f);
	PSMTXConcat(light, (const f32*)_gj_matrix_, light);
	GXLoadTexMtxImm(light, GX_TEXMTX1, GX_MTX3x4);
	PSMTXIdentity(identity);
	GXLoadTexMtxImm(identity, GX_TEXMTX2, GX_MTX3x4);
	frameTexSetCur();
	gjLoadTextureNum(0, 0, 0, 1);
	genTexLoad(1, 2);
	njSetTexture(mTexList);
	_nj_control_3d_flag_ |= 0x80000;
	gjSetNumPostTexGens(2);
	gjSetPostTexGenCallback((void*)texGenCallback);
	gjSetNumPostTevStages(1);
	gjSetPostTevStageCallback((void*)tevCallback);
	GXSetNumIndStages(1);
	GXSetIndTexOrder(GX_IND_TEX_STAGE_0, GX_TEXCOORD2, GX_TEXMAP2);
	GXSetIndTexCoordScale(GX_IND_TEX_STAGE_0, GX_ITS_1, GX_ITS_1);
	GXSetTevIndWarp(GX_TEVSTAGE1, GX_IND_TEX_STAGE_0, GX_TRUE, GX_FALSE, GX_ITM_0);

	f32 sin = mWarp * njSin(mWarpAngle);
	f32 cos = mWarp * njCos(mWarpAngle);
	mWarpAngle += 0x400;
	mWarpAngle = (u16)mWarpAngle;
	f32 warp[6];
	warp[0] = sin;
	warp[1] = cos;
	warp[2] = 0.0f;
	warp[3] = cos;
	warp[4] = sin;
	warp[5] = 0.0f;
	GXSetIndTexMtx(GX_ITM_0, warp, -2);
	GXSetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_COPY);
	GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0xFF);
	gjDrawObject(mObject);

	GXSetNumIndStages(0);
	gjSetNumPostTevStages(0);
	GXSetTevDirect(GX_TEVSTAGE1);
	GXSetTevDirect(GX_TEVSTAGE0);
	gjSetNumPostTexGens(0);
	_nj_control_3d_flag_ &= ~0x80000;
	njPopMatrixEx();
	blendModeAlpha();
}

// 0x80133F84
static void texGenCallback(u8 id)
{
	GXSetTexCoordGen2(id, GX_TG_MTX3X4, GX_TG_POS, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
	GXSetTexCoordGen2(id + 1, GX_TG_MTX2X4, GX_TG_NRM, GX_TEXMTX2, GX_FALSE, GX_PTIDENTITY);
}

// 0x80133FE4
static void tevCallback(u8 stage)
{
	GXSetTevColor(GX_TEVREG0, sBubbleTevColor);
	u8 coord = gjGetPostTexGenStartID();
	GXSetTevColorIn(stage, GX_CC_CPREV, GX_CC_TEXC, GX_CC_A0, GX_CC_ZERO);
	GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_DIVIDE_2, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(stage, GX_CA_APREV, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_DIVIDE_2, GX_TRUE, GX_TEVPREV);
	GXSetTevOrder(stage, coord, GX_TEXMAP1, GX_COLOR_NULL);
}

// 0x801340A8
int TAniBubble::vfn_20()
{
	return 4;
}

// 0x801340B0
const Vec3f* TAniBubble::vfn_24()
{
	return mPosRef;
}

// 0x801340B8
int TAniBubble::vfn_28()
{
	return mRotRef[1];
}
