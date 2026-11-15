#include "stage/SetManager.h"
#include "object/TObjPilingSwitch.h"
#include "stage/EventScript.h"
#include "Ninja/ninja.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "object/ObjArchive.h"
#include "stage/StageObjUtil.h"
#include "camera/TCamera.h"
#include "stage/TStageOrange.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "object/ObjAction.h"
#include "camera/TCamPath.h"
#include "engine/FxSystem.h"
#include "core/MathUtil.h"
#include "egg/EggCreature.h"
#include "engine/Colli.h"
#include "stage/SetSlot.h"
#include "engine/GroundQuery.h"
#include "engine/MeshPool.h"
#include "engine/WorldMesh.h"
#include "object/TObjKey00.h"
#include "core/VecMath.h"
#include "engine/LightSet.h"
#include "engine/Color.h"
#include "camera/TCameraCtrl.h"
#include "object/last/TObjLastResonanceSwitch.h"

const char* gTObjPilingSwitchName = "TObjPilingSwitch";
struct CharaColli;
struct MeshColli;
struct MeshData;
struct MeshObject;
struct SetLink;
struct EnemyLight;
struct DrawState;
struct Fx;

static inline Vec3f colliNeg(const Vec3f& v)
{
	Vec3f r;
	r.x = -v.x;
	r.y = -v.y;
	r.z = -v.z;
	return r;
}

typedef char TObjPilingSwitch_size_check[(sizeof(TObjPilingSwitch) == 0x1A4) ? 1 : -1];

const f32 gPilingSwitchSetRange = 800.0f;

// 0x800C343C
// The set table's init
extern "C" void pilingSwitchSetInit()
{
}

// 0x800C3440
// The set table's exit
extern "C" void pilingSwitchSetExit()
{
}

// 0x800C3444
extern "C" TObject* createTObjPilingSwitch(TObject* parent, SetDesc* desc)
{
	return new TObjPilingSwitch(parent, desc);
}

// 0x800C3498
void TObjPilingSwitch::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	NJS_OBJECT* entry = getStgObjModels()[32];
	mCenter           = *(Vec3f*)&entry->model->center;
	mRadius           = 1.5f * entry->model->r;
	vecAdd(&mCenter, &mCenter, &mPos);

	mEventId  = desc->getInt(1);
	mSwitchId = desc->getInt(3);
	mSwitchId = (mSwitchId > 0x7FF) ? 0x7FF : mSwitchId;

	int off;
	if (desc->getInt(2) < 0) {
		off = 0;
	} else if (desc->getInt(2) > 1) {
		off = 1;
	} else {
		off = desc->getInt(2);
	}
	mInMirror = 1 - off;
}

// 0x800C35D0
void TObjPilingSwitch::debugPrint(void*)
{
	njPrintC(0xA0014, ":CALL EVENT ID");
	njPrint(0xA0015, ":DRAW TO MIRROR %s", (mInMirror != 0) ? "ON" : "OFF");
	njPrintC(0xA0016, ":SW ID");
	njPrintC(0xA0017, "");
}

// 0x800C3658
TObjPilingSwitch::TObjPilingSwitch(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00    = (u32)gTObjPilingSwitchName;
	mSize  = 0x1A4;
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 0xFF, 0xFF, 0xF7);
	initFromDesc(desc);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
	vfn_74();
}

// 0x800C3748
TObjPilingSwitch::~TObjPilingSwitch()
{
	meshColliFree(mBaseColli);
	meshColliFree(mPileColli);
	colliRetire(mShape);
	mShape = 0;
	removeDrawTask(mDrawTask);
}

// 0x800C37F4
static void setColliBox(void* colli, const Vec3f* center, f32 radius)
{
	Vec3f size;
	size.x = 1.0f;
	size.y = 1.0f;
	size.z = 1.0f;
	gjScaleVector(radius, &size, &size);

	Vec3f high;
	high.x = size.x;
	high.y = size.y;
	high.z = size.z;
	Vec3f low;
	Vec3f t = colliNeg(size);
	low.x   = t.x;
	low.y   = t.y;
	low.z   = t.z;

	gjPushUnitMatrix();
	njTranslateEx(center);
	njCalcPointEx(&high, &high);
	njCalcPointEx(&low, &low);
	njPopMatrixEx();

	f32 box[7];
	boxAroundLine(box, &low, &high);
	meshColliSetBox((MeshObject*)colli, (const f32*)box);
}

