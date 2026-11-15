#include "object/blue/TObjBlueWaterfall.h"
#include "engine/DrawTask.h"
#include "object/ObjScript.h"
#include "object/ObjArchive.h"
#include "engine/CharaColli.h"
#include "engine/GenTex.h"
#include "engine/DrawState.h"
#include "object/ObjAction.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCameraCtrl.h"
#include "ui/AeArc.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"
#include "engine/FrameTex.h"
#include "egg/EggCreature.h"
#include "stage/SetSlot.h"
struct ColliShape;
struct SetLink;
struct NJS_OBJECT;
struct CamView;

extern "C" {

// 0x802C4000
ObjClassName gTObjBlueWaterfallName             = { "TObjBlueWaterfall", 0 };
extern const ColliShape sBlueWaterfallShapes[1] = {
	{ { 10.0f, -160.0f, 0.0f }, 0x0, 0, 0x0, 10.0f, 160.0f, 0.0f, 0x0, COLLISHAPE_CYLINDER, 0x0 },
};
}

// The view camGetView returns mAspect is the second argument of C_MTXLightPerspective
struct WaterView {
	u8 _00[0x44];
	int mFov; // _44
	u8 _48[4];
	f32 mAspect; // _4C
};

typedef char TObjBlueWaterfall_size_check[(sizeof(TObjBlueWaterfall) == 0x88) ? 1 : -1];

static void chanCallback();
static void texGenCallback();
static void tevCallback(int stage);
static void emptyDrawCallback();

// 0x800BC6EC
extern "C" TObject* createTObjBlueWaterfall(TObject* parent, SetDesc* desc)
{
	return new TObjBlueWaterfall(parent, desc);
}

// 0x800BC740
void TObjBlueWaterfall::setup()
{
	f32 zero = 0.0f;

	mStopped                   = 0;
	mMatId                     = 2;
	mScroll                    = zero;
	mIndScroll                 = zero;
	((u8*)&sWaterfallColor)[0] = 0;
	((u8*)&sWaterfallColor)[1] = 0;
	((u8*)&sWaterfallColor)[2] = 0;
	mWater                     = getGeObjModels()[0x16];
	mPlain                     = getGeObjModels()[0x15];
	mTex                       = getGeObjTextures()[0x13];
	setLinkDone((SetLink*)&mLink);
	mShape = NULL;
	mShape = colliCreate(1, 1);
	colliCopyShapes(mShape, (const ColliShape*)sBlueWaterfallShapes);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 0xFF, 0, 0xF6);
	colliShapeSetBit8(mShape, 0);
	colliPlaceShapesRotY(mShape, &mPos, mRot.y, sBlueWaterfallShapes);
}

// 0x800BC830
TObjBlueWaterfall::TObjBlueWaterfall(TObject* parent, SetDesc* desc)
    : TObjAction(parent)
{
	setObjId(this, (s16)0xFFFF);
	_00   = (u32)gTObjBlueWaterfallName.mName;
	mSize = 0x88;
	mLink.get()->attach(desc);
	initFromDesc(desc);
	mWaterTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x2A, 0);
	mPlainTask = addDrawTask(this, (void (*)(void*))drawMistTask, 0x21, 0);
	setup();
}

// 0x800BC8FC
TObjBlueWaterfall::~TObjBlueWaterfall()
{
	colliRetire(mShape);
	mShape = 0;
	removeDrawTask(mWaterTask);
	removeDrawTask(mPlainTask);
	notifySetLinkDeath(mLink.get(), this);
	releaseObjId(this);
}

// 0x800BC99C
void TObjBlueWaterfall::initFromDesc(SetDesc* desc)
{
	desc->getPos(&mPos);
	desc->getRot(&mRot);

	((u8*)&sWaterfallColor)[3] = desc->getInt(0);
	((u8*)&sWaterfallColor)[3] = ((u8*)&sWaterfallColor)[3] > 0xFF ? 0xFF : ((u8*)&sWaterfallColor)[3];

	mIndOffset   = 0.6f + desc->getFloat(1);
	mScrollSpeed = 0.03f + desc->getFloat(2);
	mScrollSpeed *= 0.1f;
	mIndSpeed = 0.12f + desc->getFloat(3);
	mIndSpeed *= 0.1f;
}

