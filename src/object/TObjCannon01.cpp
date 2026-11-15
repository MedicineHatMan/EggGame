#include "stage/SetManager.h"
#include "stage/SetActor.h"
#include "object/ObjScript.h"
#include "object/ObjAction.h"
#include "engine/DrawTask.h"
#include "engine/CharaColli.h"
#include "object/ObjArchive.h"
#include "enemy/EnemyLight.h"
#include "stage/StageObjUtil.h"
#include "egg/ObjEgg.h"
#include "engine/TParticle.h"
#include "object/TObjBlast.h"
#include "Ninja/ninja.h"
#include "sdk/stl/math.h"
#include "camera/TCamera.h"
#include "egg/EggCreature.h"
#include "engine/Colli.h"
#include "sound/Sound3DPlay.h"
#include "ui/AeArc.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "core/MathUtil.h"
#include "engine/FxSystem.h"
#include "engine/GroundQuery.h"
#include "core/VecMath.h"
#include "object/TObject.h"
#include "object/TObjCannon01.h"
#include "animal/RacerMover.h"
struct TParticleGenerator;
struct _GXColor;
struct Fx;

static const GXColor sDebugScopeColor = { 0x20, 0xFF, 0x20, 0x80 };
static const GXColor sDebugLineColor  = { 0x20, 0x20, 0x20, 0xFF };

extern "C" {
const f32 gCannon01SetRange = 800.0f;

// 0x802C3C28
ObjClassName gTObjCannon01Name = { "TObjCannon01", 0 };
}

#define ANGLE_PER_RADIAN 10430.38043493439 // 0x10000 / 2 pi

typedef char TObjCannon01_size_check[(sizeof(TObjCannon01) == 0x120) ? 1 : -1];

// 0x80029768
// The scope's contact filter hands back the owner of the first contact and stops
extern "C" int cannonShotFilter(ColliContact* contact, u16* owner)
{
	if (owner == 0) {
		return 0;
	}
	*owner = colliContactOwner(contact);
	return 0;
}

// 0x800297A8
extern "C" TObject* createTObjCannon01(TObject* parent, SetDesc* desc)
{
	return new TObjCannon01(parent, desc);
}

// 0x800297FC
static inline Vec3f cannonScale(f32 s, const Vec3f* v)
{
	Vec3f r;
	gjScaleVector(s, &r, v);
	return r;
}

static inline Vec3f cannonZero() { return gVecZero; }

// x y z turned about Y
static inline void cannonTurnY(Vec3f* out, f32 x, f32 y, f32 z, int ang)
{
	out->x = z * njSin(ang) + x * njCos(ang);
	out->y = y;
	out->z = z * njCos(ang) - x * njSin(ang);
}

void TObjCannon01::initFromDesc(SetDesc* desc)
{
	f32 up;
	f32 flat;
	int rot;
	Vec3f min;
	Vec3f max;

	TSetActor::initFromDesc(desc);
	mScale = 0.5f + desc->getFloat(0);
	mPower = desc->getFloat(2);
	rot    = mRot.y;
	up     = desc->getFloat(3);
	flat   = 1.0f - up;
	cannonTurnY(&mDir, 0.0f, up, flat, rot);
	mPitch = (int)(ANGLE_PER_RADIAN * (f32)atan2(up, flat));
	mShotVel = cannonScale(mPower, &mDir);
	aimMouth();
	mScope = landingPoint();
	colliSetSphere((CharaColli*)mShape, &mPos, 0, &cannonZero(), 50.0f * mScale);
	f32 radius = 50.0f + desc->getFloat(1);
	colliSetCylinder(mScopeColli, &mScope, 0x10, &cannonZero(), radius, 25.0f);
	colliRefreshBounds(mScopeColli);
	modelSphere(10.0f, mModels[4], &min, &max, &mFoot, &mHeight);
	vecAdd(&mFoot, &mFoot, &mPos);
}

// 0x80029A6C
// What the set values mean the aim and the scope
// 0x802C6A28
// and 0x802C6A2C the scope cylinder and the aim line

static inline Vec3f cannonScaleAdd(const Vec3f* a, f32 s, const Vec3f* dir)
{
	Vec3f r;
	Vec3f b = cannonScale(s, dir);
	gjAddVector(&r, a, &b);
	return r;
}

