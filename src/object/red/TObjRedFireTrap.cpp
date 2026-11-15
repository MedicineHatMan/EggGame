#include "object/red/TObjRedFireTrap.h"
#include "engine/DrawTask.h"
#include "object/ObjScript.h"
#include "engine/CharaColli.h"
#include "object/ObjArchive.h"
#include "engine/TParticle.h"
#include "object/TObjLauncher.h"
#include "core/MathUtil.h"
#include "camera/TCameraCtrl.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"
#include "core/VecMath.h"
#include "stage/SetSlot.h"
struct SetLink;

extern "C" {
// 0x802C3E20
ObjClassName gTObjRedFireTrapName = { "TObjRedFireTrap", 0 };
extern const MotionEntry sRedFireTrapMotions[]
    = { { 1, 1, -1, 0, 0.0f, 29.0f, 0.4f, 0.0f, 0 }, { 1, 2, -1, 0, 0.0f, 14.0f, 0.4f, 0.0f, 0 } };
}

typedef char TObjRedFireTrap_size_check[(sizeof(TObjRedFireTrap) == 0x144) ? 1 : -1];

// 0x80085500
extern "C" void fireListCreate()
{
	gFireList = new FireList;
}

// 0x80085538
extern "C" void fireListFree()
{
	delete gFireList;
}

// 0x80085560
FireList::FireList()
{
	mHead  = 0;
	mTail  = 0;
	mCount = 0;
}

// 0x80085574
FireList::~FireList()
{
	clear();
}

// 0x800855BC
// Unlinks every node
void FireList::clear()
{
	FireNode* node = mHead;
	while (node != 0) {
		FireNode* next = node->mNext;
		node->mOwner   = 0;
		node->mNext    = 0;
		node->mPrev    = 0;
		node           = next;
	}
}

// 0x800855E8
// The set table's init
extern "C" void fireTrapSetInit()
{
}

// 0x800855EC
// The set table's exit
extern "C" void fireTrapSetExit()
{
}

// 0x800855F0
extern "C" TObject* createTObjRedFireTrap(TObject* parent, SetDesc* desc)
{
	return new TObjRedFireTrap(parent, desc);
}

// 0x80085644
void TObjRedFireTrap::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mEternal = desc->getInt(0);
	mPower   = desc->getInt(1) + 25;
	mOnTime  = desc->getInt(2) + 120;
	mOffTime = desc->getInt(3) + 180;
	makeShapes();

	Vec3f top;
	top.x = 0.0f;
	top.y = -4.0f;
	top.z = 2.0f;

	f32 mtx[12];
	mtxSetRotY(mtx, mRot.y);
	mtxRotateX(mtx, mtx, mRot.x);
	mtxMultVec(mtx, &top, &top);
	vecAdd(&top, &top, &mPos);

	mTrail->mPos = top;
	mTrail->setRot(mRot.x, mRot.y);
}

// 0x80085778
// The strings are at 0x802434C4 0x802434D4 0x802434E0 and 0x802434EC
void TObjRedFireTrap::debugPrint(void*)
{
	njPrintC(0xA0013, ":ETERNAL FLAG");
	njPrintC(0xA0014, "+25:POWER");
	njPrint(0xA0015, "+%d:ON TIME", 120);
	njPrint(0xA0016, "+%d:OFF_TIME", 180);
}

// 0x800857F4
// Kind 0 takes mPower
int TObjRedFireTrap::onHit(const TObjAction* other, int, int* info)
{
	if (((TObjAction*)other)->getKind() == 0) {
		resetHitInfo((HitInfo*)info);
		*info = mPower;
		return 1;
	}
	return 0;
}

// 0x80085858
TObjRedFireTrap::TObjRedFireTrap(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjRedFireTrapName.mName;
	mSize = 0x144;

	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 255, 246, 254);

	mJetColli = colliCreate(7, 1);
	colliSetOwner(mJetColli, (u16)mObjId);
	colliSetMasks(mJetColli, 4, 255, 255, 254);

	mDraw  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mTrail = particleGenById(this, &mPos, 0, 0xEC);
	initFromDesc(desc);
	vfn_74();
}

