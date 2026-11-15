#include "engine/RockFlight.h"
#include "stage/Stage.h"
#include "engine/CharaColli.h"
#include "engine/MeshColli.h"
#include "egg/EggCreature.h"
#include "enemy/boss/EneBoss.h"
#include "Ninja/ninja.h"
#include "core/VecMath.h"
#include "engine/GroundQuery.h"
#include "enemy/EnemyFx.h"
#include "animal/RacerMover.h"
#include "enemy/Enemy.h"

struct MeshList;

/** What the mesh walk fills in. @size 0x44 */
struct RockHit {
	int mHit;      // _00
	f32 mRadius;   // _04
	Vec3f mPos;    // _08 pushed out of each triangle
	Vec3f mVel;    // _14 bounced off each triangle
	Vec3f mVel0;   // _20 the velocity before the walk
	Vec3f mNormal; // _2C of the first triangle hit
	Vec3f mPoint;  // _38 where the first hit was
};

static inline Vec3f scaled(const Vec3f& v, f32 s)
{
	Vec3f r;
	gjScaleVector(s, &r, &v);
	return r;
}

// 0x80041568
RockFlight::RockFlight()
{
	init();
}

// 0x800415A0
RockFlight::RockFlight(f32 size, const SetTransform* from)
{
	init();
	mBody.mPos = from->mPos;
	setRot(&from->mRot);
	setSize(size);
}

// 0x80041620
RockFlight::~RockFlight()
{
	if (mDust != 0) {
		mDust->mFlags |= 1;
		mDust = 0;
	}
}

// 0x800416B4
void RockFlight::init()
{
	mBody.mGravity.x = 0.0f;
	mBody.mGravity.y = -0.11433334f;
	mBody.mGravity.z = 0.0f;
	mBody.mRadius    = 100.0f;
	mMaxSpeed        = 5.0f;
	mFriction        = 0.00001f;
	mDust            = 0;
}

// 0x800416EC
void RockFlight::setPos(const Vec3f* pos)
{
	mBody.mPos = *pos;
}

// 0x80041708
void RockFlight::setVel(const Vec3f* vel)
{
	mBody.mVel = *vel;
}

// 0x80041724
void RockFlight::setRot(const SetRot* rot)
{
	mBody.vfn_1C(rot);
}

// 0x80041750
void RockFlight::setSize(f32 size)
{
	rigidBodySetSize(size, &mBody);
}

// 0x80041770
f32 RockFlight::clampSpeed(f32 max)
{
	Vec3f dir;
	dir.x = mBody.mVel.x;
	dir.y = mBody.mVel.y;
	dir.z = mBody.mVel.z;
	f32 speed = njUnitVector(&dir);
	f32 result;

	if (speed < gUnitVectorMin) {
		dir = gVecZero;
	}
	result = speed;
	if (speed > max) {
		result     = max;
		mBody.mVel = scaled(dir, max);
	}
	return result;
}

// 0x80041858
void RockFlight::addForce(const Vec3f* force)
{
	njAddVector(&mBody.mForce, force);
}

// 0x8004187C
void RockFlight::step()
{
	mBody.vfn_14();
}

// 0x800418A8
void RockFlight::push(void* colli)
{
	CharaColli* c = (CharaColli*)colli;
	ColliContact* contact;

	if (c->mContactCount != 0) {
		for (contact = c->mContacts; contact != 0; contact = contact->mNext) {
			Vec3f force;
			force.x = contact->mPush.x;
			force.y = contact->mPush.y;
			force.z = contact->mPush.z;
			gjScaleVector(0.1f, &force, &force);
			addForce(&force);
		}
	}
}

// 0x80041924
// One triangle the body's sphere sinks depth into bounce the velocity off it with 0.8 of the normal speed kept push the
// body out and keep the first hit
extern "C" int rockHitTri(f32 depth, MeshColli* mesh, int tri, RockHit* hit, const Vec3f* point)
{
	Vec3f vel;
	Vec3f normal;
	f32 dot;

	if (depth > 0.0f) {
		vel = hit->mVel0;
		meshTriNormal(mesh, tri, &normal);
		dot = njInnerProduct(&vel, &normal);
		Vec3f bounce;
		bounce = scaled(scaled(scaled(normal, -2.0f), dot), 0.8f);
		Vec3f out;
		gjAddVector(&out, &bounce, &vel);
		vel = out;
		Vec3f change;
		gjSubVector(&change, &vel, &hit->mVel0);
		njAddVector(&hit->mVel, &change);
		Vec3f lift;
		const Vec3f& ref2 = scaled(normal, hit->mRadius - depth);
		lift = ref2;
		njAddVector(&hit->mPos, &lift);
		if (hit->mHit == 0) {
			hit->mHit    = 1;
			hit->mPoint  = *point;
			hit->mNormal = normal;
		}
	}
	return 1;
}

// 0x80041B1C
int RockFlight::collide(f32 friction)
{
	MeshList list;
	MeshSphere sphere;
	RockHit hit;
	Vec3f side;
	Vec3f along;
	Vec3f dir;

	mBody.vfn_18();
	sphere.mPos    = mBody.mPos;
	sphere.mRadius = mBody.mSize;
	hit.mHit       = 0;
	hit.mRadius    = mBody.mSize;
	hit.mPos       = mBody.mPos;
	hit.mVel       = mBody.mVel;
	hit.mVel0      = mBody.mVel;
	queryBegin(&list);
	queryGatherBall(&list, &sphere);
	if (meshListQuerySphereEx(&list, (const MeshSphere*)&sphere, (void*)rockHitTri, 0, 0, (void*)&hit, (unsigned long)1) != 0 && hit.mHit != 0) {
		njOuterProduct(&hit.mNormal, &hit.mVel, &side);
		njOuterProduct(&side, &hit.mNormal, &along);
		vecNormalize(&dir, &along);
		if (njInnerProduct(&dir, &hit.mVel) >= friction) {
			Vec3f slow = scaled(dir, friction);
			njSubVector(&hit.mVel, &slow);
		} else {
			njSubVector(&hit.mVel, &along);
		}
		Vec3f down;
		down.x = -gVecUp.x;
		down.y = -gVecUp.y;
		down.z = -gVecUp.z;
		Vec3f spin;
		njOuterProduct(&hit.mVel, &down, &spin);
		mBody.mSpin = scaled(spin, 1.0f / mBody.mSize);
	}
	mBody.mVel = hit.mVel;
	setPos(&hit.mPos);
	mBody.vfn_0C();
	return hit.mHit;
}

// 0x80041D78
void RockFlight::setGroundFriction()
{
	mFriction = 0.07f;
}

// 0x80041D84
void RockFlight::update(void* colli)
{
	push(colli);
	step();
	int fast = clampSpeed(mMaxSpeed) >= 1.5f;
	int hit  = collide(mFriction);

	if (fast & hit) {
		Vec3f at;
		at.x = mBody.mPos.x;
		at.y = mBody.mPos.y;
		at.z = mBody.mPos.z;
		at.y = mBody.mPos.y - mBody.mSize;
		particleKeepAt((void**)&mDust, &at, 0, 0x49);
	} else if (mDust != 0) {
		mDust->mFlags |= 1;
		mDust = 0;
	}
}