// 0x800BCABC
void TObjBlueWaterfall::debugPrint(void*)
{
	njPrintC(0xA0013, ":ALPHA(0~255)");
	njPrintC(0xA0018, "+0.06:IND_OFFSET");
	njPrintC(0xA0019, "*0.1+0.003:IND_SCROLL SPEED(0.0~1.0)");
	njPrintC(0xA001A, "*0.1+0.012:SCROLL SPEED(0.0~1.0)");
}

// 0x800BCB28
int TObjBlueWaterfall::onHit(const TObjAction*, int, int* result)
{
	HitInfo* hit = (HitInfo*)result;
	resetHitInfo(hit);
	hit->mDamage = 0x19;
	hit->mFlags.setBit(7);
	return 1;
}

// 0x800BCB70
// Both scrolls wrap at 0 by adding 1
void TObjBlueWaterfall::scroll()
{
	mIndScroll -= mIndSpeed;
	if (mIndScroll < 0.0f) {
		mIndScroll += 1.0f;
	}
	mScroll -= mScrollSpeed;
	if (mScroll < 0.0f) {
		mScroll += 1.0f;
	}
}

// 0x800BCBCC
void TObjBlueWaterfall::draw()
{
	if (isInRange(this, &mPos, 1100.0f) == 0) {
		if (mStopped == 0) {
			setLinkFree((SetLink*)&mLink);
		}
		mFlags |= 1;
	} else {
		switch (mStopped) {
		case 0:
			scroll();
			break;
		}
	}
}

// 0x800BCC44
// Bit 0 of a draw task skips it
void TObjBlueWaterfall::vfn_10()
{
	if (cameraIsVisible(1000.0f, 200.0f, getPos(), 0) != 0) {
		mWaterTask->mFlags &= ~1;
		mPlainTask->mFlags &= ~1;
	} else {
		mWaterTask->mFlags |= 1;
		mPlainTask->mFlags |= 1;
	}
}

// 0x800BCCD8
void TObjBlueWaterfall::render()
{
	drawWater();
}

// 0x800BCCF8
void TObjBlueWaterfall::drawMistTask(TObjBlueWaterfall* self)
{
	self->drawMist();
}

// 0x800BCD18
void TObjBlueWaterfall::drawMist()
{
	njPushMatrixEx();
	njTranslateEx(&mPos);
	gjRotateY(mRot.y);
	njSetTexture(mTex);
	drawObjectDefault(mPlain);
	njPopMatrixEx();
}

