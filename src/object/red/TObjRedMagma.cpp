#include "object/red/TObjRedMagma.h"
#include "engine/DrawTask.h"
#include "camera/TCamPath.h"
#include "egg/EggCreature.h"
#include "camera/TCameraCtrl.h"
#include "enemy/boss/TEneBoss02_Pool.h"
#include "object/red/TObjRedMagmaScene.h"
#include "Ninja/ninja.h"
#include "camera/TCamera.h"
#include "enemy/EnemyLight.h"
#include "engine/ScrollModel.h"
#include "object/ObjModel.h"
#include "io/ResBank.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "engine/GenTex.h"
#include "object/TObjLauncher.h"
#include "engine/DrawState.h"
#include "core/MathUtil.h"
#include "ui/AeArc.h"
#include "object/red/TObjRedMagmaObj.h"
struct GjActor;
struct NJS_OBJECT;
struct DrawStateSlot;

static const int sStageSceneModels[7] = { 0, 1, 2, 3, 4, 5, 6 };
static const int sStageObjModels[7]   = { 0, 1, 2, 3, 4, 5, 6 };
static const int sBossSceneModels[2]  = { 0, 1 };
static const int sBossObjModels[2]    = { 2, 3 };

const char* gTObjRedMagmaName = "TObjRedMagma";
const char* gTObjRedMagmaSceneName = "TObjRedMagmaScene";
const char* gTObjRedMagmaObjName = "TObjRedMagmaObj";

extern u32 gScrollWarpCoord; // the texture coordinate the scroll's warp reads

// --- ScrollModel ---

// 0x800649E4
// Steps a scroll and keeps it within one
static f32 scrollStep(f32 at, f32 speed)
{
	at += speed;
	if (speed >= 0.0f) {
		if (at >= 1.0f) {
			at -= 1.0f;
		}
		return at;
	}
	if (at <= -1.0f) {
		at += 1.0f;
	}
	return at;
}

// 0x80064A2C
ScrollModel::ScrollModel() { reset(); }

// 0x80064A68
ScrollModel::~ScrollModel() {}

// 0x80064AA4
void ScrollModel::reset()
{
	mScrollU  = 0.0f;
	mScrollV  = 0.0f;
	mSpeedU   = 0.01f;
	mSpeedV   = 0.01f;
	mTexIndex = -3;
	mAngle    = 0;
	mSin      = 0.0f;
	mScale    = 1.0f;
}

// 0x80064ADC
void ScrollModel::update()
{
	mAngle += 0x100;
	mSin     = njSin(mAngle);
	mScrollU = scrollStep(mScrollU, mSpeedU);
	mScrollV = scrollStep(mScrollV, mSpeedV);
}

