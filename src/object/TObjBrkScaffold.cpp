#include "stage/SetManager.h"
#include "stage/SetActor.h"
#include "engine/ModelDraw.h"
#include "object/ObjModel.h"
#include "object/ObjArchive.h"
#include "engine/DrawTask.h"
#include "Ninja/ninja.h"
#include "engine/CharaColli.h"
#include "stage/StageObjUtil.h"
#include "egg/EggCreature.h"
#include "enemy/boss/EneBoss.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "camera/TCameraCtrl.h"
#include "engine/GroundQuery.h"
#include "engine/MeshPool.h"
#include "engine/WorldMesh.h"
#include "stage/SetSlot.h"
#include "core/VecMath.h"
#include "object/TObjBrkScaffold.h"
struct MeshData;
struct SetLink;
struct MeshObject;
struct ColliShape;

const f32 gBrkScaffoldSetRange = 800.0f;

// 0x801AAB34
extern "C" TObject* createTObjBrkScaffold(TObject* parent, SetDesc* desc)
{
	return new TObjBrkScaffold(parent, desc);
}

const char* gTObjBrkScaffoldName = "TObjBrkScaffold";

// 0x801AAB88
TObjBrkScaffold::TObjBrkScaffold(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	mBlink        = 0;
	mAct          = 0;
	_00           = (u32)gTObjBrkScaffoldName;
	mSize         = 0x104;
	mObject       = getStgObjModels()[53];
	mTexList      = getStgObjTextures()[47];
	mRadius       = mObject->model->r + 10.0f;
	mMesh         = gStgObjCommon->mSet.mMeshes[6];
	mDrawTask     = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
	mShape        = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 255, 255, 247);
	mMeshColli = (MeshObject*)meshColliCreate();
	meshColliSetData((MeshObject*)mMeshColli, (MeshData*)mMesh);
	mDraw.setModel(mObject);
	mDraw.mDrawState = &mState;
	mDraw.mTexList   = mTexList;
	mDraw.mMatrix    = mMatrix;
	mState.mTev2     = 0x808080FF;
	initFromDesc(desc);
}

// 0x801AACE0
TObjBrkScaffold::~TObjBrkScaffold()
{
	meshColliFree(mMeshColli);
	colliRetire(mShape);
	mShape = 0;
	removeDrawTask(mDrawTask);
}

// 0x801AAD6C
// The box is 40 by 2 by 40 on top
void TObjBrkScaffold::initFromDesc(SetDesc* desc)
{
	f32 box[6];

	TSetActor::initFromDesc(desc);
	mPos.y -= 5.0f;
	mHits = desc->getInt(0) < 1 ? 1 : desc->getInt(0);
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
	Vec3f top = mPos;
	Vec3f radius;
	Vec3f low;
	Vec3f high;
	Vec3f center;
	top.y += 5.0f;
	colliShapeMakeBox(colliGetShapes(mShape), &top, mRot.y, 40.0f, 2.0f, 40.0f, 1);
	center.set(mObject->model->center.x, mObject->model->center.y, mObject->model->center.z);
	vecAdd(&mCenter, &mPos, &center);
	meshColliSetMatrix(mMeshColli, (const f32*)mMatrix);
	radius.x = mRadius;
	radius.y = mRadius;
	radius.z = mRadius;
	vecScaleAdd(-1.0f, &low, &radius, &mCenter);
	vecAdd(&high, &radius, &mCenter);
	boxAroundLine(box, &low, &high);
	meshColliSetBox(mMeshColli, (const f32*)box);
}

// 0x801AAEE4
// A title row and seven blank ones
void TObjBrkScaffold::debugPrint(void*)
{
	njPrintC(0xA0013, "+1:HP");
	njPrintC(0xA0014, "");
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, "");
	njPrintC(0xA0017, "");
	njPrintC(0xA0018, "");
	njPrintC(0xA0019, "");
	njPrintC(0xA001A, "");
}

// 0x801AAF80
void TObjBrkScaffold::draw()
{
	switch (mAct) {
	case 0:
		stateWait();
		break;
	case 1:
		stateHit();
		break;
	case 2:
		shatter();
		break;
	}
}

// 0x801AAFD8
void TObjBrkScaffold::stateWait()
{
	ColliContact* contact;

	for (contact = ((CharaColli*)mShape)->mContacts; contact != 0; contact = contact->mNext) {
		if (colliContactHasBit8(contact) == 0) {
			continue;
		}
		TObjAction* other = findObjById(colliContactOwner(contact));
		if (other == 0) {
			continue;
		}
		HitInfo hit;
		if (other->onHit(this, 0, (int*)&hit) == 0 || hit.mFlags.testBit(0xF) == 0) {
			continue;
		}
		mHits = mHits - 1 < 0 ? 0 : mHits - 1;
		if (mHits != 0) {
			mState.mFlags = 0x20;
			mBlink        = 60;
			mAct          = 1;
		} else {
			meshColliRefresh(mMeshColli);
			mAct = 2;
		}
		return;
	}
}

// 0x801AB0D8
void TObjBrkScaffold::stateHit()
{
	if (mBlink == 0) {
		mState.mFlags = 0;
		mAct          = 0;
		return;
	}
	mBlink--;
}

// 0x801AB100
// Ten pieces of the four models from 54 from the bottom of the matrix
void TObjBrkScaffold::shatter()
{
	ShatterList pieces;
	Vec3f at;
	Vec3f size;

	pieces.mModels     = getStgObjModels() + 54;
	pieces.mTexture    = mTexList;
	pieces.mFirst      = 0;
	pieces.mModelCount = 4;
	pieces.mCount      = 10;
	pieces.mScale      = 1.0f;
	pieces.mSpeed      = 1.0f;
	size.x             = 40.0f;
	size.y             = 5.0f;
	size.z             = 40.0f;
	mtxGetPos(mMatrix, &at);
	shatterList(0, &at, mRot.y, &at, &size, &pieces);
	setLinkDone((SetLink*)&mLink);
	mFlags |= 1;
}

// 0x801AB1B0
// Skip the draw task while the camera cannot see the sphere
void TObjBrkScaffold::vfn_10()
{
	if (cameraIsVisible(700.0f, mRadius, &mCenter, 0)) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x801AB218
void TObjBrkScaffold::render()
{
	mDraw.draw();
	if (objDebugEnabled(this) != 0) {
		meshDebugDraw((MeshColli*)mMeshColli, (const MeshData*)mMesh);
		colliDrawShapes(mShape);
	}
}
