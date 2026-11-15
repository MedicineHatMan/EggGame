#include "stage/SetManager.h"
#include "object/purple/TObjPurpleIceWall.h"
#include "engine/DrawTask.h"
#include "object/ObjScript.h"
#include "engine/CharaColli.h"
#include "Ninja/ninja.h"
#include "object/ObjArchive.h"
#include "stage/StageObjUtil.h"
#include "object/ObjAction.h"
#include "object/TObjLauncher.h"
#include "camera/TCameraCtrl.h"
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

extern "C" {

const f32 gPurpleIceWallSetRange = 2000.0f;

// 0x802C41B0
ObjClassName gTObjPurpleIceWallName = { "TObjPurpleIceWall", 0 };
}

typedef char TObjPurpleIceWall_size_check[(sizeof(TObjPurpleIceWall) == 0xDC) ? 1 : -1];

// 0x801022B4
extern "C" TObject* createTObjPurpleIceWall(TObject* parent, SetDesc* desc)
{
	return new TObjPurpleIceWall(parent, desc);
}

// 0x80102308
TObjPurpleIceWall::TObjPurpleIceWall(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00        = (u32)gTObjPurpleIceWallName.mName;
	mSize      = 0xDC;
	mDrawTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	mModelData = findGeObjModel("MODEL_ICE_WALL");
	mTexList   = (NJS_TEXLIST*)findGeObjData("TLS_MODEL_ICE_WALL");
	mColliData = (MeshData*)findGeObjData("COL_MODEL_ICE_WALL");
	makeShape();
	initColli();
	initFromDesc(desc);
	vfn_74();
}

// 0x801023E4
TObjPurpleIceWall::~TObjPurpleIceWall()
{
	freeColli();
	freeShape();
	removeDrawTask(mDrawTask);
}

// 0x80102464
void TObjPurpleIceWall::initFromDesc(SetDesc* desc)
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

// 0x801024FC
// Eight blank rows
void TObjPurpleIceWall::debugPrint(void*)
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

// 0x80102598
void TObjPurpleIceWall::vfn_74()
{
	mModel.setModel(mModelData);
	mModel.mTexList = mTexList;
	mModel.mState   = (DrawState*)getDefaultDrawState();
	mAct            = 0;
	setLinkFree((SetLink*)&mLink);
}

// 0x801025EC
void TObjPurpleIceWall::makeShape()
{
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 0xFF, 0xFF, 0xD7);
}

// 0x80102648
// 85 by 70 by 5 sitting on mPos
void TObjPurpleIceWall::updateShape()
{
	ColliShape shape;
	colliShapeMakeBox(&shape, &mPos, mRot.y, 85.0f, 70.0f, 5.0f, 0);
	shape.mFlags |= 0x10;
	colliCopyShapes(mShape, &shape);
}

// 0x801026A8
void TObjPurpleIceWall::freeShape()
{
	colliRetire(mShape);
	mShape = 0;
}

// 0x801026E0
// 1.0f is 1
void TObjPurpleIceWall::setColliBox(void* colli, const Vec3f* center, f32 radius)
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

// 0x801027CC
void TObjPurpleIceWall::initColli()
{
	mColli = (MeshObject*)meshColliCreate();
	meshColliSetData(mColli, mColliData);
}

// 0x80102808
void TObjPurpleIceWall::updateColli()
{
	setColliBox(mColli, &mCenter, mRadius);
	meshColliSetMatrix(mColli, (const f32*)mMatrix);
}

// 0x8010284C
void TObjPurpleIceWall::freeColli()
{
	meshColliFree(mColli);
}

// 0x80102870
// 2050.0f is 2050
void TObjPurpleIceWall::draw()
{
	if (objDebugEnabled(this) != 0) {
		return;
	}
	if (!isInRange(this, &mPos, 2050.0f)) {
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

// 0x801028FC
// colliContactHasBit8 is the other shape's flag 8
void TObjPurpleIceWall::stateWait()
{
	for (ColliContact* hit = mShape->mContacts; hit != 0; hit = hit->mNext) {
		ColliShape* shape = colliContactShape(hit);
		if ((colliContactInGroup8(hit) == 0 || (shape->mFlags & 0x10) == 0) && colliContactHasBit8(hit) != 0) {
			Vec3f at;
			at.set(shape->mPos.x, shape->mPos.y, shape->mPos.z);
			sePlay3D(0x238, &at);
			meshColliRefresh(mColli);
			mAct = 1;
			return;
		}
	}
}

// 0x801029AC
// 0.5f is 0.5 and 10.0f is 10
void TObjPurpleIceWall::shatter()
{
	ShatterList burst;
	burst.mModels     = getGeObjModels();
	burst.mTexture    = getGeObjTextures()[0];
	burst.mFirst      = 2;
	burst.mModelCount = 2;
	burst.mCount      = 10;
	burst.mScale      = 1.0f;
	burst.mSpeed      = 0.5f;

	Vec3f at = mCenter;
	at.y -= mRadius;
	Vec3f size;
	size.x = mRadius;
	size.y = mRadius;
	size.z = 10.0f;
	shatterList(0, &at, mRot.y, &at, &size, &burst);
	setLinkDone((SetLink*)&mLink);
	mFlags |= 1;
}

// 0x80102A74
void TObjPurpleIceWall::vfn_10()
{
	u8 out[0x10];
	if (cameraIsVisible(2050.0f, mRadius, &mCenter, (Vec3f*)out) != 0) {
		mDrawTask->mSortKey = drawTaskDepthKey(*(f32*)(out + 8));
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x80102AEC
void TObjPurpleIceWall::render()
{
	pushCullMode(0);
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	mModel.draw();
	njPopMatrixEx();
	popCullMode();
	if (objDebugEnabled(this) != 0) {
		colliDrawShapes(mShape);
	}
}

// 0x80102B54
// The set hook run when the stage loads empty
extern "C" void initPurpleIceWallSet()
{
}

// 0x80102B58
// The set hook run when the stage ends empty
extern "C" void termPurpleIceWallSet()
{
}
