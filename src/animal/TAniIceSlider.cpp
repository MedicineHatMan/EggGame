#include "animal/AniAttacks.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "enemy/boss/EneBoss.h"
#include "engine/DrawState.h"
#include "core/MathUtil.h"
#include "engine/Colli.h"
#include "camera/TCameraCtrl.h"
#include "Ninja/ninja.h"
#include "ui/AeArc.h"
#include "engine/GroundQuery.h"
#include "stl/math.h"
#include "animal/TAniIceSlider.h"
#include "core/TMainTask.h"
#include "animal/AniModel.h"
struct ColliShape;
struct CharaColli;

extern "C" {
extern const ColliShape sIceSliderShape[] = {
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 5.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

const char* gTAniIceSliderName = "TAniIceSlider";

// 0x80136940
TAniIceSlider::TAniIceSlider(TEggCreatureTypeE* owner, const Vec3f* pos, const Vec3f* vel, u16 ownerId, u16 targetId)
    : TObject(owner)
{
	_00        = (u32)gTAniIceSliderName;
	mSize      = 0x70;
	mDrawTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	mObject    = owner->mMotion->mObject[1];
	mTexList   = owner->mTexList;
	mState     = 0;
	mPos       = *pos;
	mVel       = *vel;
	mRot[0]    = 0;
	mRot[1]    = 0;
	mRot[2]    = 0;
	mRot[1]    = vecAngleXZ(&mVel);
	mTargetId  = targetId;
	mOwnerId   = ownerId;
	mDone      = 0;
	mTicks     = 0;
	mRange     = 0.0f;
	mTravelled = 0.0f;
	mGround    = 0;

	AniParams* params = (AniParams*)gAnimalParams[2];
	if (params == 0) {
		mFlags |= 1;
		return;
	}
	f32* values = params->mValues;
	u8 shape[0x30];
	if ((int)values[4] == 0) {
		colliShapeMakeSphere((ColliShape*)shape, &gVecZero, 0, values[5]);
	} else {
		colliShapeMakeCylinder((ColliShape*)shape, &gVecZero, 1, values[5], values[6]);
	}
	mColli = (CharaColli*)colliCreate(1, 1);
	if (mColli != 0) {
		colliSetOwner(mColli, (u16)ownerId);
		colliSetMasks(mColli, 0x20, 0xFF, 0xFF, 0x10);
		colliCopyShapes(mColli, (const ColliShape*)shape);
		colliPlaceShapes(mColli, &mPos, (const ColliShape*)shape);
	}
}

// 0x80136B1C
TAniIceSlider::~TAniIceSlider()
{
	colliRetire(mColli);
	removeDrawTask(mDrawTask);
}

// 0x80136B8C
void TAniIceSlider::draw()
{
	if (mDone == 0) {
		switch (mState) {
		case 0:
			break;
		case 1:
			run();
			break;
		}
	}
}

// 0x80136BCC
void TAniIceSlider::vfn_10()
{
	Vec3f view;
	if (cameraIsVisible(1000.0f, 50.0f, &mPos, &view) != 0) {
		mDrawTask->mSortKey = drawTaskDepthKey(view.z);
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x80136C44
void TAniIceSlider::render()
{
	njPushMatrixEx();
	njTranslateEx(&mPos);
	gjRotateY(mRot[1]);
	njSetTexture(mTexList);
	drawObjectDefault(mObject);
	njPopMatrixEx();
}

/** a plus b returned by value */
static inline Vec3f iceAdd(const Vec3f* a, const Vec3f* b)
{
	Vec3f v;
	gjAddVector(&v, a, b);
	return v;
}

// 0x80136C94
void TAniIceSlider::run()
{
	EggHits hits;
	mVel.y += -9.8f / 60.0f;
	Vec3f flat = mVel;
	flat.y     = 0.0f;
	f32 length = njUnitVector(&flat);
	if (length < gUnitVectorMin) {
		flat = gVecZero;
	}

	AniParams* params = (AniParams*)gAnimalParams[2];
	if (params == 0) {
		return;
	}
	f32* values = params->mValues;
	f32 speed;
	if (length < values[7]) {
		speed = length + values[8];
	} else {
		speed = values[7];
	}
	gjScaleVector(speed, &flat, &flat);
	mVel.x = flat.x;
	mVel.z = flat.z;

	Vec3f from = mPos;
	from.y += 10.0f;
	Vec3f ahead = iceAdd(&flat, &from);
	if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&ahead, (unsigned long)0xD) != 0) {
		if (mPos.y - hits.mHits[0].mPos.y >= 20.0f) {
			mGround |= 2;
			mVel.y = 0.0f;
		}
	} else {
		mGround |= 2;
		mVel.y = 0.0f;
	}

	if ((mGround & 2) == 0) {
		if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&from, (unsigned long)1) != 0) {
			f32 ground = hits.mHits[0].mPos.y;
			if (ground - mPos.y < mVel.y) {
				mGround &= ~1;
				mTicks = 0;
			} else {
				mVel.y = 0.0f;
				mPos.y = ground;
				mGround |= 1;
			}
		} else {
			mGround &= ~1;
			mTicks = 0;
		}
	}

	mTicks++;
	if (mTicks % 8 == 0) {
		new TAniIceRock(gTL_06, &mPos, mOwnerId);
	}

	Vec3f to = iceAdd(&from, &mVel);
	if (meshColliProbeLine((EggHit*)&hits, (const Vec3f*)&from, (const Vec3f*)&to, (unsigned long)((-1))) != 0 || touchedTarget() != 0
	    || mTravelled >= mRange) {
		mVel.x = 0.0f;
		mVel.y = 0.0f;
		mVel.z = 0.0f;
		mDone  = 1;
	} else {
		mRot[1] = (int)(10430.38043493439 * (f32)atan2(mVel.x, mVel.z));
		njAddVector(&mPos, &mVel);
		mTravelled += speed;
		colliPlaceShapes(mColli, &mPos, sIceSliderShape);
	}
}

// 0x80136FF4
int TAniIceSlider::touchedTarget()
{
	ColliContact* contact = mColli->mContacts;
	while (contact != 0) {
		if (colliContactNoBit10(contact) != 0) {
			int owner = (u16)colliContactOwner(contact);
			return owner == mTargetId;
		}
		contact = contact->mNext;
	}
	return 0;
}
