#include "enemy/TEneAm02_Rock.h"
struct DrawTask;
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "stage/SetManager.h"
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "enemy/ObjEnemy.h"
#include "core/MslMath.h"
#include "enemy/TEneAm02.h"
#include "egg/EggCreature.h"
#include "animal/AniMount.h"
#include "enemy/boss/EneBoss.h"
#include "engine/Colli.h"
#include "object/ObjShadow.h"
#include "Ninja/ninja.h"
#include "camera/TCameraCtrl.h"
#include "ui/AeArc.h"
struct CharaColli;
struct ColliShape;
struct NJS_OBJECT;

static const char* sClassName = "TEneAm02_Rock";

extern "C" {

extern const ColliShape sAm02_RockShape[] = {
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 10.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

inline void TEneAm02_Rock::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}

// The task addDrawTask returns Only the halfword at _14 is touched here
struct RockTask {
	u8 _00[0x14];
	u16 mBits; // _14
};

// 0x80040EB4
TEneAm02_Rock::TEneAm02_Rock(TObject* parent, Vec3f* pos, int rotY)
    : TObjAction(parent)
    , mFlight()
    , mShadow()
{
	_00 = (u32)sClassName;
	mSize = 0xF4;
	setObjId(this, -1);
	mTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
	mState = 0;
	mTimer = 0;
	mScale = 0.1f;
	mAngle = rotY;
	mFlight.setPos(pos);
	mFlight.setSize(10.0f);
	mFlight.mBody.mGravity.x = 0.0f;
	mFlight.mBody.mGravity.y = -0.08f;
	mFlight.mBody.mGravity.z = 0.0f;
	mFlight.mFriction = 0.2f;
	mFlight.mMaxSpeed = 3.0f;

	mShape = colliCreate(1, 1);
	colliCopyShapes((CharaColli*)mShape, (const ColliShape*)sAm02_RockShape);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 4, 255, 0, 14);
	colliDisable((CharaColli*)mShape);
	colliShapeSetBit8((CharaColli*)mShape, 0);

	shadowSetGround(&mShadow, pos, 0);
	mShadow._30 = 20.0f;
	mShadow._34 = 20.0f;
	shadowDisable(&mShadow);
}

// 0x80041014
TEneAm02_Rock::~TEneAm02_Rock()
{
	colliRetire((CharaColli*)mShape);
	mShape = 0;
	removeDrawTask(mTask);
	releaseObjId(this);
}

// 0x800410AC
// 20 when the other object's kind is 0 otherwise 0
int TEneAm02_Rock::vfn_48(TObjAction* other)
{
	if (other->getKind() == 0) {
		return 20;
	}
	return 0;
}

// 0x800410EC
// A player takes 20 and the hit flags are cleared
int TEneAm02_Rock::onHit(const TObjAction* other, int, int* result)
{
	int hit = 0;
	HitInfo* info = (HitInfo*)result;
	if (isPlayerObj(other) != 0) {
		info->mDamage = 20;
		hit = 1;
		info->mFlags.clear();
	}
	return hit;
}

// 0x80041140
// Aims at the target Steep shots leave at speed 1
static inline f32 rockSqrt(f32 x)
{
	static const f64 _half  = .5;
	static const f64 _three = 3.0;
	volatile f32 y;
	if (x > 0.0f) {
		f64 guess = __frsqrte((f64)x);
		guess     = _half * guess * (_three - guess * guess * x);
		guess     = _half * guess * (_three - guess * guess * x);
		y         = (f32)(x * (_half * guess * (_three - guess * guess * x)));
		return y;
	}
	return x;
}

static inline Vec3f rockSub(const Vec3f* a, const Vec3f* b)
{
	Vec3f r;
	gjSubVector(&r, a, b);
	return r;
}

void TEneAm02_Rock::throwAt(Vec3f* target)
{
	Vec3f dir;
	SetRot rot;
	Vec3f delta = rockSub(target, &mFlight.mBody.mPos);
	dir   = delta;
	dir.y = 0.0f;
	f32 len = njUnitVector(&dir);
	if (len < gUnitVectorMin) {
		dir = gVecZero;
	}
	f32 dist = len - 1.5f;
	f32 speed;
	if (delta.y > 13.3125f) {
		speed = 1.0f;
	} else {
		f32 h = 13.3125f;
		h -= delta.y;
		f32 arc = rockSqrt(1.0f + 8.0f * h / 0.08f);
		f32 t = 0.5f * (-1.0f + arc);
		speed = dist / (18.75f + t);
		if (speed > 20.0f) {
			speed = 20.0f;
		}
	}
	gjScaleVector(speed, &dir, &dir);
	dir.y = 1.5f;
	mFlight.setVel(&dir);

	rot.x = 0;
	rot.y = 0;
	rot.z = 0;
	rot.y = mAngle;
	mFlight.setRot(&rot);
	mState = 1;
	mTimer = 0;
	colliEnable(mShape);
}

// 0x80041304
void TEneAm02_Rock::hold(Vec3f* pos, int rotY)
{
	mFlight.setPos(pos);
	mAngle = rotY;
}

// 0x8004133C
void TEneAm02_Rock::draw()
{
	switch (mState) {
	case 0:
		mScale += 0.02f;
		if (mScale > 1.0f) {
			mScale = 1.0f;
		}
		break;
	case 1:
		mFlight.update(mShape);
		colliPlaceShapes(mShape, &mFlight.mBody.mPos, (const ColliShape*)sAm02_RockShape);
		mTimer++;
		if (mTimer >= 300) {
			mState = 2;
			colliDisable(mShape);
		}
		break;
	case 2: {
		f32 prev = mScale;
		mScale -= 0.05f;
		if (mScale <= 0.05f) {
			mFlags |= 1;
		}
		f32 size = 2.0f * (10.0f * mScale);
		mShadow._30 = size;
		mShadow._34 = size;
		mFlight.mBody.mPos.y -= 10.0f * (prev - mScale);
		break;
	}
	}
}

// 0x8004145C
void TEneAm02_Rock::vfn_10()
{
	if (cameraIsVisible(500.0f, 12.0f, &mFlight.mBody.mPos, 0) != 0) {
		((RockTask*)mTask)->mBits &= ~1;
	} else {
		((RockTask*)mTask)->mBits |= 1;
	}
}

// 0x800414C4
// Faces the holder's angle until it leaves the hand then the flight body's own rotation
void TEneAm02_Rock::render()
{
	njSetTexture(shapeTEneAm02());
	njPushMatrixEx();
	njTranslateEx(&mFlight.mBody.mPos);
	if (mState == 0) {
		gjRotateY(mAngle);
	} else {
		((SetTransform*)&mFlight)->vfn_20();
	}
	njScale(0, mScale, mScale, mScale);
	gjDrawObject((NJS_OBJECT*)modelTEneAm02());
	njPopMatrixEx();
	shadowSetAngle(&mShadow, 0);
}

// 0x80041558
Vec3f* TEneAm02_Rock::getPos()
{
	return &mFlight.mBody.mPos;
}

// 0x80041560
int TEneAm02_Rock::getKind()
{
	return 3;
}
