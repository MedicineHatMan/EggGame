#include "stage/SetActor.h"
struct DrawTask;
#include "object/ObjScript.h"
#include "sound/SoundHandle.h"
#include "object/ObjAction.h"
#include "object/ObjModel.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "object/ObjArchive.h"
#include "camera/TCamera.h"
#include "engine/TParticle.h"
#include "object/TObjLauncher.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "core/MathUtil.h"
#include "camera/TCameraCtrl.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"
#include "core/VecMath.h"
#include "stage/SetSlot.h"
#include "object/yellow/TObjYellowFireTrap.h"
struct CharaColli;
struct SetLink;

extern "C" {
// 0x802C4350
ObjClassName gTObjYellowFireTrapName = { "TObjYellowFireTrap", 0 };
}

#define sFireTrapVisible sYellowFireTrapVisible

typedef char TObjYellowFireTrap_size_check[(sizeof(TObjYellowFireTrap) == 0x104) ? 1 : -1];

// 0x8012E350
extern "C" TObject* createTObjYellowFireTrap(TObject* parent, SetDesc* desc)
{
	return new TObjYellowFireTrap(parent, desc);
}

// 0x8012E3A4
// Power is the second int plus 25 on time the third plus 120 and off time the fourth plus 180
void TObjYellowFireTrap::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mEternal = desc->getInt(0);
	mPower   = desc->getInt(1) + 25;
	mOnTime  = desc->getInt(2) + 120;
	mOffTime = desc->getInt(3) + 180;
	makeShapes();

	Vec3f top;
	top.x = 0.0f;
	top.y = 8.0f;
	top.z = 2.0f;

	f32 mtx[12];
	mtxSetRotY(mtx, mRot.y);
	mtxRotateX(mtx, mtx, mRot.x);
	mtxMultVec(mtx, &top, &top);
	vecAdd(&top, &top, &mPos);

	mTrail->mPos = top;
	mTrail->setRot(mRot.x, mRot.y);
}

// 0x8012E4D8
void TObjYellowFireTrap::debugPrint(void*)
{
	njPrintC(0xA0013, ":ETERNAL FLAG");
	njPrintC(0xA0014, "+25:POWER");
	njPrint(0xA0015, "+%d:ON TIME", 120);
	njPrint(0xA0016, "+%d:OFF_TIME", 180);
}

// 0x8012E554
// The player kind 0 takes mPower
int TObjYellowFireTrap::onHit(const TObjAction* other, int, int* info)
{
	if (other == 0) {
		return 0;
	}
	if (((TObjAction*)other)->getKind() == 0) {
		resetHitInfo((HitInfo*)info);
		*info = mPower;
		return 1;
	}
	return 0;
}

// 0x8012E5C8
TObjYellowFireTrap::TObjYellowFireTrap(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00       = (u32)gTObjYellowFireTrapName.mName;
	mSize     = 0x104;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);

	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 255, 246, 246);

	mJetColli = colliCreate(7, 1);
	colliSetOwner(mJetColli, (u16)mObjId);
	colliSetMasks(mJetColli, 4, 255, 255, 254);

	mTrail = particleGenById(this, &mPos, 0, 0xEC);
	initFromDesc(desc);
	vfn_74();
}

// 0x8012E6F8
TObjYellowFireTrap::~TObjYellowFireTrap()
{
	seStop3D(&mSound);
	if (mTrail != 0) {
		mTrail->mFlags |= 1;
		mTrail = 0;
	}
	colliRetire(mJetColli);
	colliRetire(mShape);
	mShape = 0;
	removeDrawTask(mDrawTask);
}

// 0x8012E7AC
void TObjYellowFireTrap::vfn_74()
{
	mTimer = 0;
	updateMatrix();
	mModel.setModel(getGeObjModels()[1]);
	mModel.mTexList = getGeObjTextures()[1];
	mModel.mState   = (DrawState*)getDefaultDrawState();
	setLinkFree((SetLink*)&mLink);

	((ColliShape*)colliGetShapes(mShape))->mFlags |= 8;
	colliEnable(mJetColli);
	mTrail->mFlags &= ~4;

	if (mEternal != 0) {
		sePlay3DHeld(300.0f, &mSound, 0x262, 100, &mPos);
		mAct = 2;
	} else {
		mAct = 0;
	}
}

// 0x8012E874
void TObjYellowFireTrap::makeShapes()
{
	f32 mtx[12];
	mtxSetRotY(mtx, mRot.y);
	mtxRotateX(mtx, mtx, mRot.x);

	ColliShape shape;
	colliShapeMakeSphere(&shape, &mPos, 0, 20.0f);
	colliCopyShapes(mShape, &shape);

	Vec3f step;
	step.x = 0.0f;
	step.y = 0.0f;
	step.z = 12.0f;
	mtxMultVec(mtx, &step, &step);

	Vec3f pos = mPos;
	pos.y += 8.0f;

	ColliShape shapes[7];
	for (int i = 0; i < 7; i++) {
		njAddVector(&pos, &step);
		colliShapeMakeSphere(&shapes[i], &pos, 0, 12.0f);
		shapes[i].mFlags |= 0x18;
	}
	colliCopyShapes(mJetColli, shapes);
	colliRefreshBounds(mJetColli);
}

// 0x8012E984
void TObjYellowFireTrap::updateMatrix()
{
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
	mtxRotateX(mMatrix, mMatrix, mRot.x);
}

// 0x8012E9D8
void TObjYellowFireTrap::draw()
{
	if (!isInRange(this, &mPos, 900.0f)) {
		mFlags |= 1;
		return;
	}
	if (objDebugEnabled(this) != 0) {
		mTrail->mFlags &= ~4;
		mTrail->mFlags &= ~0x10;
		updateMatrix();
		return;
	}
	switch (mAct) {
	case 0:
		stateOn();
		break;
	case 1:
		stateOff();
		break;
	}
}

// 0x8012EA90
void TObjYellowFireTrap::stateOn()
{
	if (++mTimer > mOnTime) {
		mTimer = 0;
		((ColliShape*)colliGetShapes(mShape))->mFlags &= ~8;
		colliDisable(mJetColli);
		mTrail->mFlags |= 4;
		seStop3D(&mSound);
		mAct = 1;
	}
}

// 0x8012EB14
void TObjYellowFireTrap::stateOff()
{
	if (++mTimer > mOffTime) {
		mTimer = 0;
		((ColliShape*)colliGetShapes(mShape))->mFlags |= 8;
		colliEnable(mJetColli);
		mTrail->mFlags &= ~4;
		sePlay3DHeld(300.0f, &mSound, 0x262, 100, &mPos);
		mAct = 0;
	}
}

// 0x8012EBA8
void TObjYellowFireTrap::vfn_10()
{
	sFireTrapVisible = cameraIsVisible(800.0f, 96.0f, &mPos, 0);
	if (sFireTrapVisible != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x8012EC14
void TObjYellowFireTrap::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	mModel.draw();
	njPopMatrixEx();
}
