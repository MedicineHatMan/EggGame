#include "core/types.h"
struct DrawTask;
#include "engine/WaterSurface.h"
#include "object/ObjScript.h"
#include "object/ObjAction.h"
#include "engine/DrawTask.h"
#include "object/ObjArchive.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "engine/GenTex.h"
#include "camera/TCamera.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCameraCtrl.h"
#include "egg/EggCreature.h"
#include "ui/AeArc.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "engine/FrameTex.h"
#include "core/VecMath.h"
#include "object/green/TObjGreenPondScene.h"
struct NJS_OBJECT;
struct CamView;

/** The parts of a model the water reads */
struct WaterMesh {
	u8 _00[0x14];
	Vec3f mCentre; // _14
	f32 mRadius;   // _20
};

struct WaterObject {
	u8 _00[4];
	WaterMesh* mMesh; // _04
};

// Colours for the TEV callback which has no way to read them from the water
f32 gGreenPondSceneSetRange = 3000.0f; // 0x802C45C0

const f32 sWaterFarDist = gGreenPondSceneSetRange;         // 0x802C5920
f32 sWaterKeepDist      = gGreenPondSceneSetRange + 50.0f; // 0x802C5924
static int sBlendMode;                                     // 0x802C5928
static u8 sColor[4];                                       // 0x802C592C
static u8 sAddColor[4];                                    // 0x802C5930

static const int sPondModels[3]   = { 3, 4, 5 }; // 0x8024B23C
static const int sPondTextures[3] = { 3, 4, 5 }; // 0x8024B248

// 0x80184A34
WaterSurface::WaterSurface()
{
	mModel     = 0;
	mTexList   = 0;
	mCentre.x  = 0.0f;
	mCentre.y  = 0.0f;
	mCentre.z  = 0.0f;
	mRadius    = 0.0f;
	mScroll[0] = 0.0f;
	mScroll[1] = 0.0f;
	mScroll[2] = 0.0f;
	mScroll[3] = 0.0f;
	mSpeedU    = 0.0f;
	mSpeedV    = 0.0f;
	mIndExp    = 0;
	mIndRate   = 0.0f;
	mFarDist   = sWaterFarDist;
	mtxIdentity(mMatrix);
	mColor[0]    = 0xBE;
	mColor[1]    = 0xDC;
	mColor[2]    = 0xFF;
	mColor[3]    = 0;
	mAddColor[0] = 0x20;
	mAddColor[1] = 0;
	mAddColor[2] = 0;
	mAddColor[3] = 0;
}

// 0x80184AF0
WaterSurface::~WaterSurface()
{
}

// 0x80184B38
void WaterSurface::setModel(void* model, void* texList)
{
	mModel   = (NJS_OBJECT*)model;
	mTexList = (NJS_TEXLIST*)texList;
	mRadius  = ((WaterObject*)model)->mMesh->mRadius;
}

// 0x80184B50
// The record's rotation is the colour and byte 0x3E is the blend mode
void WaterSurface::initFromDesc(SetDesc* desc)
{
	SetRot color;

	desc->getPos(&mPos);
	desc->getRot(&color);

	mBlendMode = 1 - CLAMP_RANGE(((SetDescObj*)desc)->get3E(), 0, 1);
	mColor[0]  = color.x;
	mColor[1]  = color.y;
	mColor[2]  = color.z;
	mColor[3]  = desc->getInt(1);

	mFakeBump = CLAMP_RANGE(desc->getInt(3), 0, 1);
	mIndExp   = desc->getInt(2) - 2;
	mIndRate  = 0.01f * desc->getFloat(2);
	mIndScale = 1.0f + desc->getFloat(3);
	mSpeedU   = 0.01f * desc->getFloat(0);
	mSpeedV   = 0.01f * desc->getFloat(1);
	mCentre   = mPos;

	if (mModel != 0 && ((WaterObject*)mModel)->mMesh != 0) {
		Vec3f at;
		Vec3f offset;
		const Vec3f& c = ((WaterObject*)mModel)->mMesh->mCentre;
		offset.set(c.x, c.y, c.z);
		vecSub(&at, &mPos, &offset);
		mtxSetTransV(mMatrix, &at);
	} else {
		mtxSetTransV(mMatrix, &mPos);
	}
}