// 0x80085990
TObjRedFireTrap::~TObjRedFireTrap()
{
	seStop3D(&mSound);
	mTrail->mFlags |= 1;
	removeDrawTask(mDraw);
	colliRetire(mJetColli);
	colliRetire(mShape);
	mShape = 0;
}

// 0x80085A34
void TObjRedFireTrap::vfn_74()
{
	mTimer = 0;
	updateMatrix();
	mMotion.init((const MotionEntry*)sRedFireTrapMotions, gGeObjCommon->mSet.mMotions);
	mModel.setModel(getGeObjModels()[1]);
	mModel.mTexList = getGeObjTextures()[1];
	mModel.mState   = (DrawState*)getDefaultDrawState();
	mModel.mMotion  = &mMotion;
	setLinkFree((SetLink*)&mLink);

	if (mEternal != 0) {
		mMotion.play(1);
		mTrail->mFlags &= ~4;
		((ColliShape*)colliGetShapes(mShape))->mFlags |= 8;
		colliEnable(mJetColli);
		sePlay3DHeld(300.0f, &mSound, 0x223, 100, &mPos);
		mAct = 3;
	} else {
		mMotion.play(0);
		mTrail->mFlags |= 4;
		((ColliShape*)colliGetShapes(mShape))->mFlags &= ~8;
		colliDisable(mJetColli);
		mAct = 0;
	}
}

// 0x80085B60
void TObjRedFireTrap::makeShapes()
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
	ColliShape shapes[7];
	for (int i = 0; i < 7; i++) {
		njAddVector(&pos, &step);
		colliShapeMakeSphere(&shapes[i], &pos, 0, 12.0f);
		shapes[i].mFlags |= 0x18;
	}
	colliCopyShapes(mJetColli, shapes);
	colliRefreshBounds(mJetColli);
}

// 0x80085C60
void TObjRedFireTrap::updateMatrix()
{
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateZYX((f32*)mMatrix, (const f32*)mMatrix, (const int*)&mRot);
}

// 0x80085CA4
void TObjRedFireTrap::draw()
{
	if (isInRange(this, &mPos, 900.0f) == 0) {
		mFlags |= 1;
		return;
	}
	if (objDebugEnabled(this) != 0) {
		mTrail->mFlags &= ~4;
		updateMatrix();
		return;
	}
	switch (mAct) {
	case 0:
		stateIgnite();
		break;
	case 1:
		stateOn();
		break;
	case 2:
		stateOff();
		break;
	}
}

// 0x80085D64
void TObjRedFireTrap::stateIgnite()
{
	if (mMotion.isDone() != 0) {
		((ColliShape*)colliGetShapes(mShape))->mFlags |= 8;
		colliEnable(mJetColli);
		mTrail->mFlags &= ~4;
		sePlay3DHeld(300.0f, &mSound, 0x223, 100, &mPos);
		mAct = 1;
	}
	mMotion.update();
	mModel.calcMotion();
}

// 0x80085DF8
void TObjRedFireTrap::stateOn()
{
	if (++mTimer > mOnTime && mEternal == 0) {
		mTimer = 0;
		((ColliShape*)colliGetShapes(mShape))->mFlags &= ~8;
		colliDisable(mJetColli);
		mTrail->mFlags |= 4;
		mMotion.play(1);
		seStop3D(&mSound);
		mAct = 2;
	}
}

// 0x80085E94
void TObjRedFireTrap::stateOff()
{
	if (mMotion.isDone() == 0) {
		mMotion.update();
		mModel.calcMotion();
	} else {
		if (++mTimer > mOffTime) {
			mTimer = 0;
			mMotion.play(0);
			mAct = 0;
		}
	}
}

// 0x80085F14
void TObjRedFireTrap::vfn_10()
{
	sRedFireTrapVisible = cameraIsVisible(800.0f, 96.0f, &mPos, 0);
	if (sRedFireTrapVisible != 0) {
		mDraw->mFlags &= 0xFFFE;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x80085F80
void TObjRedFireTrap::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	mModel.draw();
	njPopMatrixEx();
}

// 0x80085FC4
extern "C" void redFireTrapStubA()
{
}

// 0x80085FC8
extern "C" void redFireTrapStubB()
{
}