// 0x800BCD68
void TObjBlueWaterfall::drawWater()
{
	gjLoadPosMatrix(0);
	gjLoadNrmMatrix(0);
	GXSetCurrentMtx(GX_PNMTX0);
	njPushMatrixEx();
	njTranslateEx(&mPos);
	gjRotateY(mRot.y);

	f32 scrollMtx[12];
	PSMTXTrans(scrollMtx, 0.0f, mIndScroll, 0.0f);
	GXLoadTexMtxImm(scrollMtx, GX_TEXMTX0, GX_MTX2x4);

	WaterView* view = (WaterView*)camGetView(0);
	f32 fov         = 0.75f * (360.0f * ((f32)view->mFov / 65536.0f));
	f32 light[12];
	C_MTXLightPerspective((float*)light, fov, view->mAspect, 0.5f, -0.5f, 0.5f, 0.5f);
	f32 proj[12];
	PSMTXConcat(light, _gj_matrix_, proj);
	GXLoadTexMtxImm(proj, GX_TEXMTX1, GX_MTX3x4);

	f32 flowMtx[12];
	PSMTXTrans(flowMtx, 0.0f, mScroll, 0.0f);
	GXLoadTexMtxImm(flowMtx, GX_TEXMTX2, GX_MTX2x4);

	frameTexSetCur();
	gjLoadTextureNum(0, 0, 0, 1);
	njSetTexture(mTex);
	gjLoadTextureNum(0, 1, 1, 0);
	genTexLoad(mMatId, 2);

	GXSetNumIndStages(1);
	GXSetIndTexOrder(GX_IND_TEX_STAGE_0, GX_TEXCOORD3, GX_TEXMAP2);
	GXSetIndTexCoordScale(GX_IND_TEX_STAGE_0, GX_ITS_1, GX_ITS_1);
	GXSetTevIndWarp(sWaterfallTevStage, GX_IND_TEX_STAGE_0, GX_TRUE, GX_FALSE, GX_ITM_0);

	f32 warp[6];
	warp[0] = mIndOffset;
	warp[1] = 0.0f;
	warp[2] = 0.0f;
	warp[3] = 0.0f;
	warp[4] = mIndOffset;
	warp[5] = 0.0f;
	GXSetIndTexMtx(GX_ITM_0, warp, -2);

	void* drawIf[8];
	_nj_control_3d_flag_ |= 0x80000;
	drawIf[7] = (void*)emptyDrawCallback;
	gjChangeDrawObjIF(drawIf, 0x10);
	gjSetNumPostChans(1);
	gjSetPostChanCallback((void (*)(s32))chanCallback);
	gjSetNumPostTevStages(2);
	gjSetPostTevStageCallback((void*)tevCallback);
	gjSetNumPostTexGens(3);
	gjSetPostTexGenCallback((void*)texGenCallback);
	gjDrawObject(mWater);

	GXSetNumIndStages(0);
	GXSetTevDirect(sWaterfallTevStage);
	drawIf[7] = 0;
	gjChangeDrawObjIF(drawIf, 0x10);
	gjSetNumPostChans(0);
	gjSetNumPostTexGens(0);
	gjSetNumPostTevStages(0);
	_nj_control_3d_flag_ &= 0xFFF7FFFF;
	njPopMatrixEx();
}

// 0x800BCFE8
// Post channel callback The argument is unused
static void chanCallback()
{
	GXColor white = *(GXColor*)&sBlueWaterfallWhite;
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
	GXSetChanMatColor(GX_COLOR0A0, white);
}

// 0x800BD048
// Post tex-gen callback The arguments are unused
static void texGenCallback()
{
	GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2X4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
	GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX3X4, GX_TG_POS, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
	GXSetTexCoordGen2(GX_TEXCOORD3, GX_TG_MTX2X4, GX_TG_TEX0, GX_TEXMTX2, GX_FALSE, GX_PTIDENTITY);
}

// 0x800BD0B8
// Post tev-stage callback Stores the next stage in sWaterfallTevStage which drawWater reads
static void tevCallback(int stage)
{
	GXSetTevColor(GX_TEVREG0, *(GXColor*)&sWaterfallColor);
	stage &= 0xFF;
	int next           = stage + 1;
	sWaterfallTevStage = next;
	GXSetTevOp(stage, GX_REPLACE);
	GXSetTevColorIn(next, GX_CC_CPREV, GX_CC_TEXC, GX_CC_A0, GX_CC_ZERO);
	GXSetTevColorOp(next, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(next, GX_CA_APREV, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
	GXSetTevAlphaOp(next, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevOrder(stage, GX_TEXCOORD1, GX_TEXMAP0, GX_COLOR0A0);
	GXSetTevOrder(next, GX_TEXCOORD2, GX_TEXMAP1, GX_COLOR0A0);
}

// 0x800BD19C
// Draw-object callback at slot +0x1C
static void emptyDrawCallback()
{
}

// 0x800BD1A0
Vec3f* TObjBlueWaterfall::getPos()
{
	return &mPos;
}

// 0x800BD1A8
int TObjBlueWaterfall::getKind()
{
	return 3;
}

// 0x800BD1B0
int TObjBlueWaterfall::getRotY()
{
	return mRot.y;
}

// 0x800BD1B8
SetLink* TObjBlueWaterfall::getLink()
{
	return mLink.get();
}

// 0x800BD1C0
// Nothing in this class calls it
void initBlueWaterfallSet()
{
}

// 0x800BD1C4
// Nothing in this class calls it
void termBlueWaterfallSet()
{
}
