#include "stage/SetManager.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "engine/ScrollModel.h"
#include "engine/FxHolder.h"
#include "object/ObjAction.h"
#include "enemy/boss/LastBossRes.h"
#include "engine/DrawState.h"
#include "object/last/TObjLastBossInd.h"
#include "Ninja/ninja.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCameraCtrl.h"
#include "stl/math.h"
#include "core/MathUtil.h"
#include "ui/AeArc.h"
#include "object/last/TObjLastBossEgg.h"
#include "core/TMainTask.h"
#include "animal/RacerMover.h"

extern "C" {
const char* gTObjLastBossEggName = "TObjLastBossEgg";
const char* gTObjLastBossEgg_FragName = "TObjLastBossEgg_Frag";
int gLastBossEggOpen;
int gLastBossEggYaw;
}

Vec3f gLastBossEggPos;

typedef char TObjLastBossEgg_size_check[(sizeof(TObjLastBossEgg) == 0xF4) ? 1 : -1];

// 0x8016CF34
extern "C" TObject* createTObjLastBossEgg(TObject* parent, SetDesc* desc)
{
	parent = gTL_TIMESTOP;
	return new TObjLastBossEgg(parent, desc);
}

// 0x8016CF88
TObjLastBossEgg::TObjLastBossEgg(TObject* parent, SetDesc* desc)
    : TSetObj(parent, desc)
{
	_00      = (u32)gTObjLastBossEggName;
	mSize    = 0xF4;
	mDraw    = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	mSurface.reset();
	mModel   = lastBossGetModel(3);
	mRadius  = lastBossModelRadius(mModel);
	mTrans.mRot.y = 0;
	initFromDesc(desc);
	fxHolderStop(&mGlow);
	mOpen = 0;
}

// 0x8016D054
TObjLastBossEgg::~TObjLastBossEgg()
{
	fxHolderStop(&mGlow);
	removeDrawTask(mDraw);
	mDraw = 0;
}

// 0x8016D0E4
void TObjLastBossEgg::initFromDesc(SetDesc* desc)
{
	int yaw = mTrans.mRot.y;
	readDesc(desc);
	mTrans.mRot.y = yaw;
	mTrans.mRot.x = desc->getInt(2) + 0x14;
	mSurface.readDesc(desc);
	mSurface.bindModel(mModel->model, &mTrans);

	f32 sx    = -mTrans.mPos.x;
	f32 sz    = -mTrans.mPos.z;
	f32* mtx;
	int angle = (int)(10430.38043493439 * (f32)atan2(sx, sz));
	njSinCos(angle, &sx, &sz);
	mtx = (f32*)&mGlow;
	Vec3f at;
	at = mTrans.mPos;
	at.x += -280.0f * sx;
	at.y += -30.0f;
	at.z += -280.0f * sz;
	mtxSetTransV(mtx, &at);
	mtxRotateY(mtx, mtx, angle);
	mtxRotateZ(mtx, mtx, 0x4000);
}

// 0x8016D224
void TObjLastBossEgg::draw()
{
	if (isInRange((void*)this, (const Vec3f*)&mTrans.mPos, 1000020.0f) == 0) {
		mFlags |= 1;
		return;
	}
	if (mOpen == 0) {
		if (gLastBossEggOpen != 0) {
			mOpen = 1;
		}
	} else if (gLastBossEggOpen == 0) {
		mOpen = 0;
	}
	if (mOpen == 0) {
		mSurface.update();
		mTrans.mRot.y += mTrans.mRot.x;
		mGlow.start(5, 0x20);
	} else {
		fxHolderStop(&mGlow);
	}
	gLastBossEggPos = mTrans.mPos;
	gLastBossEggYaw = mTrans.mRot.y;
}

// 0x8016D30C
void TObjLastBossEgg::debugPrint(void*)
{
	njPrint(0xA0015, ":ROT_SPD %6d", mTrans.mRot.x);
	mSurface.debugPrint();
	mSurface.debugPrintTrans(&mTrans);
}

// 0x8016D364
void TObjLastBossEgg::vfn_10()
{
	if (cameraIsVisible(1000000.0f, mRadius, &mTrans.mPos, 0) != 0) {
		mFlags &= ~0x10;
	} else {
		mFlags |= 0x10;
	}
}