// 0x800C38E0
void TObjPilingSwitch::vfn_74()
{
	mPause        = 0;
	mLight.mTimer = 0;
	updateCylinder();

	mPile.setModel(getStgObjModels()[32]);
	mPile.mTexList = getStgObjTextures()[31];
	mPile.mState   = &mState;
	mBase.setModel(getStgObjModels()[33]);
	mBase.mTexList = getStgObjTextures()[32];
	mBase.mState   = &mState;

	mPileColli = (MeshObject*)meshColliCreate();
	meshSetFlag2(mPileColli, 1);
	meshColliSetData(mPileColli, (MeshData*)gStgObjCommon->mSet.mMeshes[4]);
	setColliBox(mPileColli, &mCenter, mRadius);
	mBaseColli = (MeshObject*)meshColliCreate();
	meshColliSetData(mBaseColli, (MeshData*)gStgObjCommon->mSet.mMeshes[5]);
	setColliBox(mBaseColli, &mCenter, mRadius);

	if (keyBitIsSet(mSwitchId) == 1) {
		mPileOffset = gPileDownOffset;
		colliDisable(mShape);
		meshColliRefresh(mPileColli);
		mAct = 2;
	} else {
		mPileOffset.x = 0.0f;
		mPileOffset.y = 0.0f;
		mPileOffset.z = 0.0f;
		mPileOffset.y -= 8.0f;
		mAct = 0;
	}
	updateShapePos();
	updateMatrices();
	updateColli();
	mLightSet = lightSetFindArea(&mPos);
	setLinkFree((SetLink*)&mLink);
}

// 0x800C3A68
void TObjPilingSwitch::updateCylinder()
{
	Vec3f pos = mPileOffset;
	pos.y += 24.0f;
	vecAdd(&pos, &mPos, &pos);
	Vec3f offset = gVecZero;
	colliSetCylinder((CharaColli*)mShape, &pos, 0x10, &offset, 10.0f, 13.0f);
}

// 0x800C3B04
void TObjPilingSwitch::updateMatrices()
{
	mtxSetTransV(mBaseMtx, &mPos);
	mtxTranslateV(mPileMtx, mBaseMtx, &mPileOffset);
	mtxRotateZYX((f32*)mPileMtx, (const f32*)mPileMtx, (const int*)&mRot);
	mtxRotateZYX((f32*)mBaseMtx, (const f32*)mBaseMtx, (const int*)&mRot);
}

// 0x800C3B68
void TObjPilingSwitch::updateShapePos()
{
	Vec3f pos = mPos;
	vecAdd(&pos, &pos, &mPileOffset);
	pos.y += 24.0f;
	colliShapeSetPos(colliGetShape(mShape, 0), &pos);
}

// 0x800C3BDC
void TObjPilingSwitch::updateColli()
{
	meshColliSetMatrix(mPileColli, mPileMtx);
	meshColliSetMatrix(mBaseColli, mBaseMtx);
}

// 0x800C3C1C
void TObjPilingSwitch::draw()
{
	if (!isInRange(this, &mPos, 850.0f)) {
		mFlags |= 1;
		return;
	}
	if (objDebugEnabled(this) != 0) {
		updateMatrices();
		updateShapePos();
		updateColli();
		return;
	}
	switch (mAct) {
	case 0:
		stateWait();
		break;
	case 1:
		stateIn();
		break;
	case 2:
		stateDone();
		break;
	}
}

// 0x800C3CDC
void TObjPilingSwitch::stateWait()
{
	if (mPause > 0) {
		mPause--;
		return;
	}
	TObjAction* other;
	for (ColliContact* hit = mShape->mContacts; hit != 0; hit = hit->mNext) {
		ColliShape* shape = colliGetShape(hit->mOther, hit->mOtherShape);
		if ((shape->mFlags & 0x400) == 0) {
			continue;
		}
		other = (TObjAction*)findObjById((u16)colliContactOwner(hit));
		if (other == 0) {
			continue;
		}
		HitInfo info;
		if (other->onHit(this, 0, (int*)&info) == 0) {
			continue;
		}
		if (info.mFlags.testBit(15) == 0) {
			continue;
		}
		mPileOffset.y -= 24.0f;
		mPause = 30;
		if (mPileOffset.y - gPileDownOffset.y <= 0.0f) {
			mPileOffset = gPileDownOffset;
			colliDisable(mShape);
			meshColliRefresh(mPileColli);
			fxSpawnAt(1, 7, 0x43, mBaseMtx);
			mPause = 30;
			mAct   = 1;
		}
		updateMatrices();
		updateShapePos();
		updateColli();
		return;
	}
}

// 0x800C3E50
void TObjPilingSwitch::stateIn()
{
	if (mPause > 0) {
		mPause--;
		return;
	}
	setKeyBit(mSwitchId);
	eventScriptRequest(mEventId);
	mAct = 2;
}

// 0x800C3EA8
void TObjPilingSwitch::stateDone()
{
}

// 0x800C3EAC
void TObjPilingSwitch::vfn_10()
{
	if (mInMirror == 0 && mirrorIsReflecting() != 0) {
		mDrawTask->mFlags |= 1;
		return;
	}
	if (mLight.isVisible(mDrawTask, 700.0f, &mCenter, mRadius, 0) != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
	setDrawAmbient(&mLight, &mState);
}

// 0x800C3F54
void TObjPilingSwitch::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mBaseMtx);
	lightSetSelect(mLightSet);
	mBase.draw();
	njPopMatrixEx();

	njPushMatrixEx();
	njMultiMatrix(0, mPileMtx);
	lightSetSelect(mLightSet);
	mPile.draw();
	njPopMatrixEx();
}

