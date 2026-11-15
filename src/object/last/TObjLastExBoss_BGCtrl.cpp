#include "stage/Stage.h"
struct DrawTask;
#include "engine/FxHolder.h"
#include "object/last/TObjLastExBoss_Gareki.h"
#include "animal/AniMount.h"
#include "core/MathUtil.h"
#include "object/last/TObjLastExBoss_BGCtrl.h"
#include "engine/DrawTask.h"
#include "object/ObjModel.h"
#include "Ninja/ninja.h"
#include "stage/SetManager.h"
#include "object/last/TObjLastBossInd.h"
#include "object/ObjAction.h"
#include "object/TObjLauncher.h"
#include "engine/DrawState.h"
#include "egg/EggCreature.h"
#include "camera/TCameraCtrl.h"
#include "ui/AeArc.h"

extern "C" {
extern TObject* gTL_TIMESTOP; // TMainTask.cpp's
void* sLastExBg;
}

typedef char TObjLastExBoss_BGCtrl_size_check[(sizeof(TObjLastExBoss_BGCtrl) == 0x60) ? 1 : -1];

// 0x8017B18C
extern "C" void clearLastExBg()
{
	sLastExBg = 0;
}

// 0x8017B198
extern "C" void releaseLastExBg()
{
	TObjLastExBoss_BGCtrl* bg = (TObjLastExBoss_BGCtrl*)sLastExBg;
	if (bg == 0) {
		return;
	}
	bg->mFlags |= 1;
	sLastExBg = 0;
}

// 0x8017B1BC
extern "C" void createTObjLastExBoss_BGCtrl()
{
	if (sLastExBg != 0) {
		return;
	}
	void* mem = objHeapAlloc(gObjHeap, 0x60);
	mem       = new (mem) TObjLastExBoss_BGCtrl(gTL_TIMESTOP);
	sLastExBg = mem;
}

// 0x8017B208
extern "C" void armLastExBg()
{
	TObjLastExBoss_BGCtrl* bg = (TObjLastExBoss_BGCtrl*)sLastExBg;
	if (bg == 0) {
		return;
	}
	bg->mMode = 1;
}

// The rubble's five model ids lastBossGetModel's 3 to 7
const int gGarekiModels[5] = { 3, 4, 5, 6, 7 };

// Gareki's name comes first the DOL has its string and pointer ahead of BGCtrl's
const char* gTObjLastExBoss_GarekiName = "TObjLastExBoss_Gareki";
const char* gTObjLastExBoss_BGCtrlName = "TObjLastExBoss_BGCtrl";

// 0x8017B220
TObjLastExBoss_BGCtrl::TObjLastExBoss_BGCtrl(TObject* parent)
    : TObject(parent)
{
	_00   = (u32)gTObjLastExBoss_BGCtrlName;
	mSize = 0x60;
	mMode = 0;
	_24   = 0;
	mWait = 0;
	Vec3f pos;
	pos.x = 0.0f;
	pos.y = 20.0f;
	pos.z = 0.0f;
	mGlow.mMtx.setPosRot(&pos, 0xFFFFC000, 0);
}

// 0x8017B2AC
TObjLastExBoss_BGCtrl::~TObjLastExBoss_BGCtrl()
{
	fxHolderStop(&mGlow);
	if (sLastExBg == this) {
		sLastExBg = 0;
	}
}

// 0x8017B334
void TObjLastExBoss_BGCtrl::draw()
{
	if (mMode == 1) {
		fxHolderStop(&mGlow);
		return;
	}
	mGlow.start(8, 2);
	mWait--;
	if (mWait > 0) {
		return;
	}
	int wait = (int)(24.0f * (3.0517578125e-05f * (f32)(int)rand()));
	if (wait >= 24) {
		wait = 23;
	}
	mWait    = wait + 6;
	f32 dist = 800.0f * (3.0517578125e-05f * (f32)(int)rand()) + 450.0f;
	Vec3f pos;
	vecFromAngleXZ(dist, &pos, (int)(65536.0f * (3.0517578125e-05f * (f32)(int)rand())));
	pos.y    = -600.0f;
	int size = (int)(5.0f * (3.0517578125e-05f * (f32)(int)rand()));
	if (size >= 5) {
		size = 4;
	}
	new TObjLastExBoss_Gareki(this, &pos, gGarekiModels[size]);
}

