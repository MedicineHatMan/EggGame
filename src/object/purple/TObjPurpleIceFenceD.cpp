#include "object/purple/TObjPurpleIceFenceD.h"
struct DrawTask;
#include "object/ObjScript.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "Ninja/ninja.h"
#include "object/ObjArchive.h"
#include "core/MathUtil.h"
#include "camera/TCamPath.h"
#include "engine/Colli.h"
#include "engine/GroundQuery.h"
#include "engine/MeshPool.h"
#include "engine/WorldMesh.h"
#include "stage/SetSlot.h"
struct SetLink;
struct MeshColli;
struct MeshData;
struct MeshObject;
struct EnemyLight;
struct DrawState;

static inline Vec3f colliNeg(const Vec3f& v)
{
	Vec3f r;
	r.x = -v.x;
	r.y = -v.y;
	r.z = -v.z;
	return r;
}

extern "C" {

// 0x802C41A8
ObjClassName gTObjPurpleIceFenceDName = { "TObjPurpleIceFenceD", 0 };
}

typedef char TObjPurpleIceFenceD_size_check[(sizeof(TObjPurpleIceFenceD) == 0x144) ? 1 : -1];

// 0x80101A9C
extern "C" TObject* createTObjPurpleIceFenceD(TObject* parent, SetDesc* desc)
{
	return new TObjPurpleIceFenceD(parent, desc);
}

// 0x80101AF0
TObjPurpleIceFenceD::TObjPurpleIceFenceD(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjPurpleIceFenceDName.mName;
	mSize = 0x144;

	mModelData = findGeObjModel("MODEL_ICE_FENCE_D");
	mTexList   = (NJS_TEXLIST*)findGeObjData("TLS_MODEL_ICE_FENCE_D");
	mColliData = (MeshData*)findGeObjData("COL_MODEL_ICE_FENCE_D");
	mDrawTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	makeShape();
	initColli();
	initFromDesc(desc);
	vfn_74();
}

// 0x80101BE0
TObjPurpleIceFenceD::~TObjPurpleIceFenceD()
{
	freeColli();
	freeShape();
	removeDrawTask(mDrawTask);
}

// 0x80101C6C
void TObjPurpleIceFenceD::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mPower = desc->getInt(0);
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRot.y);

	mCenter  = *(Vec3f*)&mModelData->model->center;
	mtxMultVec(mMatrix, &mCenter, &mCenter);
	mRadius = mModelData->model->r;
	updateShape();
	updateColli();
}

// 0x80101D24
void TObjPurpleIceFenceD::debugPrint(void*)
{
	njPrintC(0xA0013, ":POWER");
	njPrintC(0xA0014, "");
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, "");
	njPrintC(0xA0017, "");
	njPrintC(0xA0018, "");
	njPrintC(0xA0019, "");
	njPrintC(0xA001A, "");
}

// 0x80101DC0
void TObjPurpleIceFenceD::vfn_74()
{
	mModel.setModel(mModelData);
	mModel.mTexList = mTexList;
	mModel.mState   = &mState;
	mAct            = 0;
	setLinkFree((SetLink*)&mLink);
}

// 0x80101E14
void TObjPurpleIceFenceD::makeShape()
{
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 0xFF, 0xF4, 0xF4);
}

// 0x80101E70
// 23 by 16 by 8 with its centre 16 above mPos
void TObjPurpleIceFenceD::updateShape()
{
	Vec3f pos = mPos;
	pos.y += 16.0f;

	ColliShape shape;
	colliShapeMakeBox(&shape, &pos, mRot.y, 23.0f, 16.0f, 8.0f, 0);
	shape.mFlags |= 8;
	colliCopyShapes(mShape, &shape);
}

// 0x80101EF4
void TObjPurpleIceFenceD::freeShape()
{
	colliRetire(mShape);
	mShape = 0;
}

// 0x80101F2C
// 1.0f is 1
void TObjPurpleIceFenceD::setColliBox(void* colli, const Vec3f* center, f32 radius)
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
	low.x = t.x;
	low.y = t.y;
	low.z = t.z;

	gjPushUnitMatrix();
	njTranslateEx(center);
	njCalcPointEx(&high, &high);
	njCalcPointEx(&low, &low);
	njPopMatrixEx();

	f32 box[6];
	boxAroundLine(box, &low, &high);
	meshColliSetBox((MeshObject*)colli, (const f32*)box);
}

// 0x80102018
void TObjPurpleIceFenceD::initColli()
{
	mColli = (MeshColli*)meshColliCreate();
	meshColliSetData((MeshObject*)mColli, mColliData);
}

// 0x80102054
void TObjPurpleIceFenceD::updateColli()
{
	setColliBox(mColli, &mCenter, mRadius);
	meshColliSetMatrix(mColli, (const f32*)mMatrix);
}

// 0x80102098
void TObjPurpleIceFenceD::freeColli()
{
	meshColliFree(mColli);
}

// 0x801020BC
int TObjPurpleIceFenceD::onHit(const TObjAction*, int, int* info)
{
	HitInfo* hit = (HitInfo*)info;
	resetHitInfo(hit);
	hit->mFlags.setBit(13);
	hit->mDamage = mPower;
	return 1;
}

// 0x80102108
// 850.0f is 850
void TObjPurpleIceFenceD::draw()
{
	if (objDebugEnabled(this) != 0) {
		return;
	}
	if (!isInRange(this, &mPos, 850.0f)) {
		mFlags |= 1;
		return;
	}
	switch (mAct) {
	case 0:
			stateWait();
			break;
	case 1:
			stateDone();
			break;
	}
}

// 0x80102194
void TObjPurpleIceFenceD::stateWait()
{
}

// 0x80102198
void TObjPurpleIceFenceD::stateDone()
{
	setLinkDone((SetLink*)&mLink);
	mFlags |= 1;
}

// 0x801021D4
// 700.0f is 700
void TObjPurpleIceFenceD::vfn_10()
{
	if (mLight.isVisible(mDrawTask, 700.0f, &mCenter, mRadius, 0) != 0) {
		setDrawAmbient(&mLight, &mState);
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x80102250
void TObjPurpleIceFenceD::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	mModel.draw();
	njPopMatrixEx();
	if (objDebugEnabled(this) != 0) {
		colliDrawShapes(mShape);
	}
}

// 0x801022AC
// The set hook run when the stage loads empty
extern "C" void initPurpleIceFenceDSet()
{
}

// 0x801022B0
// The set hook run when the stage ends empty
extern "C" void termPurpleIceFenceDSet()
{
}