struct Vec3f;
struct MeshObject;
struct ColliShape;
struct CharaColli;
struct TObjAction;

extern "C" {
}

typedef char TObjLastResonanceSwitch_size_check[(sizeof(TObjLastResonanceSwitch) == 0x1B0) ? 1 : -1];

extern const char* gTObjLastRippleName;

// 0x800C3FC4
extern "C" TObject* createTObjLastResonanceSwitch(TObject* parent, SetDesc* desc)
{
	return new TObjLastResonanceSwitch(parent, desc);
}

const char* gTObjLastResonanceSwitchName = "TObjLastResonanceSwitch";

// 0x800C4018
TObjLastResonanceSwitch::TObjLastResonanceSwitch(TObject* parent, SetDesc* desc)
    : TObjPilingSwitch(parent, desc)
{
	mTimer = 0;
	mMode  = 0;
	_00    = (u32)gTObjLastResonanceSwitchName;
	mSize  = 0x1B0;
	mRing  = colliCreate(1, 1);
	colliSetOwner(mRing, (u16)mObjId);
	colliSetMasks(mRing, 1, 0xFF, 0xFF, 0xFE);
	colliShapeMakeCylinder(colliGetShapes(mRing), &mPos, 1, 12.5f, 12.0f);
	mState.mFlags   = DRAWSTATE_AMBIENT;
	mState.mAmbient = 0xFFFFFF00;
	if (mAct == 2) {
		setColorAlpha(&mState.mAmbient, 0xFF);
		mMode = 2;
	} else {
		meshColliRefresh((MeshObject*)mPileColli);
		meshColliRefresh((MeshObject*)mBaseColli);
	}
}

// 0x800C4104
TObjLastResonanceSwitch::~TObjLastResonanceSwitch()
{
	colliRetire(mRing);
}

// 0x800C416C
void TObjLastResonanceSwitch::draw()
{
	if (objDebugEnabled(this) != 0) {
		TObjPilingSwitch::draw();
		colliShapeMakeCylinder(colliGetShapes(mRing), &mPos, 1, 12.5f, 12.0f);
		setColorAlpha(&mState.mAmbient, 0xFF);
	} else {
		switch (mMode) {
		case 0:
			stateIdle();
			break;
		case 1:
			stateLit();
			break;
		case 2:
			stateHeld();
			break;
		}
		if ((mState.mAmbient & 0xFF) == 0xFF) {
			moveDrawTask(mDrawTask, 0x21);
		} else {
			moveDrawTask(mDrawTask, 0x2A);
		}
	}
}

// 0x800C4248
void TObjLastResonanceSwitch::stateIdle()
{
	TObjPilingSwitch::draw();
	for (ColliContact* hit = mRing->mContacts; hit != 0; hit = hit->mNext) {
		TObjAction* obj = findObjById(colliContactOwner(hit));
		if (obj != 0 && obj->_00 == (u32)gTObjLastRippleName) {
			meshClearState400(mPileColli);
			meshClearState400(mBaseColli);
			mTimer = 300;
			mMode  = 1;
			break;
		}
	}
	addColorAlpha(&mState.mAmbient, -2);
}

// 0x800C42E4
void TObjLastResonanceSwitch::stateLit()
{
	TObjPilingSwitch::draw();
	if (mAct == 2) {
		setColorAlpha(&mState.mAmbient, 0xFF);
		mMode = 2;
	} else {
		addColorAlpha(&mState.mAmbient, 2);
		if (mTimer == 0) {
			meshColliRefresh(mPileColli);
			meshColliRefresh(mBaseColli);
			mMode = 0;
		}
		mTimer = mTimer - 1;
	}
}

// 0x800C4370
void TObjLastResonanceSwitch::stateHeld()
{
	TObjPilingSwitch::draw();
}

// 0x800C4390
void TObjLastResonanceSwitch::vfn_10()
{
	if ((mState.mAmbient & 0xFF) == 0) {
		mDrawTask->mFlags |= 1;
		return;
	}
	f32 view[3];
	if (cameraIsVisible(700.0f, mRadius, &mCenter, (Vec3f*)view) != 0) {
		mDrawTask->mSortKey = drawTaskDepthKey(view[2]);
		mDrawTask->mFlags &= 0xFFFE;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x800C4428
void TObjLastResonanceSwitch::render()
{
	TObjPilingSwitch::render();
	if (objDebugEnabled(this) != 0) {
		colliDrawShapes(mRing);
	}
}

/** How far down a driven pile sits */
// 0x802803B0
// how far down a driven pile sits
PileDownOffset gPileDownOffset(0.0f, -23.0f, 0.0f);