void TObjCannon01::debugPrint(void*)
{
	njPrintC(0xA0017, "+0.5f:SCALE");
	njPrintC(0xA0018, "+50:SCOPE_SCALE");
	njPrintC(0xA0019, ":POWER(0~10?)");
	njPrintC(0xA001A, ":VECTOR(0.0~1.0)");
	Vec3f sum = cannonScaleAdd(&mMouth, 50.0f, &mDir);
	GXColor color;
	color.r = sDebugLineColor.r;
	color.g = sDebugLineColor.g;
	color.b = sDebugLineColor.b;
	color.a = sDebugLineColor.a;
	debugLine(&mMouth, &sum, &color);
	color.r = sDebugScopeColor.r;
	color.g = sDebugScopeColor.g;
	color.b = sDebugScopeColor.b;
	color.a = sDebugScopeColor.a;
	debugCylinder(&mScope, &color, colliGetShapes(mScopeColli)->mSizeX, colliGetShapes(mScopeColli)->mSizeY);
}

// 0x80029BAC
TObjCannon01::TObjCannon01(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	ObjCommonData* pack;

	_00         = (u32)gTObjCannon01Name.mName;
	mSize       = 0x120;
	mModels     = getGeObjModels();
	mTextures   = getGeObjTextures();
	pack        = gGeObjCommon;
	mOpenMotion = (NJS_MOTION*)pack->mSet.mMotions[1];
	mShape      = colliCreate(1, 1);
	colliSetOwner((CharaColli*)mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 4, 255, 0xF6, 0xF6);
	mScopeColli = colliCreate(1, 1);
	colliSetOwner(mScopeColli, (u16)mObjId);
	colliSetMasks(mScopeColli, 4, 255, 255, 0xFE);
	initFromDesc(desc);
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	reset();
}

// 0x80029CDC
TObjCannon01::~TObjCannon01()
{
	colliRetire((CharaColli*)mShape);
	mShape = 0;
	colliRetire(mScopeColli);
	mScopeColli = 0;
	removeDrawTask(mDraw);
}

// 0x80029D7C
void TObjCannon01::reset()
{
	mTarget.setId(0xFFFF);
	mAct          = ACT_WAIT;
	mPhase        = PHASE_START;
	mOpen         = 0.0f;
	mOpenStep     = 1.0f;
	mLight.mTimer = 0;
}

// 0x80029DD4
// The mouth is 75 along the shot scaled from the barrel's pivot 22.5 up
void TObjCannon01::aimMouth()
{
	Vec3f dir;

	dir = mShotVel;
	if (njUnitVector(&dir) < gUnitVectorMin) {
		dir = gVecZero;
	}
	gjScaleVector(75.0f, &dir, &dir);
	gjScaleVector(mScale, &dir, &dir);
	njUnitMatrix(0);
	gjTranslate(0.0f, 22.5f * mScale, 0.0f);
	njTranslateEx(&dir);
	njCalcPointEx(&mPos, &mMouth);
}

// 0x80029E98
// Flies a shot from the mouth until it meets the ground
Vec3f TObjCannon01::landingPoint()
{
	SetTransform shot;
	EggHits hits;
	int flying = 1;

	shot.mGravity.x = 0.0f;
	shot.mGravity.y = -CANNON_GRAVITY;
	shot.mGravity.z = 0.0f;
	shot.mPos = mMouth;
	shot.mVel = mShotVel;
	shot.vfn_0C();
	while (flying) {
		shot.vfn_0C();
		shot.vfn_14();
		Vec3f at = shot.mPos;
		if (meshColliProbe(hits.mHits, &at, -1) != 0) {
			f32 rise = hits.mHits[0].mPos.y - shot.mPos.y;

			if (!(rise < shot.mVel.y)) {
				shot.mVel.y = rise;
				flying = 0;
			}
		} else if (shot.mPos.y < -9999.0f) {
			Vec3f origin;

			origin.x = 0.0f;
			origin.y = 0.0f;
			origin.z = 0.0f;
			return origin;
		}
		shot.vfn_18();
	}
	return shot.mPos;
}

static inline Vec3f vecDiff(const Vec3f* a, const Vec3f* b)
{
	Vec3f d;
	gjSubVector(&d, a, b);
	return d;
}

// 0x8002A034
// Keeps the shot's upward speed and sets the across speed so it comes down at target's spot at the scope's height the
// time up to the top of the arc then the fall from there
void TObjCannon01::aimAt(const Vec3f* target)
{
	f32 up = mShotVel.y / CANNON_GRAVITY;
	f32 peak = mShotVel.y * up - CANNON_GRAVITY * (0.5f * (up * up));
	f32 down = dolsqrtf(2.0f * (peak + (mMouth.y - mScope.y)) / CANNON_GRAVITY);
	Vec3f flat = vecDiff(target, &mMouth);
	flat.y     = 0.0f;
	gjScaleVector(1.0f / (up + down), &flat, &flat);
	mShotVel.x = flat.x;
	mShotVel.z = flat.z;
}

