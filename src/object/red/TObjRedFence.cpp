#include "object/red/TObjRedFence.h"
#include "engine/DrawTask.h"
#include "object/ObjArchive.h"
#include "egg/EggCreature.h"
#include "object/TObjLauncher.h"
#include "engine/TParticle.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
struct NJS_QUATERNION;

typedef char TObjRedFence_size_check[(sizeof(TObjRedFence) == 0x114) ? 1 : -1];

const char* gTObjRedFenceName = "TObjRedFence";

// 0x800CEFC4
TObjRedFence::TObjRedFence(TObject* parent)
    : TObjAction(parent)
{
	_00       = (u32)gTObjRedFenceName;
	mSize     = 0x114;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	setup();
}

// 0x800CF04C
TObjRedFence::~TObjRedFence()
{
	stopFx();
	removeDrawTask(mDrawTask);
}

// 0x800CF0D0
void TObjRedFence::setup()
{
	mLight.reset();
	mPlace.mFlags = 3;
	mPlace.mPos   = mPos;
	njXYZAngleToQuaternion((const s32*)&mRot, (NJS_QUATERNION*)mPlace.mRot);
	mPlace.update();
	mModel.setModel(getGeObjModels()[0x13]);
	mModel.mPlace   = &mPlace;
	mModel.mTexList = getGeObjTextures()[0x0A];
	mModel.mState   = (DrawState*)getDefaultDrawState();
	for (u16 i = 0; i < 3; i++) {
		mFx[i] = 0;
	}
}

// 0x800CF18C
void TObjRedFence::draw()
{
	mPlace.mPos = mPos;
	njXYZAngleToQuaternion((const s32*)&mRot, (NJS_QUATERNION*)mPlace.mRot);
	mPlace.update();
}

// 0x800CF1E0
void TObjRedFence::vfn_10()
{
	int show = mLight.isVisible(mDrawTask, 600.0f, getPos(), 100.0f, 0);
	if (show != 0) {
		mDrawTask->mFlags &= 0xFFFE;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x800CF260
void TObjRedFence::render()
{
	ObjDrawState state;
	mModel.mState = (DrawState*)mLight.drawState(&state);
	mModel.draw();
}

// 0x800CF2AC
void TObjRedFence::startFx()
{
	for (u16 i = 0; i < 3; i++) {
		Vec3f at;
		pointOn(&at, i);
		mFx[i] = (TObject*)particleGenById(this, &at, 1, 0xC);
	}
}

// 0x800CF31C
void TObjRedFence::stopFx()
{
	for (u16 i = 0; i < 3; i++) {
		if (mFx[i] != 0) {
			mFx[i]->mFlags |= 1;
			mFx[i] = 0;
		}
	}
}

const Vec3f sRedFenceTop[3]    = { { -78.0f, 75.0f, -5.0f }, { 0.0f, 75.0f, -5.0f }, { 78.0f, 75.0f, -5.0f } };
const Vec3f sRedFenceBottom[3] = { { -78.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 78.0f, 0.0f, 0.0f } };

// 0x800CF360
void TObjRedFence::pointOn(Vec3f* out, u16 i)
{
	if (i >= 3) {
		return;
	}
	Vec3f p;
	p = sRedFenceTop[i];
	mtxMultVec(mPlace.mMatrix, &p, out);
}

// 0x800CF3C0
void TObjRedFence::pointOff(Vec3f* out, u16 i)
{
	if (i >= 3) {
		return;
	}
	Vec3f p;
	p = sRedFenceBottom[i];
	mtxMultVec(mPlace.mMatrix, &p, out);
}