// 0x80184DB0
// The lines are the set-file labels
void WaterSurface::debugPrint()
{
	if (mModel != 0 && ((WaterObject*)mModel)->mMesh != 0) {
		njPrint(0xA0005, ":%.4f", ((WaterObject*)mModel)->mMesh->mCentre.x);
		njPrint(0xA0006, ":%.4f", ((WaterObject*)mModel)->mMesh->mCentre.y);
		njPrint(0xA0007, ":%.4f", ((WaterObject*)mModel)->mMesh->mCentre.z);
	}
	njPrintC(0xA0014, ":ALPHA");
	njPrint(0xA0015, "+%d:IND EXP", -2);
	njPrintC(0xA0016, ":FAKE BUNP");
	njPrintC(0xA0017, "*0.01:U SPD");
	njPrintC(0xA0018, "*0.01:V SPD");
	njPrintC(0xA0019, "*0.01:IND RATE");
	njPrint(0xA001A, "+1.0:IND SCALE");
}

// 0x80184ECC
// Adds a step and wraps the result to stay under 1 in the direction of the step
static f32 wrapScroll(f32 value, f32 step)
{
	value += step;
	if (step >= 0.0f) {
		if (value >= 1.0f) {
			return value - 1.0f;
		}
	} else if (value <= -1.0f) {
		value += 1.0f;
		return value;
	}
	return value;
}

// 0x80184F14
// The second layer moves at the speed of the first times the indirect scale
void WaterSurface::update()
{
	mScroll[0] = wrapScroll(mScroll[0], mSpeedU);
	mScroll[1] = wrapScroll(mScroll[1], mSpeedV);
	mScroll[2] = wrapScroll(mScroll[2], mSpeedU * mIndScale);
	mScroll[3] = wrapScroll(mScroll[3], mSpeedV * mIndScale);
}

// 0x80184F8C
int WaterSurface::isVisible()
{
	return cameraIsVisible(mFarDist, mRadius, &mCentre, 0);
}

// 0x80184FC0
// A projection from the camera so that the first layer lands on the screen
static void getScreenTexMatrix(f32* out)
{
	f32 proj[12];
	CamView* view = (CamView*)camGetView(camGetDrawIndex());

	f32 fov = 0.75f * (360.0f * ((f32)view->mFov / 65536.0f));
	C_MTXLightPerspective((float*)proj, fov, view->mAspect, 0.5f, -0.5f, 0.5f, 0.5f);
	PSMTXConcat(proj, _gj_matrix_, out);
}

// 0x80185058
void WaterSurface::beginDraw()
{
	u32 info[8];

	_nj_control_3d_flag_ |= 0x80000;
	info[4] = (u32)waterTexGen0;
	gjChangeDrawObjIF((void* const*)info, (u32)0x40);
	gjSetNumPostChans(0);
	gjSetPostChanCallback(0);
	gjSetNumPostTevStages((mFakeBump != 0 ? 1 : 0) + 1);
	gjSetPostTevStageCallback((void*)waterTevStage);
	gjSetNumPostTexGens(2);
	gjSetPostTexGenCallback((void*)waterTexGen1);
}

// 0x801850E8
void WaterSurface::endDraw()
{
	u32 info[8];

	GXSetNumIndStages(0);
	GXSetTevDirect(GX_TEVSTAGE0);
	GXSetTevDirect(GX_TEVSTAGE1);
	info[4] = 0;
	gjChangeDrawObjIF((void* const*)info, (u32)0x40);
	gjSetNumPostChans(0);
	gjSetNumPostTexGens(0);
	gjSetNumPostTevStages(0);
	_nj_control_3d_flag_ &= ~0x80000;
}