// 0x8002A164
// Whether something is in the scope
int TObjCannon01::caught()
{
	u16 id;

	if (colliFindContact(mScopeColli, 1, (ContactFilter)cannonShotFilter, &id) != 0) {
		mTarget.setId(id);
		return 1;
	}
	return 0;
}

// 0x8002A1C4
void TObjCannon01::advanceOpen()
{
	if (mOpen < (f32)mOpenMotion->nbFrame) {
		mOpen = mOpen + mOpenStep;
	} else {
		mOpen = (f32)mOpenMotion->nbFrame;
	}
}

// 0x8002A220
void TObjCannon01::stateWait()
{
	if (caught() != 0) {
		mAct = ACT_FIRE;
	}
}

// 0x8002A25C
// Turns to the target 0x40 a frame then raises the barrel the same way
void TObjCannon01::stateFire()
{
	switch (mPhase) {
	case PHASE_START:
		mPhase = PHASE_AIM;
		break;
	case PHASE_AIM: {
		TObjAction* target;

		if (caught() == 0) {
			mPhase = PHASE_START;
			mAct   = ACT_WAIT;
			break;
		}
		target = mTarget.get();
		if (target != 0) {
			Vec3f at = *target->getPos();
			Vec3f flat = vecDiff(&at, &mPos);
			int left;
			f32 len;

			mRot.y = angleApproachDiff(mRot.y, (int)(ANGLE_PER_RADIAN * (f32)atan2(flat.x, flat.z)), 0x40, &left);
			if (left < 0x40) {
				aimAt(&at);
				aimMouth();
				Vec3f dir = mShotVel;
				int pitchLeft;
				dir.y = 0.0f;
				len   = njUnitVector(&dir);
				if (len < gUnitVectorMin) {
					dir = gVecZero;
				}
				mPitch = angleApproachDiff(mPitch, (int)(ANGLE_PER_RADIAN * (f32)atan2(mShotVel.y, len)), 0x40, &pitchLeft);
				if (pitchLeft < 0x40) {
					mPhase = PHASE_OPEN;
				}
			}
		}
		break;
	}
	case PHASE_OPEN:
		advanceOpen();
		if (mOpen > 20.0f) {
			TParticleGenerator* smoke;

			sePlay3D(0x21A, &mMouth);
			new TCannonBall(this, &mMouth, &mShotVel, &mScope, mScale);
			smoke = particleOnceByIdDefault(&mMouth, 1, 0);
			smoke->setRot(0x4000 - mPitch, mRot.y);
			mAct   = ACT_RECOVER;
			mPhase = PHASE_START;
		}
		break;
	}
}

// 0x8002A4A8
// Ready again once the ball is gone
void TObjCannon01::stateRecover()
{
	if (objCountChildren(this) == 0) {
		mAct      = ACT_WAIT;
		mOpen     = 0.0f;
		mOpenStep = 1.0f;
	}
}

// 0x8002A4F4
// Stopped while debugging gone beyond 850
void TObjCannon01::draw()
{
	if (objDebugEnabled(this) != 0) {
		return;
	}
	if (!isInRange(this, &mPos, 850.0f)) {
		mFlags |= 1;
		return;
	}
	switch (mAct) {
	case ACT_WAIT:
		stateWait();
		break;
	case ACT_FIRE:
		stateFire();
		break;
	case ACT_RECOVER:
		stateRecover();
		break;
	}
}

// 0x8002A598
void TObjCannon01::vfn_10()
{
	if (mLight.isVisible(mDraw, 700.0f, &mFoot, mHeight, 0) != 0) {
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x8002A608
// The base turned to face then the barrel on its pivot opening while it fires
void TObjCannon01::render()
{
	njPushMatrixEx();
	njTranslateEx(&mPos);
	gjRotateY(mRot.y);
	gjRotateY(0xC000);
	njScale(NULL, mScale, mScale, mScale);
	njSetTexture(mTextures[4]);
	mLight.drawObject(mModels[4]);
	gjTranslate(0.0f, 22.5f, 0.0f);
	gjRotateZ(mPitch);
	njSetTexture(mTextures[3]);
	switch (mAct) {
	case ACT_WAIT:
		mLight.drawObject(mModels[3]);
		break;
	case ACT_FIRE:
	case ACT_RECOVER:
		mLight.drawMotion(mModels[3], mOpenMotion, mOpen);
		break;
	}
	njPopMatrixEx();
}
