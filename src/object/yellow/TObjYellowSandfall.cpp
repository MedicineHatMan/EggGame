#include "stage/SetManager.h"
struct DrawTask;
#include "stage/SetActor.h"
#include "sound/SoundHandle.h"
#include "object/ObjModel.h"
#include "engine/DrawTask.h"
#include "Ninja/ninja.h"
#include "object/ObjArchive.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "camera/TCamera.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCameraCtrl.h"
#include "core/MathUtil.h"
#include "engine/Color.h"
#include "engine/TParticle.h"
#include "stage/TEventCtrl.h"
#include "stage/SetSlot.h"
#include "object/yellow/TObjYellowSandfall.h"
#include "object/yellow/SandfallListImpl.h"
#include "stage/TStageSelectModelE3.h"
struct SetLink;

#define sBrick gTimberBrickObj

class TObjYellowSandfall;

static SandfallList sSandfalls;

typedef char TObjYellowSandfall_size_check[(sizeof(TObjYellowSandfall) == 0x150) ? 1 : -1];

const f32 gYellowSandfallSetRange = 1000.0f;

// 0x8016B0A0
extern "C" TObject* createTObjYellowSandfall(TObject* parent, SetDesc* desc)
{
	return new TObjYellowSandfall(parent, desc);
}

// 0x8016B0F4
extern "C" void pauseEventSandfalls()
{
	for (SandfallNode* node = sSandfalls.mHead; node != 0; node = node->mNext) {
		if (node->mObj->mType != 0) {
			eventHideObj((u8*)node->mObj);
		}
	}
}

// 0x8016B148
void TObjYellowSandfall::start()
{
	if (mType != 0) {
		showEffect();
		updateEffect();
		sePlay3DHeld(800.0f, &mSound, 0x25E, 100, &mEffectPos);
		mAct = 1;
	}
}

// 0x8016B1A8
void TObjYellowSandfall::stop()
{
	if (mType != 0) {
		mState.mFlags = DRAWSTATE_AMBIENT;
		seStop3D(&mSound);
		mAct = 2;
	}
}

// 0x8016B1F4
extern "C" void startEventSandfalls()
{
	for (SandfallNode* node = sSandfalls.mHead; node != 0; node = node->mNext) {
		node->mObj->start();
	}
}

// 0x8016B23C
extern "C" void stopEventSandfalls()
{
	for (SandfallNode* node = sSandfalls.mHead; node != 0; node = node->mNext) {
		node->mObj->stop();
	}
}

const char* gTObjYellowSandfallName = "TObjYellowSandfall";

// 0x8016B284
TObjYellowSandfall::TObjYellowSandfall(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00        = (u32)gTObjYellowSandfallName;
	mSize      = 0x150;
	mNode.mObj = this;
	sSandfalls.add(&mNode);

	mModelData = getGeObjModels()[4];
	mTexList   = getGeObjTextures()[4];
	mDrawTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
	mEffect    = (SandfallFx*)particleGenById(this, &mPos, 1, 0);
	if (mEffect != 0) {
		mEffect->mFlags |= 4;
	}
	initFromDesc(desc);
	vfn_74();
}

// 0x8016B3A8
TObjYellowSandfall::~TObjYellowSandfall()
{
	seStop3D(&mSound);
	if (mEffect != 0) {
		mEffect->mFlags |= 1;
		mEffect = 0;
	}
	removeDrawTask(mDrawTask);
	sSandfalls.remove(&mNode);
}

// 0x8016B454
void TObjYellowSandfall::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	int type;
	if (desc->getInt(0) < 0) {
		type = 0;
	} else if (desc->getInt(0) > 1) {
		type = 1;
	} else {
		type = desc->getInt(0);
	}
	mType = type;
	if (mType == 0) {
		showEffect();
	}
}

// 0x8016B504
void TObjYellowSandfall::debugPrint(void*)
{
	njPrint(0xA0013, ":TYPE=%s", (mType != 0) ? "EVENT" : "NORMAL");
	njPrintC(0xA0014, "");
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, "");
	njPrintC(0xA0017, "");
	njPrintC(0xA0018, "");
	njPrintC(0xA0019, "");
	njPrintC(0xA001A, "");
}

// 0x8016B5BC
void TObjYellowSandfall::vfn_74()
{
	mState.mAmbient = 0xFF;
	mScrollV        = 0.0f;
	mScrollU        = 0.0f;
	updateMatrix();
	updateEffect();
	mModel.setModel(mModelData);
	mModel.mTexList = mTexList;
	mModel.mState   = &mState;
	setLinkFree((SetLink*)&mLink);
	mAct = 0;
}