// 0x80185154
void WaterSurface::render()
{
	f32 m[12];
	f32 ind[6];

	if (mModel == 0 || mTexList == 0) {
		return;
	}
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);

	mtxSetTrans((float*)m, mScroll[0], mScroll[1], 0.0f);
	GXLoadTexMtxImm(m, GX_TEXMTX0, GX_MTX2x4);
	getScreenTexMatrix(m);
	GXLoadTexMtxImm(m, GX_TEXMTX1, GX_MTX3x4);

	if (0.0f == mIndRate) {
		mtxSetTrans((float*)m, mScroll[2], mScroll[3], 0.0f);
		mtxScale(mIndScale, mIndScale, 1.0f, m, m);
	} else {
		mtxSetTrans((float*)m, mScroll[2], mScroll[3], 0.0f);
		mtxScale(mIndScale * (1.0f + mIndRate), mIndScale * (1.0f - mIndRate), 1.0f, m, m);
	}
	GXLoadTexMtxImm(m, GX_TEXMTX2, GX_MTX2x4);

	frameTexSetCur();
	gjLoadTextureNum(0, 0, 0, 1);
	genTexLoad(2, 2);
	if (mFakeBump != 0) {
		genTexLoad(2, 3);
	}
	njSetTexture(mTexList);
	GXSetNumIndStages(1);
	GXSetIndTexOrder(GX_IND_TEX_STAGE_0, GX_TEXCOORD2, GX_TEXMAP2);
	GXSetIndTexCoordScale(GX_IND_TEX_STAGE_0, GX_ITS_1, GX_ITS_1);
	GXSetTevIndWarp(GX_TEVSTAGE1, GX_IND_TEX_STAGE_0, GX_TRUE, GX_FALSE, GX_ITM_0);

	ind[0] = 0.5f;
	ind[1] = 0.0f;
	ind[2] = 0.0f;
	ind[3] = 0.0f;
	ind[4] = 0.5f;
	ind[5] = 0.0f;
	GXSetIndTexMtx(GX_ITM_0, ind, (s8)mIndExp);

	beginDraw();
	sColor[0]    = mColor[0];
	sColor[1]    = mColor[1];
	sColor[2]    = mColor[2];
	sColor[3]    = mColor[3];
	sAddColor[0] = mAddColor[0];
	sAddColor[1] = mAddColor[1];
	sAddColor[2] = mAddColor[2];
	sAddColor[3] = mAddColor[3];
	sBlendMode   = mBlendMode;
	pushCullMode(0);
	gjDrawObject(mModel);
	popCullMode();
	endDraw();
	njPopMatrixEx();
}

typedef char TObjGreenPondScene_size_check[(sizeof(TObjGreenPondScene) == 0x114) ? 1 : -1];

// 0x801853B0
extern "C" void waterTexGen0(int coord, int type, int src)
{
	GXSetTexCoordGen2(coord, type, src, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
}

// 0x801853DC
extern "C" void waterTexGen1()
{
	GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX3X4, GX_TG_POS, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
	GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2X4, GX_TG_TEX0, GX_TEXMTX2, GX_FALSE, GX_PTIDENTITY);
}

// 0x80185430
extern "C" void waterTevStage(int stageIn)
{
	u8 stage = stageIn;
	int next = stage + 1;

	GXSetTevKColorSel(next, GX_TEV_KCSEL_K1_R);
	GXSetTevKColor(GX_KCOLOR0, *(GXColor*)sColor);
	GXSetTevKColor(GX_KCOLOR1, *(GXColor*)sAddColor);
	GXSetTevOrder(stage, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
	if (sBlendMode != 0) {
		GXSetTevKColorSel(stage, GX_TEV_KCSEL_K0_A);
		GXSetTevColorIn(stage, GX_CC_CPREV, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
	} else {
		GXSetTevKColorSel(stage, GX_TEV_KCSEL_K0);
		GXSetTevColorIn(stage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
	}
	GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevOrder(next, GX_TEXCOORD2, GX_TEXMAP3, GX_COLOR_NULL);
	GXSetTevColorIn(next, GX_CC_KONST, GX_CC_ZERO, GX_CC_TEXA, GX_CC_CPREV);
	GXSetTevColorOp(next, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(next, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevAlphaOp(next, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}

// 0x801855C8
extern "C" TObject* createTObjGreenPondScene(TObject* parent, SetDesc* desc)
{
	return new TObjGreenPondScene(parent, desc);
}

const char* gTObjGreenPondSceneName = "TObjGreenPondScene";

// 0x8018561C
TObjGreenPondScene::TObjGreenPondScene(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00       = (u32)gTObjGreenPondSceneName;
	mSize     = 0x114;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x2A, 0);
	initFromDesc(desc);
	vfn_74();
}

// 0x801856BC
TObjGreenPondScene::~TObjGreenPondScene()
{
	removeDrawTask(mDrawTask);
}

// 0x80185730
void TObjGreenPondScene::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);

	mIndex   = CLAMP_RANGE(desc->getInt(0), 0, 3);
	mModel   = getGeObjModels()[sPondModels[mIndex]];
	mTexList = getGeObjTextures()[sPondTextures[mIndex]];
	mWater.setModel(mModel, mTexList);
	mWater.initFromDesc(desc);
}

// 0x80185830
void TObjGreenPondScene::debugPrint(void*)
{
	njPrintC(0xA0013, ":MODEL IDX");
	mWater.debugPrint();
}

// 0x80185874
void TObjGreenPondScene::vfn_74()
{
}

// 0x80185878
void TObjGreenPondScene::draw()
{
	mWater.update();
}

// 0x8018589C
void TObjGreenPondScene::vfn_10()
{
	if (mWater.isVisible() != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x801858F8
void TObjGreenPondScene::render()
{
	mWater.render();
}
