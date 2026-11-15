#include "stage/SetManager.h"
#include "object/TObjPilingSwitch.h"
#include "Ninja/ninja.h"
#include "engine/DrawTask.h"
#include "engine/ModelDraw.h"
#include "object/ObjArchive.h"
#include "engine/CharaColli.h"
#include "stage/EventScript.h"
#include "engine/TParticle.h"
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "sound/Sound3DPlay.h"
#include "camera/TCamPath.h"
#include "egg/EggCreature.h"
#include "core/VecMath.h"
#include "object/TObjKey00.h"
#include "object/last/TObjLastTorch.h"
struct TParticleGenerator;

enum LastTorchMode {
	TORCH_UNLIT    = 0,
	TORCH_LIGHTING = 1, // playing the motion
	TORCH_LIT      = 2,
};

typedef char TObjLastTorch_size_check[(sizeof(TObjLastTorch) == 0x188) ? 1 : -1];

const char* gTObjLastTorchName = "TObjLastTorch";
const f32 gLastTorchSetRange   = 800.0f;

// 0x8018C9B8
TObjLastTorch::TObjLastTorch(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00    = (u32)gTObjLastTorchName;
	mSize  = 0x188;
	mModel = getGeObjModels()[1];
	mDraw  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mShape = colliCreate(1, 1);
	colliSetOwner((CharaColli*)mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 4, 0xFF, 0xD6, 0xD6);
	mFlame = particleGenById(this, &gVecZero, 1, 1);
	initFromDesc(desc);
	vfn_74();
}

// 0x8018CAD0
TObjLastTorch::~TObjLastTorch()
{
	colliRetire((CharaColli*)mShape);
	mShape = 0;
	removeDrawTask(mDraw);
}

// 0x8018CB6C
// The cylinder stands on the model's first child turned with the torch
void TObjLastTorch::initFromDesc(SetDesc* desc)
{
	Vec3f base;
	Vec3f center;

	TSetActor::initFromDesc(desc);
	mtxSetTransV(mMtx, &mPos);
	mtxRotateY(mMtx, mMtx, mRot.y);
	GjModel* model = mModel->model;
	center.set(model->center.x, model->center.y, model->center.z);
	vecAdd(&mCenter, &mPos, &center);
	mRadius           = mModel->model->r;
	ColliShape* shape = colliGetShape((CharaColli*)mShape, 0);
	NJS_OBJECT* child = mModel->child;
	base.set(child->pos[0], child->pos[1], child->pos[2]);
	vecRotateY(&base, &base, mRot.y);
	vecAdd(&base, &base, &mPos);
	colliShapeMakeCylinder(shape, &base, 1, 10.0f, 15.0f);
	mSwitchId = (desc->getInt(0) < 0) ? 0 : ((desc->getInt(0) > 0x800) ? 0x800 : desc->getInt(0));
	mEventId  = desc->getInt(1);
}

// 0x8018CCEC
void TObjLastTorch::debugPrint(void*)
{
	njPrintC(0xA0013, ":SWITCH ID");
	njPrintC(0xA0014, ": CALL EVENT ID");
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, "");
	njPrintC(0xA0013, "");
	njPrintC(0xA0014, "");
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, "");
}

static void torchNodeFunc(int node, void* arg);

// 0x8018CD90
void TObjLastTorch::vfn_74()
{
	mLight.mTimer = 0;
	mTrack.set(gGeObjCommon->mSet.mMotions[0], 0, 0.4f);
	mMesh.setModel(mModel);
	mMesh.mDrawState = &mState;
	mMesh.mTexList   = getGeObjTextures()[1];
	mMesh.mTrack     = &mTrack;
	mMesh.mMatrix    = mMtx;
	mMesh.setNodeFunc((void*)torchNodeFunc, this);
	if (!keyBitIsSet(mSwitchId)) {
		if (mFlame != NULL) {
			mFlame->mFlags |= 4;
		}
		mMode = TORCH_UNLIT;
	} else {
		mTrack.mFrame = mTrack.mEnd;
		mMode         = TORCH_LIT;
	}
	mMesh.walkNodes();
}

// 0x8018CE6C
// Node 2 is the flame's
static void torchNodeFunc(int node, void* arg)
{
	TObjLastTorch* torch = (TObjLastTorch*)arg;
	if (node == 2) {
		njGetMatrix(torch->mFlameMtx);
		torch->placeFlame();
	}
}

// 0x8018CEAC
void TObjLastTorch::placeFlame()
{
	if (mFlame != NULL) {
		Vec3f pos;
		mtxMultVec(mFlameMtx, &gVecZero, &pos);
		mFlame->mPos = pos;
	}
}

// 0x8018CF10
void TObjLastTorch::draw()
{
	switch (mMode) {
	case TORCH_UNLIT:
		stateUnlit();
		break;
	case TORCH_LIGHTING:
		stateLighting();
		break;
	case TORCH_LIT:
		stateLit();
		break;
	}
}

// 0x8018CF68
void TObjLastTorch::stateUnlit()
{
	for (ColliContact* contact = ((CharaColli*)mShape)->mContacts; contact != NULL; contact = contact->mNext) {
		if (colliContactHasBit8(contact)) {
			TObjAction* other = findObjById(colliContactOwner(contact));
			if (other != NULL) {
				HitInfo hit;
				if (other->onHit(this, 0, (int*)&hit) && hit.mFlags.testBit(10)) {
					if (mFlame != NULL) {
						mFlame->mFlags &= ~4;
					}
					sePlay3D(0x28B, &mPos);
					mMode = TORCH_LIGHTING;
				}
			}
		}
	}
}

// 0x8018D044
void TObjLastTorch::stateLighting()
{
	if (mTrack.isDone()) {
		setKeyBit(mSwitchId);
		eventScriptRequest(mEventId);
		mMode = TORCH_LIT;
	}
	mTrack.update();
	mMesh.walkNodes();
}

// 0x8018D0A4
void TObjLastTorch::stateLit()
{
}

// 0x8018D0A8
void TObjLastTorch::vfn_10()
{
	if (mLight.isVisible(mDraw, 700.0f, &mCenter, mRadius, 0)) {
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
	setDrawAmbient(&mLight, &mState);
}

// 0x8018D124
void TObjLastTorch::render()
{
	mMesh.draw();
}