struct DrawStateSlot;

extern "C" {
}

typedef char TObjLastExBoss_Gareki_size_check[(sizeof(TObjLastExBoss_Gareki) == 0x88) ? 1 : -1];

// 0x8017B4F8
TObjLastExBoss_GarekiBase::TObjLastExBoss_GarekiBase(TObject* parent, const Vec3f* pos, int modelId)
    : TObject(parent)
{
	mXform.mPos.x = pos->x;
	mXform.mPos.y = pos->y;
	mXform.mPos.z = pos->z;
	mSpinX        = 0;
	mSpinY        = 0;
	_6C           = 0;
	mModel        = lastBossGetModel(modelId);
	if (mModel != 0 && mModel->model != 0) {
		mRadius = (int)(200.0f + mModel->model->r);
	} else {
		mRadius = 200;
	}
	mAlpha = 0;
	mDraw  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
}

// 0x8017B5DC
TObjLastExBoss_GarekiBase::~TObjLastExBoss_GarekiBase()
{
	removeDrawTask(mDraw);
	mDraw = 0;
}

// Unused so the linker strips it
int garekiIsVisible(TObjLastExBoss_GarekiBase* rock)
{
	return cameraIsVisible(10000.0f, rock->mRadius, &rock->mXform.mPos, 0);
}

// 0x8017B674
TObjLastExBoss_Gareki::TObjLastExBoss_Gareki(TObject* parent, const Vec3f* pos, int modelId)
    : TObjLastExBoss_GarekiBase(parent, pos, modelId)
{
	_00    = (u32)gTObjLastExBoss_GarekiName;
	mSize  = 0x88;
	mVelY  = randSpan(4.0f, 3.0f);
	mTop   = randSpan(200.0f, 0.0f);
	mSpinX = (int)randSpan(0.0f, 500.0f);
	mSpinY = (int)randSpan(0.0f, 200.0f);
}

// 0x8017B7B8
TObjLastExBoss_Gareki::~TObjLastExBoss_Gareki()
{
}

// 0x8017B814
void TObjLastExBoss_Gareki::draw()
{
	mXform.mPos.y += mVelY;
	mXform.mRot.x += mSpinX;
	mXform.mRot.y += mSpinY;
	f32 y = mXform.mPos.y;
	if (y > mTop) {
		mVelY += 0.3f;
	}
	if (y < 600.0f) {
		mAlpha += 5.0f;
		if (mAlpha > 0xFF) {
			mAlpha = 0xFF;
		}
	} else {
		mAlpha = mAlpha - 5.0f;
		if (mAlpha <= 0) {
			mAlpha = 0;
			mFlags |= 1;
		}
	}
}

// 0x8017B924
void TObjLastExBoss_Gareki::vfn_10()
{
	if (cameraIsVisible(10000.0f, mRadius, &mXform.mPos, 0)) {
		mFlags &= ~0x10;
	} else {
		mFlags |= 0x10;
	}
	if (mAlpha < 0xFF) {
		moveDrawTask(mDraw, 0x22);
	} else {
		moveDrawTask(mDraw, 0x21);
	}
}

// 0x8017B9C8
void TObjLastExBoss_Gareki::render()
{
	DrawStateSlot* slot = getDrawStateSlot();
	DrawState* def      = getDefaultDrawState();
	ObjDrawState buf;
	DrawState* use;

	if (slot == 0 || def == 0) {
		return;
	}
	njSetTexture(lastBossGetTexList(0));
	if (mAlpha < 0xFF) {
		use  = &buf;
		*use = *def;
		use->mFlags |= 0x200;
		use->mAmbient = (u8)mAlpha;
	} else {
		use = def;
	}
	njPushMatrixEx();
	njTranslateEx(&mXform.mPos);
	gjRotateY(mXform.mRot.y);
	gjRotateX(mXform.mRot.x);
	njTranslateEx(&mXform.mForce);
	beginDrawState(slot, use);
	gjDrawObject(mModel);
	endDrawState(slot);
	njPopMatrixEx();
}