// 0x8016D3C4
void TObjLastBossEgg::render()
{
	if (mOpen == 1) {
		return;
	}
	if (mModel == 0) {
		return;
	}
	NJS_TEXLIST* tex = lastBossGetTexList(0);
	((void (*)())njSetTexture)();
	njPushMatrixEx();
	njTranslateEx(&mTrans.mPos);
	gjRotateY(mTrans.mRot.y);
	njTranslateEx(&mTrans.mForce);
	drawModelDefault(mModel->model);
	njPopMatrixEx();
	void* node = mModel->child;
	if (node != 0) {
		mSurface.draw(&mTrans, node, tex);
	}
}

#include "stage/Stage.h"
#include "engine/DrawTask.h"
#include "stage/SetManager.h"
#include "object/blue/TObjBlueCaptain.h"
#include "object/ObjAction.h"
#include "engine/DrawState.h"
#include "object/last/TObjLastBossInd.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "core/PlacementNew.h"
#include "camera/TCameraCtrl.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "ui/AeArc.h"
#include "object/last/TObjLastBossEgg_Frag.h"
struct DrawTask;

typedef char TObjLastBossEgg_Frag_size_check[(sizeof(TObjLastBossEgg_Frag) == 0x80) ? 1 : -1];

// 0x8016D460
extern "C" void spawnTObjLastBossEgg_Frag(void* model, const Vec3f* pos, const Vec3f* dir, int angle)
{
	new TObjLastBossEgg_Frag(gTL_TIMESTOP, 0, model, pos, *dir, angle);
}

// 0x8016D4DC
extern "C" void spawnTObjLastBossEgg_FragShell(void* model, const Vec3f* pos, const Vec3f* dir, int angle)
{
	new TObjLastBossEgg_Frag(gTL_TIMESTOP, 1, model, pos, *dir, angle);
}

// 0x8016D558
TObjLastBossEgg_Frag::TObjLastBossEgg_Frag(TObject* parent, int kind, void* model, const Vec3f* pos, Vec3f dir, int angle)
    : TObject(parent)
{
	_00            = (u32)gTObjLastBossEgg_FragName;
	mSize          = 0x80;
	mXform.mPos    = *pos;
	mXform.mVel    = dir;
	if (njUnitVector(&mXform.mVel) < gUnitVectorMin) {
		mXform.mVel = gVecZero;
	}
	gjScaleVector(8.0f, &mXform.mVel, &mXform.mVel);
	mXform.mRot.y = angle;
	mXform.mForce = *pos;
	mSpinX  = (int)(1200.0f * (3.0517578125e-05f * (f32)rand()) + -600.0f);
	mSpinY  = (int)(400.0f * (3.0517578125e-05f * (f32)rand()) + -200.0f);
	mModel  = (NJS_OBJECT*)model;
	mRadius = lastBossModelRadius(mModel);
	mKind   = kind;
	mLife   = 300;
	mDraw   = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
}

// 0x8016D70C
TObjLastBossEgg_Frag::~TObjLastBossEgg_Frag()
{
	removeDrawTask(mDraw);
	mDraw = 0;
}

// 0x8016D7A4
void TObjLastBossEgg_Frag::draw()
{
	if (gLastBossEggOpen == 0) {
		mFlags |= 1;
		return;
	}
	if (mKind == 1) {
		return;
	}
	njAddVector(&mXform.mPos, &mXform.mVel);
	mXform.mVel.y += -0.2f;
	mXform.mRot.x += mSpinX;
	mXform.mRot.y += mSpinY;
	mLife -= 1;
	if (mLife <= 0) {
		mFlags |= 1;
	}
}

// 0x8016D854
void TObjLastBossEgg_Frag::vfn_10()
{
	if (cameraIsVisible(1000000.0f, mRadius, &mXform.mPos, 0) != 0) {
		mFlags &= ~0x10;
	} else {
		mFlags |= 0x10;
	}
}

// 0x8016D8B4
void TObjLastBossEgg_Frag::render()
{
	lastBossGetTexList(0);
	((void (*)())njSetTexture)();
	njPushMatrixEx();
	njTranslateEx(&mXform.mPos);
	gjRotateY(mXform.mRot.y);
	gjRotateX(mXform.mRot.x);
	drawModelDefault(mModel->model);
	njPopMatrixEx();
}