// 0x80064B30
static void scrollPostTexGen(u8 coord)
{
	gScrollWarpCoord = coord;
	GXSetTexCoordGen2(coord, GX_TG_MTX2X4, GX_TG_TEX0, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
	GXSetIndTexOrder(GX_IND_TEX_STAGE_0, gScrollWarpCoord, GX_TEXMAP1);
}

// 0x80064B7C
// The model's own coordinates go through the scroll matrix
static void scrollTexCoordGen(int dst, int func, int src, int mtx)
{
	if ((u32)mtx == 0x2A) {
		GXSetTexCoordGen2(dst, func, src, mtx, GX_TRUE, GX_PTTEXMTX19);
	} else {
		GXSetTexCoordGen2(dst, func, src, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
	}
}

// 0x80064BB4
void ScrollModel::begin()
{
	void* funcs[8];
	f32 ind[2][3];
	f32 mtx[3][4];
	f32 warp;

	genTexLoad(2, 1);
	PSMTXIdentity(mtx[0]);
	mtx[0][0] = 1.0f;
	mtx[0][1] = 0.0f;
	mtx[0][2] = mScrollU;
	mtx[0][3] = 0.0f;
	mtx[1][0] = 0.0f;
	mtx[1][1] = 1.0f;
	mtx[1][2] = mScrollV;
	mtx[1][3] = 0.0f;
	GXLoadTexMtxImm(mtx[0], GX_TEXMTX0, GX_MTX2x4);
	mtx[0][0] = mScale;
	mtx[0][1] = 0.0f;
	mtx[0][2] = 0.0f;
	mtx[0][3] = 0.0f;
	mtx[1][0] = 0.0f;
	mtx[1][1] = mScale;
	mtx[1][2] = 0.0f;
	mtx[1][3] = 0.0f;
	GXLoadTexMtxImm(mtx[0], GX_TEXMTX1, GX_MTX2x4);
	warp      = 0.4f * mSin;
	ind[0][0] = 0.5f + warp;
	ind[0][1] = 0.0f;
	ind[0][2] = 0.0f;
	ind[1][0] = 0.0f;
	ind[1][1] = 0.5f - warp;
	ind[1][2] = 0.0f;
	GXSetIndTexMtx(GX_ITM_0, ind[0], mTexIndex);
	GXSetNumIndStages(1);
	GXSetIndTexCoordScale(GX_IND_TEX_STAGE_0, GX_ITS_1, GX_ITS_1);
	GXSetTevIndWarp(GX_TEVSTAGE0, GX_IND_TEX_STAGE_0, GX_TRUE, GX_FALSE, GX_ITM_0);
	funcs[4] = (void*)scrollTexCoordGen;
	gjChangeDrawObjIF(funcs, 0x40);
	_nj_control_3d_flag_ |= 0x80000;
	gjSetNumPostChans(0);
	gjSetNumPostTexGens(1);
	gjSetNumPostTevStages(0);
	gjSetPostChanCallback(0);
	gjSetPostTexGenCallback((void*)scrollPostTexGen);
	gjSetPostTevStageCallback(0);
}

// 0x80064D38
void ScrollModel::end()
{
	void* funcs[8];

	_nj_control_3d_flag_ &= ~0x80000;
	gjSetNumPostChans(0);
	gjSetNumPostTexGens(0);
	gjSetNumPostTevStages(0);
	gjSetPostChanCallback(0);
	gjSetPostTexGenCallback(0);
	gjSetPostTevStageCallback(0);
	funcs[4] = 0;
	gjChangeDrawObjIF(funcs, 0x40);
	GXSetTevDirect(GX_TEVSTAGE0);
	GXSetNumIndStages(0);
}

// 0x80064DB4
// Takes the model's bounding sphere
void ScrollModel::bindModel(void* node, SetTransform* trans)
{
	GjModel* model = (GjModel*)node;

	trans->mVel.x   = model->center.x;
	trans->mVel.y   = model->center.y;
	trans->mVel.z   = model->center.z;
	trans->mRadius  = model->r;
	trans->mForce.x = trans->mVel.x;
	trans->mForce.y = trans->mVel.y;
	trans->mForce.z = trans->mVel.z;
	gjScaleVector(-1.0f, &trans->mForce, &trans->mForce);
}

// 0x80064E24
extern "C" f32 scrollModelRadius(SetTransform* trans) { return trans->mRadius; }

// 0x80064E2C
void ScrollModel::draw(SetTransform* trans, void* model, void* texList)
{
	begin();
	njSetTexture(texList);
	njPushMatrixEx();
	njTranslateEx(&trans->mPos);
	gjRotateY(trans->mRot.y);
	njTranslateEx(&trans->mForce);
	gjDrawObject((NJS_OBJECT*)model);
	njPopMatrixEx();
	end();
}

// 0x80064E9C
void ScrollModel::readDesc(SetDesc* desc)
{
	mTexIndex = desc->getInt(1) - 2;
	mScale    = 0.1f + desc->getFloat(0);
	mSpeedU   = 0.01f * desc->getFloat(1);
	mSpeedV   = 0.01f * desc->getFloat(2);
}

// 0x80064F54
void ScrollModel::debugPrint()
{
	njPrint(0xA0014, ":IND EXP %4d", mTexIndex);
	njPrint(0xA0017, ":IND UV %3.3f", mScale);
	njPrint(0xA0018, ":U SPD %8.4f", mSpeedU);
	njPrint(0xA0019, ":V SPD %8.4f", mSpeedV);
}

// 0x80064FDC
void ScrollModel::debugPrintTrans(const SetTransform* trans)
{
	njPrintF(0xC0005, 5, trans->mVel.x);
	njPrintF(0xC0006, 5, trans->mVel.y);
	njPrintF(0xC0007, 5, trans->mVel.z);
}

// --- The red magma models ---

// 0x80065048
static void loadRedMagma(const char* arc, const char* textures, int sceneCount, const int* scene, int objCount,
                         const int* obj)
{
	resBankLoad(&gRedMagmaBank, arc);
	resBankAddTextures(&gRedMagmaBank, 0, textures);
	gRedMagmaModels = resBankGetModels(&gRedMagmaBank);
	gRedMagmaTexLists = resBankGetTextures(&gRedMagmaBank);
	setRedMagmaSceneModels(sceneCount, (int*)scene);
	setRedMagmaObjModels(objCount, (int*)obj);
}

// 0x800650E0
// For the red stage
extern "C" void loadRedMagmaStage()
{
	loadRedMagma("ar_obj_red_magma.arc", "obj_red_magma.gvm", 7, sStageSceneModels, 7, sStageObjModels);
	gRedMagmaSceneRange = 1000050.0f;
	gRedMagmaSceneFarAdd = 1000050.0f;
	gRedMagmaObjRange = 950.0f;
	gRedMagmaObjFarAdd = 950.0f;
}

// 0x80065138
extern "C" void releaseRedMagma()
{
	resBankRemoveTextures(&gRedMagmaBank, 0);
	resBankRelease(&gRedMagmaBank);
	gRedMagmaModels = 0;
	gRedMagmaTexLists = 0;
}

// 0x8006517C
// For the red boss's arena
extern "C" void loadRedMagmaBoss()
{
	loadRedMagma("ar_obj_red_boss_magma.arc", "obj_red_boss_magma.gvm", 2, sBossSceneModels, 2, sBossObjModels);
	gRedMagmaSceneRange = 2050.0f;
	gRedMagmaSceneFarAdd = 2050.0f;
	gRedMagmaObjRange = 1850.0f;
	gRedMagmaObjFarAdd = 1850.0f;
}

// 0x800651D4
extern "C" void releaseRedMagmaBoss() { releaseRedMagma(); }

// 0x800651F4
extern "C" NJS_TEXLIST* redMagmaTexList() { return *gRedMagmaTexLists; }

typedef char TObjRedMagma_size_check[(sizeof(TObjRedMagma) == 0xB8) ? 1 : -1];

// 0x80065200
TObjRedMagma::TObjRedMagma(TObject* parent, SetDesc* desc)
    : TSetObj(parent, desc)
{
	_00         = (u32)gTObjRedMagmaName;
	mSize       = 0xB8;
	mDraw       = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mModelIndex = 0;
	mSurface.reset();
	initFromDesc(desc);
}

// 0x8006529C
TObjRedMagma::~TObjRedMagma()
{
	removeDrawTask(mDraw);
	mDraw = 0;
}

// 0x80065318
void TObjRedMagma::bindModel(SetDesc* desc, int count, int* indices)
{
	int kind = desc->getInt(0);
	if (kind < 0) {
		kind = 0;
	} else if (kind > count - 1) {
		kind = count - 1;
	}
	mModelIndex = indices[kind];
	void* model = gRedMagmaModels[mModelIndex];
	mSurface.bindModel(*(void**)((u8*)model + 4), &mTrans);
}

// 0x800653AC
void TObjRedMagma::initFromDesc(SetDesc* desc)
{
	readDesc(desc);
	mSurface.readDesc(desc);
}

// 0x800653F4
void TObjRedMagma::draw()
{
	if (isInRange(this, &mTrans.mPos, getRange()) == 0) {
		mFlags |= 1;
	} else {
		mSurface.update();
	}
}

// 0x80065458
void TObjRedMagma::debugPrint(void*)
{
	njPrintC(0xA0013, ":MDL NO");
	mSurface.debugPrint();
	mSurface.debugPrintTrans(&mTrans);
}

// 0x800654A4
void TObjRedMagma::vfn_10()
{
	f32 radius = scrollModelRadius(&mTrans) + 10.0f;
	if (cameraIsVisible(radius + getFarAdd(), radius, &mTrans.mPos, 0) != 0) {
		mDraw->mFlags &= 0xFFFE;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x80065540
void TObjRedMagma::render()
{
	mSurface.draw(&mTrans, gRedMagmaModels[mModelIndex], *gRedMagmaTexLists);
}

typedef char TObjRedMagmaScene_size_check[(sizeof(TObjRedMagmaScene) == 0xB8) ? 1 : -1];

// 0x80065584
// Stores the scene's model count and index table
extern "C" void setRedMagmaSceneModels(int count, int* indices)
{
	sRedMagmaCount = count;
	sRedMagmaIndices = indices;
}

// 0x80065590
extern "C" TObject* createTObjRedMagmaScene(TObject* parent, SetDesc* desc)
{
	return new TObjRedMagmaScene(parent, desc);
}

// 0x800655E4
TObjRedMagmaScene::TObjRedMagmaScene(TObject* parent, SetDesc* desc)
    : TObjRedMagma(parent, desc)
{
	_00   = (u32)gTObjRedMagmaSceneName;
	mSize = 0xB8;
	initFromDesc(desc);
}

// 0x8006564C
void TObjRedMagmaScene::initFromDesc(SetDesc* desc)
{
	TObjRedMagma::initFromDesc(desc);
	bindModel(desc, sRedMagmaCount, sRedMagmaIndices);
}

// 0x80065690
f32 TObjRedMagmaScene::getRange()
{
	return gRedMagmaSceneRange;
}

// 0x80065698
f32 TObjRedMagmaScene::getFarAdd()
{
	return gRedMagmaSceneFarAdd;
}

typedef char TObjRedMagmaObj_size_check[(sizeof(TObjRedMagmaObj) == 0xB8) ? 1 : -1];

// 0x800656A0
// Stores the object's model count and index table
extern "C" void setRedMagmaObjModels(int count, int* indices)
{
	sRedMagmaSceneCount = count;
	sRedMagmaSceneIndices = indices;
}

// 0x800656AC
extern "C" TObject* createTObjRedMagmaObj(TObject* parent, SetDesc* desc)
{
	return new TObjRedMagmaObj(parent, desc);
}

// 0x80065700
TObjRedMagmaObj::TObjRedMagmaObj(TObject* parent, SetDesc* desc)
    : TObjRedMagma(parent, desc)
{
	_00   = (u32)gTObjRedMagmaObjName;
	mSize = 0xB8;
	initFromDesc(desc);
}

// 0x80065768
void TObjRedMagmaObj::initFromDesc(SetDesc* desc)
{
	TObjRedMagma::initFromDesc(desc);
	bindModel(desc, sRedMagmaSceneCount, sRedMagmaSceneIndices);
}

// 0x800657AC
f32 TObjRedMagmaObj::getRange()
{
	return gRedMagmaObjRange;
}

// 0x800657B4
f32 TObjRedMagmaObj::getFarAdd()
{
	return gRedMagmaObjFarAdd;
}