// 0x8016B630
void TObjYellowSandfall::updateMatrix()
{
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
	GjModel* model = mModelData->model;
	mCenter        = *(Vec3f*)&model->center;
	mtxMultVec(mMatrix, &mCenter, &mCenter);
	mRadius = mModelData->model->r;
}

// 0x8016B6B4
void TObjYellowSandfall::showEffect()
{
	if (mEffect != 0) {
		mEffect->mFlags &= ~4;
	}
}

// 0x8016B6D0
void TObjYellowSandfall::hideEffect()
{
	if (mEffect != 0) {
		mEffect->mFlags |= 4;
	}
}

// 0x8016B6EC
void TObjYellowSandfall::updateEffect()
{
	if (mEffect == 0) {
		return;
	}
	if (mType != 0) {
		if (sBrick != 0) {
			mEffectPos.set(mPos.x, *(f32*)(sBrick + 0x40) + *(f32*)(sBrick + 0xCC), mPos.z);
		} else {
			mEffectPos = mPos;
		}
	} else {
		mEffectPos = mPos;
	}
	mEffect->mPos = mEffectPos;
}

// 0x8016B788
void TObjYellowSandfall::scroll()
{
	mScrollU += 0.005f;
	if (mScrollU >= 1.0f) {
		mScrollU -= 1.0f;
	}
	mScrollV -= 0.02f;
	if (mScrollV < -1.0f) {
		mScrollV += 1.0f;
	}
}

// 0x8016B7E4
void TObjYellowSandfall::draw()
{
	if (objDebugEnabled(this) != 0) {
		updateMatrix();
		updateEffect();
		scroll();
		return;
	}
	if (!isInRange(this, &mPos, 1100.0f)) {
		mFlags |= 1;
		return;
	}
	switch (mAct) {
	case 0:
		stateWait();
		break;
	case 1:
		stateRun();
		break;
	case 2:
		stateStop();
		break;
	}
}

// 0x8016B8A4
void TObjYellowSandfall::stateWait()
{
	if (mType == 0) {
		scroll();
	}
}

// 0x8016B8D0
void TObjYellowSandfall::stateRun()
{
	scroll();
	updateEffect();
	mSound.mPos = mEffectPos;
}

// 0x8016B91C
void TObjYellowSandfall::stateStop()
{
	stateRun();
	int alpha = (u8)mState.mAmbient - 4;
	if ((s16)alpha < 0) {
		hideEffect();
		mState.mFlags = 0;
		setColorAlpha(&mState.mAmbient, 0xFF);
		mAct = 0;
	} else {
		setColorAlpha(&mState.mAmbient, (u8)(s16)alpha);
	}
}

// 0x8016B994
void TObjYellowSandfall::vfn_10()
{
	if (mType != 0 && mAct == 0) {
		mDrawTask->mFlags |= 1;
		return;
	}
	Vec3f screen;
	if (cameraIsVisible(1000.0f, mRadius, &mCenter, &screen) != 0) {
		mDrawTask->mSortKey = drawTaskDepthKey(screen.z);
		mDrawTask->mFlags &= ~1;
		if ((u8)mState.mAmbient != 0xFF) {
			moveDrawTask(mDrawTask, 34);
		} else {
			moveDrawTask(mDrawTask, 33);
		}
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x8016BA64
// Sends the scrolled texture coordinates through texture matrix 0x1E
static void sandfallTexGen(int id, int func, int src)
{
	GXSetTexCoordGen2(id, func, src, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
}

// 0x8016BA90
void TObjYellowSandfall::render()
{
	void* table[8];
	table[4] = (void*)sandfallTexGen;
	gjChangeDrawObjIF(table, 0x40);

	f32 texMtx[12];
	mtxSetTrans((float*)texMtx, mScrollU, mScrollV, 0.0f);
	GXLoadTexMtxImm(texMtx, GX_TEXMTX0, GX_MTX2x4);

	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	mModel.draw();
	njPopMatrixEx();

	table[4] = 0;
	gjChangeDrawObjIF(table, 0x40);
}

// 0x8016BB24
void SandfallList::add(SandfallNode* node)
{
	if (mHead == 0) {
		mTail = node;
		mHead = node;
	} else {
		node->mPrev  = mTail;
		mTail->mNext = node;
		mTail        = node;
	}
	node->mList = this;
	mCount++;
}

// 0x8016BB64
void SandfallList::remove(SandfallNode* node)
{
	if (node->mPrev != 0) {
		node->mPrev->mNext = node->mNext;
	} else {
		mHead = node->mNext;
	}
	if (node->mNext != 0) {
		node->mNext->mPrev = node->mPrev;
	} else {
		mTail = node->mPrev;
	}
	node->mNext = 0;
	node->mPrev = 0;
	node->mList = 0;
	mCount--;
}
