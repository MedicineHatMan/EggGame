#include "enemy/boss/TEneLastExBossCore.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "object/ObjModel.h"
#include "engine/DrawState.h"
#include "engine/MotionPose.h"
#include "object/last/TObjLastBossShot.h"
#include "enemy/boss/TEneLastExBoss.h"
#include "object/TObjLauncher.h"
#include "object/ObjShadow.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "camera/TCameraCtrl.h"
#include "ui/AeArc.h"
#include "enemy/EnemyFx.h"
#include "core/TMainTask.h"

extern "C" {
MotionEntry sLastExCoreMotionTable[] = {
	{ MOTION_LOOP, 0, -1, 0, 0.0f, 30.0f, 0.6f, 0.0f, 0 },
	{ MOTION_ONCE, 1, -1, 0, 0.0f, 719.0f, 1.0f, 0.0f, 0 },
};
}

typedef char TEneLastExBossCore_size_check[(sizeof(TEneLastExBossCore) == 0x100) ? 1 : -1];

// 0x8018D148
TEneLastExBossCore::TEneLastExBossCore(TObject* parent)
    : TObject(parent)
{
	_00      = sLastExCoreName[0];
	mSize    = 0x100;
	mModel   = modelTEneLastExBoss(7);
	mMotions = motionTEneLastExBoss(2);
	mTexList = textureTEneLastExBoss();
	motionCtrlSetPlayer(&mClip, &mMotion);
	motionCtrlInit(&mClip, sLastExCoreMotionTable, mMotions);
	motionCtrlPlay(&mClip, 0);
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	fxHolderStop(&mGlow);
	mTint.reset();
	mState = CORE_HIDDEN;
	mAlpha = 0;
}

// 0x8018D244
TEneLastExBossCore::~TEneLastExBossCore()
{
	fxHolderStop(&mGlow);
	removeDrawTask(mDraw);
}

// 0x8018D2E8
// Fades in from clear
void TEneLastExBossCore::appear()
{
	mState = CORE_APPEAR;
	mAlpha = 0;
	fxHolderStop(&mGlow);
}

// 0x8018D31C
void TEneLastExBossCore::vanish()
{
	hide();
}

// 0x8018D33C
void TEneLastExBossCore::show()
{
	mState = CORE_SHOWN;
	mAlpha = 255;
}

// 0x8018D350
void TEneLastExBossCore::hide()
{
	mState = CORE_HIDDEN;
	mAlpha = 0;
}

// 0x8018D364
void TEneLastExBossCore::burst()
{
	mState = CORE_BURST;
	mAlpha = 255;
	motionCtrlPlay(&mClip, 1);
}

// 0x8018D3A0
void TEneLastExBossCore::setPos(const Vec3f* pos)
{
	mXform.mPos.set(pos->x, pos->y, pos->z);
}

// 0x8018D3BC
void TEneLastExBossCore::flash()
{
	mTint.start();
}

// 0x8018D3E0
void TEneLastExBossCore::draw()
{
	mTint.update();
	switch (mState) {
	case CORE_APPEAR:
		motionCtrlUpdate(&mClip);
		mAlpha += 4;
		if (mAlpha >= 255) {
			mAlpha = 255;
			mState = CORE_SHOWN;
		}
		fxHolderStop(&mGlow);
		break;
	case CORE_SHOWN:
		motionCtrlUpdate(&mClip);
		mGlow.start(5, 0x14, &mXform.mPos);
		break;
	case CORE_STILL:
		fxHolderStop(&mGlow);
		break;
	case CORE_HIDDEN:
		fxHolderStop(&mGlow);
		break;
	case CORE_BURST: {
		MotionNodePose pose;
		Vec3f pos;
		mXform.mPos.x = 0.0f;
		mXform.mPos.y = 0.0f;
		mXform.mPos.z = 0.0f;
		motionCtrlUpdate(&mClip);
		f32 frame = motionCtrlFrame(&mClip);
		gjPushUnitMatrix();
		motionGetNodePose(&pose, mModel, motionCtrlGetMotion(&mClip), frame);
		motionPoseGetPos(&pose, &pos);
		njPopMatrixEx();
		if (frame < 340.0f) {
			mGlow.start(5, 0x14, &pos);
		} else {
			fxHolderStop(&mGlow);
		}
		if (motionCtrlPassed(370.0f, &mClip) || motionCtrlPassed(372.0f, &mClip)) {
			fxSpawnTurned(5, (void*)0x23, (const Vec3f*)&pos, -0x4000, 0);
		}
		if (frame >= 390.0f) {
			mState = CORE_HIDDEN;
			mAlpha = 0;
			motionCtrlPlay(&mClip, 0);
			mXform.mPos.x = pos.x;
			mXform.mPos.y = pos.y;
			mXform.mPos.z = pos.z;
		}
		break;
	}
	}
}

// 0x8018D5C8
// Always drawn while it bursts
void TEneLastExBossCore::vfn_10()
{
	if (mAlpha <= 0) {
		mFlags |= 0x10;
	} else if (mState == CORE_BURST) {
		mFlags &= ~0x10;
	} else if (cameraIsVisible(2500.0f, 50.0f, &mXform.mPos, NULL)) {
		mFlags &= ~0x10;
	} else {
		mFlags |= 0x10;
	}
}

// 0x8018D660
// Under full alpha it is drawn see-through
void TEneLastExBossCore::render()
{
	njSetTexture(mTexList);
	njPushMatrixEx();
	njTranslateEx(&mXform.mPos);
	gjRotateY(mXform.mRot.y);
	gjRotateX(mXform.mRot.x);
	gjRotateZ(mXform.mRot.z);
	ObjDrawState state;
	DrawStateSlot* slot = getDrawStateSlot();
	state = *getDefaultDrawState();
	if (mAlpha < 255) {
		state.mFlags |= 0x200;
		state.mAmbient = (u8)mAlpha;
	}
	mTint.addToLight((u32*)&state);
	beginDrawState(slot, &state);
	shadowAlphaBegin();
	f32 frame = motionCtrlFrame(&mClip);
	gjDrawMotion(mModel, motionCtrlGetMotion(&mClip), frame);
	shadowAlphaEnd();
	endDrawState(slot);
	njPopMatrixEx();
}

// 0x8018D788
extern "C" void dropLastBossShot(const Vec3f* pos)
{
	new TObjLastBossShot(gTL_04, pos);
}
