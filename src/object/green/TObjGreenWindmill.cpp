#include "stage/SetActor.h"
struct DrawTask;
#include "object/ObjAction.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "engine/ModelDraw.h"
#include "sound/SoundHandle.h"
#include "object/ObjArchive.h"
#include "camera/TCamera.h"
#include "object/TObjLauncher.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "core/MathUtil.h"
#include "object/TObjEggRing.h"
#include "Ninja/ninja.h"
#include "camera/TCameraCtrl.h"
#include "engine/Colli.h"
#include "engine/GroundQuery.h"
#include "engine/MeshPool.h"
#include "engine/WorldMesh.h"
#include "core/VecMath.h"
#include "object/TObjKey00.h"
#include "object/green/TObjGreenWindmill.h"
struct CharaColli;
struct ColliContact;
struct MeshColli;
struct MeshData;
struct MeshObject;

/** A collision contact on the list at +0x80 of the collision */
struct WindmillContact {
	WindmillContact* mNext; // _00
};

f32 gGreenWindmillSetRange = 3000.0f; // 0x802C45C8

static const f32 sDrawDist = gGreenWindmillSetRange;         // 0x802C5938
static f32 sKeepDist       = gGreenWindmillSetRange + 50.0f; // 0x802C593C

typedef char TObjGreenWindmill_size_check[(sizeof(TObjGreenWindmill) == 0x154) ? 1 : -1];

// 0x80185940
extern "C" TObject* createTObjGreenWindmill(TObject* parent, SetDesc* desc)
{
	return new TObjGreenWindmill(parent, desc);
}

const char* gTObjGreenWindmillName = "TObjGreenWindmill";

// 0x80185994
TObjGreenWindmill::TObjGreenWindmill(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00       = (u32)gTObjGreenWindmillName;
	mSize     = 0x154;
	mModel    = (WindmillNode*)getGeObjModels()[0];
	mCollData = gGeObjCommon->mSet.mMeshes[0];
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mShape    = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 4, 0xFF, 0xFF, 0xF7);
	mColli = (MeshColli*)meshColliCreate();
	meshColliSetData((MeshObject*)mColli, (MeshData*)mCollData);
	meshSetFlag2((MeshObject*)mColli, 1);
	meshSetState100((MeshObject*)mColli, 1);
	initFromDesc(desc);
	vfn_74();
}

// 0x80185AC8
TObjGreenWindmill::~TObjGreenWindmill()
{
	seStop3D(&mSound);
	meshColliFree(mColli);
	colliRetire(mShape);
	mShape = 0;
	removeDrawTask(mDrawTask);
}

// 0x80185B68
void TObjGreenWindmill::initFromDesc(SetDesc* desc)
{
	Vec3f at;
	Vec3f high;
	Vec3f low;
	Vec3f centre;
	f32 box[6];

	TSetActor::initFromDesc(desc);

	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
	mtxCopy(mFanMatrix, mMatrix);

	centre.set(mModel->mMesh->mCentre.x, mModel->mMesh->mCentre.y, mModel->mMesh->mCentre.z);
	vecAdd(&mCentre, &mPos, &centre);
	mRadius = mModel->mMesh->mRadius;

	ColliShape* shape = colliGetShape(mShape, 0);
	at.set(mModel->mChild->mPos.x, mModel->mChild->mPos.y, mModel->mChild->mPos.z);
	vecRotateY(&at, &at, mRot.y);
	vecAdd(&at, &at, &mPos);
	colliShapeMakeCylinder(shape, &at, 1, 20.0f, 7.5f);
	meshColliSetMatrix(mColli, (const f32*)mFanMatrix);

	f32 reach = 10.0f + mRadius;
	high.x    = reach;
	high.y    = reach;
	high.z    = reach;
	low.x     = -reach;
	low.y     = -reach;
	low.z     = -reach;
	vecAdd(&high, &high, &mCentre);
	vecAdd(&low, &low, &mCentre);
	boxAroundLine(box, &high, &low);
	meshColliSetBox((MeshObject*)mColli, (const f32*)box);

	mKey = CLAMP_RANGE(desc->getInt(0), 0, 0x800);
}

// 0x80185D48
void TObjGreenWindmill::debugPrint(void*)
{
	njPrintC(0xA0013, ":SWITCH ID");
	njPrintC(0xA0014, "");
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, "");
	njPrintC(0xA0013, "");
	njPrintC(0xA0014, "");
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, "");
}

// 0x80185DE8
// Called for each node as the model is walked
static void readFanMatrix(int node, void* arg)
{
	if (node == 0) {
		njGetMatrix(((TObjGreenWindmill*)arg)->mFanMatrix);
	}
}

// 0x80185E14
void TObjGreenWindmill::vfn_74()
{
	if (keyBitIsSet(mKey) != 0) {
		mSpeed = 0.3f;
		colliDisable(mShape);
		meshColliRefresh((MeshObject*)mColli);
		sePlay3DHeld(800.0f, &mSound, 0x271, 0x64, &mPos);
		mAct = 2;
	} else {
		mSpeed = 0.0f;
		mAct   = 0;
	}
	mTrack.set(gGeObjCommon->mSet.mMotions[0], 1, 1.0f);
	mTrack.mRate = mSpeed;
	mDraw.setModel(mModel);
	mDraw.mDrawState = getDefaultDrawState();
	mDraw.mTexList   = getGeObjTextures()[0];
	mDraw.mTrack     = &mTrack;
	mDraw.mMatrix    = mMatrix;
	mDraw.setNodeFunc((void*)readFanMatrix, this);
	mDraw.walkNodes();
}

// 0x80185F08
void TObjGreenWindmill::draw()
{
	switch (mAct) {
	case 0:
		updateIdle();
		break;
	case 1:
		updateSpinUp();
		break;
	case 2:
		updateSpin();
		break;
	}
}

// 0x80185F60
// Waits for a hit carrying flag 0xF from a shape with flag 0x400
void TObjGreenWindmill::updateIdle()
{
	WindmillContact* contact = (WindmillContact*)mShape->mContacts;
	while (contact != 0) {
		if ((((ColliShape*)colliContactShape((ColliContact*)contact))->mFlags & 0x400) != 0) {
			TObjAction* other = findObjById(colliContactOwner((ColliContact*)contact));
			if (other != 0) {
				HitInfo hit;
				if (other->onHit(this, 0, (int*)&hit) != 0 && hit.mFlags.testBit(0xF) != 0) {
					sePlay3DHeld(800.0f, &mSound, 0x271, 0x64, &mPos);
					mAct = 1;
				}
			}
		}
		contact = contact->mNext;
	}
}

// 0x80186034
void TObjGreenWindmill::updateSpinUp()
{
	mSpeed += 0.0015f;
	if (mSpeed > 0.3f) {
		colliDisable(mShape);
		meshColliRefresh((MeshObject*)mColli);
		mSpeed = 0.3f;
		setKeyBit(mKey);
		mAct = 2;
	}
	mTrack.mRate = mSpeed;
	mTrack.update();
	mDraw.walkNodes();
	meshColliSetMatrix(mColli, mFanMatrix);
}

// 0x801860C8
void TObjGreenWindmill::updateSpin()
{
	mTrack.update();
}

// 0x801860EC
void TObjGreenWindmill::vfn_10()
{
	if (cameraIsVisible(sDrawDist, mRadius, &mCentre, 0) != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x80186154
void TObjGreenWindmill::render()
{
	mDraw.draw();
}
