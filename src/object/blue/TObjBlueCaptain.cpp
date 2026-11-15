#include "object/blue/TObjBlueCaptain.h"
#include "engine/DrawTask.h"
#include "camera/EventCamArc.h"
#include "object/ObjScript.h"
#include "egg/ObjEgg.h"
#include "object/ObjArchive.h"
#include "object/TObjLauncher.h"
#include "stage/TEventCtrl.h"
#include "core/MslMath.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "sound/Sound3DPlay.h"
#include "camera/TCameraCtrl.h"
#include "core/MathUtil.h"
#include "object/TObjKey00.h"
#include "sound/SoundCtrl.h"
#include "enemy/boss/EneBoss.h"
#include "engine/ParticleBank.h"
#include "enemy/ObjEnemy.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "engine/TParticle.h"
#include "stl/math.h"
#include "engine/Colli.h"
#include "camera/TCamEditor.h"
#include "enemy/boss/TEneBoss03_Beam_Pera.h"

extern "C" {

// 0x802C4008
ObjClassName gTObjBlueCaptainName                = { "TObjBlueCaptain", 0 };
extern const MotionEntry sBlueCaptainMotions0[4] = {
	{ MOTION_ONCE, 6, -1, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
	{ MOTION_ONCE, 6, -1, 0, 0.0f, 50.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 7, -1, 0, 0.0f, 17.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 7, -1, 0, 17.0f, 17.0f, 0.0f, 0.0f, 0 },
};
extern const MotionEntry sBlueCaptainMotions1[4] = {
	{ MOTION_ONCE, 8, -1, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
	{ MOTION_LOOP, 8, -1, 0, 0.0f, 50.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 9, -1, 0, 0.0f, 17.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 9, -1, 0, 17.0f, 17.0f, 0.0f, 0.0f, 0 },
};
extern const MotionEntry sBlueCaptainMotions2[4] = {
	{ MOTION_ONCE, 10, -1, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
	{ MOTION_LOOP, 10, -1, 0, 0.0f, 50.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 11, -1, 0, 0.0f, 17.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 11, -1, 0, 17.0f, 17.0f, 0.0f, 0.0f, 0 },
};
}

typedef char TObjBlueCaptain_size_check[(sizeof(TObjBlueCaptain) == 0x1BC) ? 1 : -1];

// 0x800BD1C8
extern "C" TObject* createTObjBlueCaptain(TObject* parent, SetDesc* desc)
{
	return new TObjBlueCaptain(parent, desc);
}

// 0x800BD21C
TObjBlueCaptain::TObjBlueCaptain(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjBlueCaptainName.mName;
	mSize = 0x1BC;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	initFromDesc(desc);
	vfn_74();
}

// 0x800BD2E4
TObjBlueCaptain::~TObjBlueCaptain()
{
	removeDrawTask(mDraw);
}

// 0x800BD370
void TObjBlueCaptain::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mOnSwitch  = desc->getInt(0);
	mOffSwitch = desc->getInt(1);
	buildMatrix();
}

// 0x800BD3E0
void TObjBlueCaptain::debugPrint(void*)
{
	njPrintC(0xA0013, ":ON SWITCH");
	njPrintC(0xA0014, ":OFF SWITCH");
	njPrintC(0xA0017, "");
}

// 0x800BD434
void TObjBlueCaptain::vfn_74()
{
	mEventCam      = 0;
	void** motions = gGeObjCommon->mSet.mMotions;
	mMotion0.init(sBlueCaptainMotions0, motions);
	mMotion1.init(sBlueCaptainMotions1, motions);
	mMotion2.init(sBlueCaptainMotions2, motions);

	mModel0.setModel(getGeObjModels()[0x12]);
	mModel0.mTexList = getGeObjTextures()[0x10];
	mModel0.mState   = getDefaultDrawState();
	mModel0.mMotion  = &mMotion0;
	mModel1.setModel(getGeObjModels()[0x13]);
	mModel1.mTexList = getGeObjTextures()[0x11];
	mModel1.mState   = getDefaultDrawState();
	mModel1.mMotion  = &mMotion1;
	mModel2.setModel(getGeObjModels()[0x14]);
	mModel2.mTexList = getGeObjTextures()[0x12];
	mModel2.mState   = getDefaultDrawState();
	mModel2.mMotion  = &mMotion2;

	if (keyBitIsSet((u16)mOffSwitch) != 0) {
		play(3);
		advance();
		mState = STATE_OFF;
	} else if (keyBitIsSet((u16)mOnSwitch) != 0) {
		play(1);
		mState = STATE_ON;
	} else {
		play(0);
		mState = STATE_WAIT;
	}
}

// 0x800BD5A8
void TObjBlueCaptain::draw()
{
	if (isInRange(this, &mPos, 1100.0f) == 0) {
		mFlags |= 1;
		return;
	}
	switch (mState) {
	case STATE_WAIT:
		stateWait();
		break;
	case STATE_START:
		stateStart();
		break;
	case STATE_ON:
		stateOn();
		break;
	case STATE_STOP:
		stateStop();
		break;
	case STATE_OFF:
		stateOff();
		break;
	}
	if (mEventCam != 0 && mEventCam->isDone() != 0) {
		eventEnd();
		mEventCam->mFlags |= 1;
		mEventCam = 0;
	}
}

// 0x800BD690
void TObjBlueCaptain::stateWait()
{
	if (keyBitIsSet((u16)mOnSwitch) != 0 && eventStartById(mObjId, 0) != 0) {
		mEventCam = eventCamArcPlay(0.05f, 0.0f, 0x100);
		play(1);
		mState = STATE_START;
	} else {
		advance();
	}
}

// 0x800BD714
void TObjBlueCaptain::stateStart()
{
	sePlay(0x20A);
	sePlay3D(0x20B, &mPos);
	mState = STATE_ON;
}

// 0x800BD758
void TObjBlueCaptain::stateOn()
{
	if (keyBitIsSet((u16)mOffSwitch) != 0) {
		play(2);
		mState = STATE_STOP;
	} else if (keyBitIsSet((u16)mOnSwitch) == 0) {
		play(0);
		mState = STATE_WAIT;
	} else if (advance() == 0) {
		play(1);
		sePlay3D(0x20B, &mPos);
	}
}

// 0x800BD800
void TObjBlueCaptain::stateStop()
{
	if (advance() == 0) {
		mState = STATE_OFF;
	}
}

// 0x800BD83C
void TObjBlueCaptain::stateOff()
{
}

// 0x800BD840
void TObjBlueCaptain::buildMatrix()
{
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateZYX((f32*)mMatrix, (const f32*)mMatrix, (const int*)&mRot);
}

// 0x800BD884
void TObjBlueCaptain::play(int id)
{
	mMotion0.play(id);
	mMotion1.play(id);
	mMotion2.play(id);
}

// 0x800BD8D4
// Updates all three while the first has not finished
int TObjBlueCaptain::advance()
{
	if (mMotion0.isDone() == 0) {
		mMotion0.update();
		mMotion1.update();
		mMotion2.update();
		return 1;
	}
	return 0;
}

// 0x800BD930
void TObjBlueCaptain::vfn_10()
{
	if (cameraIsVisible(1000.0f, 200.0f, &mPos, 0) != 0) {
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x800BD998
void TObjBlueCaptain::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	mModel0.draw();
	mModel1.draw();
	mModel2.draw();
	njPopMatrixEx();
}
