#include "stage/SetManager.h"
#include "object/purple/TObjPurpleIceFloor.h"
#include "engine/DrawTask.h"
#include "engine/CharaColli.h"
#include "object/ObjScript.h"
#include "Ninja/ninja.h"
#include "object/ObjArchive.h"
#include "stage/StageObjUtil.h"
#include "object/TObjLauncher.h"
#include "camera/TCameraCtrl.h"
#include "object/ObjAction.h"
#include "core/MathUtil.h"
#include "sound/Sound3DPlay.h"
#include "engine/Colli.h"
#include "engine/GroundQuery.h"
#include "engine/MeshPool.h"
#include "engine/WorldMesh.h"
#include "stage/SetSlot.h"
struct Vec3f;
struct SetLink;
struct MeshColli;
struct MeshData;
struct MeshObject;
struct ColliContact;

static inline Vec3f colliNeg(const Vec3f& v)
{
	Vec3f r;
	r.x = -v.x;
	r.y = -v.y;
	r.z = -v.z;
	return r;
}

const f32 gPurpleIceFloorSetRange = 1000.0f;

// 0x801033EC
// and 0x801033F0 are bare returns

extern "C" {

// 0x802C41B8
ObjClassName gTObjPurpleIceFloorName = { "TObjPurpleIceFloor", 0 };
}

typedef char TObjPurpleIceFloor_size_check[(sizeof(TObjPurpleIceFloor) == 0xE0) ? 1 : -1];

// 0x80102B5C
extern "C" TObject* createTObjPurpleIceFloor(TObject* parent, SetDesc* desc)
{
	return new TObjPurpleIceFloor(parent, desc);
}

// 0x80102BB0
TObjPurpleIceFloor::TObjPurpleIceFloor(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjPurpleIceFloorName.mName;
	mSize = 0xE0;

	mModelData  = findGeObjModel("MODEL_ICE_FLOOR");
	mBreakModel = getGeObjModels()[5];
	mTexList    = (NJS_TEXLIST*)findGeObjData("TLS_MODEL_ICE_FLOOR");
	mColliData  = (MeshData*)findGeObjData("COL_MODEL_ICE_FLOOR");
	mDrawTask   = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	makeShape();
	initColli();
	initFromDesc(desc);
	vfn_74();
}

// 0x80102C98
TObjPurpleIceFloor::~TObjPurpleIceFloor()
{
	freeColli();
	freeShape();
	removeDrawTask(mDrawTask);
}

// 0x80102D18
void TObjPurpleIceFloor::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRot.y);

	mCenter = *(Vec3f*)&mModelData->model->center;
	mtxMultVec(mMatrix, &mCenter, &mCenter);
	mRadius = mModelData->model->r;
	updateShape();
	updateColli();
}

// 0x80102DB0
// Eight blank rows
void TObjPurpleIceFloor::debugPrint(void*)
{
	njPrintC(0xA0013, "");
	njPrintC(0xA0014, "");
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, "");
	njPrintC(0xA0017, "");
	njPrintC(0xA0018, "");
	njPrintC(0xA0019, "");
	njPrintC(0xA001A, "");
}

// 0x80102E4C
void TObjPurpleIceFloor::vfn_74()
{
	mModel.setModel(mModelData);
	mModel.mTexList = mTexList;
	mModel.mState   = (DrawState*)getDefaultDrawState();
	mAct            = 0;
	setLinkFree((SetLink*)&mLink);
}

// 0x80102EA0
void TObjPurpleIceFloor::makeShape()
{
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 0xFF, 0xFF, 0xF7);
}

// 0x80102EFC
// 140.0f is 140 and 2.5f is 2.5
void TObjPurpleIceFloor::updateShape()
{
	ColliShape shape;
	colliShapeMakeCylinder(&shape, &mPos, 1, 140.0f, 2.5f);
	shape.mFlags |= 0x10;
	colliCopyShapes(mShape, &shape);
}

// 0x80102F54
void TObjPurpleIceFloor::freeShape()
{
	colliRetire(mShape);
	mShape = 0;
}

// 0x80102F8C
// 1.0f is 1
void TObjPurpleIceFloor::setColliBox(void* colli, const Vec3f* center, f32 radius)
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

	f32 box[6];
	boxAroundLine(box, &low, &high);
	meshColliSetBox((MeshObject*)colli, (const f32*)box);
}

// 0x80103078
void TObjPurpleIceFloor::initColli()
{
	mColli = (MeshObject*)meshColliCreate();
	meshColliSetData(mColli, mColliData);
	meshSetState100(mColli, 1);
}

// 0x801030C0
void TObjPurpleIceFloor::updateColli()
{
	setColliBox(mColli, &mCenter, mRadius);
	meshColliSetMatrix(mColli, (const f32*)mMatrix);
}

// 0x80103104
void TObjPurpleIceFloor::freeColli()
{
	meshColliFree(mColli);
}

// 0x80103128
// 1050.0f is 1050
void TObjPurpleIceFloor::draw()
{
	if (objDebugEnabled(this) != 0) {
		return;
	}
	if (!isInRange(this, &mPos, 1050.0f)) {
		mFlags |= 1;
		return;
	}
	switch (mAct) {
	case 0:
		stateWait();
		break;
	case 1:
		shatter();
		break;
	}
}

// 0x801031B4
// Flag 0x400 picks the contact
void TObjPurpleIceFloor::stateWait()
{
	TObjAction* other;
	for (ColliContact* hit = mShape->mContacts; hit != 0; hit = hit->mNext) {
		if ((colliContactShape(hit)->mFlags & 0x400) == 0) {
			continue;
		}
		other = findObjById((u16)colliContactOwner(hit));
		if (other == 0) {
			continue;
		}
		HitInfo info;
		if (other->onHit(this, 0, (int*)&info) != 0 && info.mFlags.testBit(0xF) != 0) {
			ColliShape* shape = colliContactShape(hit);
			Vec3f at;
			at.set(shape->mPos.x, shape->mPos.y, shape->mPos.z);
			sePlay3D(0x238, &at);
			meshColliRefresh(mColli);
			mAct = 1;
			return;
		}
	}
}

// 0x801032A8
// 1.0f is 1 and 0.2f is 0.2
void TObjPurpleIceFloor::shatter()
{
	shatterModelRot(0, &mPos, &mPos, &mRot, (NJS_OBJECT*)mBreakModel, mTexList, 1.0f, 0.2f);
	mDrawTask->mFlags |= 1;
	setLinkDone((SetLink*)&mLink);
	mFlags |= 1;
}

// 0x80103318
void TObjPurpleIceFloor::vfn_10()
{
	u8 out[0x10];
	if (cameraIsVisible(1050.0f, mRadius, &mCenter, (Vec3f*)out) != 0) {
		mDrawTask->mSortKey = drawTaskDepthKey(*(f32*)(out + 8));
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x80103390
void TObjPurpleIceFloor::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	mModel.draw();
	njPopMatrixEx();
	if (objDebugEnabled(this) != 0) {
		colliDrawShapes(mShape);
	}
}

// 0x801033EC
// The set hook run when the stage loads empty
extern "C" void initPurpleIceFloorSet()
{
}

// 0x801033F0
// The set hook run when the stage ends empty
extern "C" void termPurpleIceFloorSet()
{
}
