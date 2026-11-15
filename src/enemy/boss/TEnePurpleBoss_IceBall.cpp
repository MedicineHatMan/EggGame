#include "enemy/boss/EneBoss.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "sound/SeHolder.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "enemy/boss/TEnePurpleBoss.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "engine/Color.h"
#include "egg/EggCreature.h"
#include "enemy/ObjEnemy.h"
#include "enemy/boss/TEnePurpleBoss_IceBall.h"
#include "Ninja/ninja.h"
#include "animal/AniMount.h"
#include "Ninja/ninja.h"
#include "ui/AeArc.h"
#include "stl/math.h"
#include "core/MathUtil.h"
#include "engine/Colli.h"
#include "camera/TCameraCtrl.h"
#include "engine/TParticle.h"
#include "enemy/EnemyFx.h"
struct NJS_OBJECT;
struct ColliShape;
struct Quat;
struct ColliContact;
struct CharaColli;
struct GjModel;
struct NJS_QUATERNION;

extern "C" {
extern const int sIceBallArmourModels[] = {
	3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
}; // the armour's models
extern const int sIceBallPieceModels[] = {
	0,
	1,
	2,
	0,
}; // a ball's piece models
extern const ColliShape sPurpleBoss_IceBallShape[] = {
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 7.5f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

typedef char TEnePurpleBoss_BigIceFragment_size_check[(sizeof(TEnePurpleBoss_BigIceFragment) == 0x4B4) ? 1 : -1];

/**
 * @size 0x180
 */
class IceBallFragments : public TObject {
public:
	IceBallFragments(TObject* parent, const Vec3f* pos, const Vec3f* vel);
	virtual ~IceBallFragments();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();

	int mWait;               // _20
	u32 mColor;              // _24
	int mCount;              // _28
	HitVec mPos[12];         // _2C
	HitVec mVel[12];         // _BC
	NJS_OBJECT* mModels[12]; // _14C
	DrawTask* mDrawTask;     // _17C
};

typedef char IceBallFragments_size_check[(sizeof(IceBallFragments) == 0x180) ? 1 : -1];

typedef char TEnePurpleBoss_IceBall_size_check[(sizeof(TEnePurpleBoss_IceBall) == 0x78) ? 1 : -1];

static inline f32 randUnit()
{
	return 3.0517578125e-05f * (f32)rand();
}

static inline f32 randSpan(f32 span, f32 base)
{
	return span * randUnit() + base;
}

#define RAND_SIGNED() (2.0f * (3.0517578125e-05f * (f32)rand() - 0.5f))

const char* gTEnePurpleBoss_BigIceFragmentName = "TEnePurpleBoss_BigIceFragment";

// 0x800D1558
TEnePurpleBoss_BigIceFragment::TEnePurpleBoss_BigIceFragment(TObject* parent, const Vec3f* pos, const f32* rot, f32 offset)
    : TObject(parent)
{
	_00   = (u32)gTEnePurpleBoss_BigIceFragmentName;
	mSize = 0x4B4;
	for (int i = 0; i < 26; i++) {
		mModels[i] = modelAtTEnePurpleBoss(sIceBallArmourModels[i]);
	}
	mRot[0] = rot[0];
	mRot[1] = rot[1];
	mRot[2] = rot[2];
	mRot[3] = rot[3];
	gjPushUnitMatrix();
	Vec3f at;
	at.x = pos->x;
	at.y = pos->y;
	at.z = pos->z;
	at.y = pos->y + offset;
	njTranslateEx(&at);
	njQuaternionEx((const NJS_QUATERNION*)mRot);
	f32 scale = offset * __frsqrte(pos->x * pos->x + pos->z * pos->z);
	at.y      = 50.0f;
	at.x      = at.x + pos->x * scale;
	at.z      = at.z + pos->z * scale;
	for (int i = 0; i < 26; i++) {
		IceChunk* chunk = &mChunks[i];
		njCalcPointEx((const Vec3f*)mModels[i]->pos, (const Vec3f*)&chunk->mPos);
		chunk->mVel.x = chunk->mPos.x;
		chunk->mVel.y = chunk->mPos.y;
		chunk->mVel.z = chunk->mPos.z;
		njSubVector(&chunk->mVel, &at);
		if (njUnitVector(&chunk->mVel) < 1e-06f) {
			chunk->mVel = gVecZero;
		}
		gjScaleVector(3.0f * randUnit() + 2.0f, &chunk->mVel, &chunk->mVel);
		chunk->mRotX  = 0;
		chunk->mRotY  = 0;
		chunk->mSpinX = (int)(1000.0f * RAND_SIGNED());
		chunk->mSpinY = (int)(500.0f * RAND_SIGNED());
	}
	njPopMatrixEx();
	mWait     = 0x2A;
	mColor    = 0xFFFFFFFF;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	at.x      = pos->x;
	at.z      = pos->z;
	at.y      = pos->y + offset;
	particleOnceByIdDefault(&at, 1, 0x42);
	particleOnceByIdDefault(&at, 1, 0x43);
}

// 0x800D18E4
IceChunk::IceChunk()
{
}

// 0x800D18E8
TEnePurpleBoss_BigIceFragment::~TEnePurpleBoss_BigIceFragment()
{
	removeDrawTask(mDrawTask);
}

// 0x800D1950
void TEnePurpleBoss_BigIceFragment::draw()
{
	if (mWait > 0) {
		mWait--;
	} else if (addColorAlpha(&mColor, -0x10) != 0) {
		mFlags |= 1;
	}
	IceChunk* chunk = mChunks;
	for (int i = 0; i < 26; i++, chunk++) {
		chunk->mVel.y += -0.1f;
		njAddVector(&chunk->mPos, &chunk->mVel);
		chunk->mRotX += chunk->mSpinX;
		chunk->mRotY += chunk->mSpinY;
	}
}

// 0x800D1A14
void TEnePurpleBoss_BigIceFragment::vfn_10()
{
}

// 0x800D1A18
static void iceChanCtrl(int, u8 enable, int ambSrc, int matSrc, int lights)
{
	GXSetChanCtrl(GX_COLOR0, enable, ambSrc, matSrc, lights, GX_DF_CLAMP, GX_AF_SPOT);
	GXSetChanCtrl(GX_ALPHA0, GX_ENABLE, GX_SRC_REG, matSrc, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
}

static inline void setIceColor(u32 color)
{
	u8 rgba[4];
	rgba[0] = color >> 24;
	rgba[1] = color >> 16;
	rgba[2] = color >> 8;
	rgba[3] = color;
	GXSetChanAmbColor(GX_ALPHA0, *(GXColor*)rgba);
}

static inline void setIceColorIn(u8* rgba, u32 color)
{
	rgba[0] = color >> 24;
	rgba[1] = color >> 16;
	rgba[2] = color >> 8;
	rgba[3] = color;
	GXSetChanAmbColor(GX_ALPHA0, *(GXColor*)rgba);
}

// 0x800D1A90
void TEnePurpleBoss_BigIceFragment::render()
{
	u8 rgba[4];
	njSetTexture(shapeTEnePurpleBoss());
	setIceColorIn(rgba, mColor);
	void* funcs[8];
	funcs[5] = (void*)iceChanCtrl;
	gjChangeDrawObjIF(funcs, 0x80);
	IceChunk* chunk = mChunks;
	for (int i = 0; i < 26; i++) {
		njPushMatrixEx();
		njTranslateEx(&chunk->mPos);
		njQuaternionEx((const NJS_QUATERNION*)mRot);
		gjRotateY(chunk->mRotY);
		gjRotateX(chunk->mRotX);
		gjDrawModel(mModels[i]->model);
		chunk++;
		njPopMatrixEx();
	}
	funcs[5] = 0;
	gjChangeDrawObjIF(funcs, 0x80);
}

// 0x800D1B78
IceBallFragments::IceBallFragments(TObject* parent, const Vec3f* pos, const Vec3f* vel)
    : TObject(parent)
{
	Vec3f dir;
	dir.x     = vel->x;
	dir.y     = vel->y;
	dir.z     = vel->z;
	dir.y     = 0.0f;
	f32 speed = njUnitVector(&dir);
	if (speed < 1e-06f) {
		dir = gVecZero;
	}
	f32 sideX = dir.z;
	f32 sideZ = -dir.x;
	mCount    = 0;
	int left  = 0xC;
	while (left > 0) {
		int take = (int)(3.0f * randUnit() + 0.0f) + 1;
		if (take > left) {
			take = left;
		}
		left -= take;
		mModels[mCount] = modelAtTEnePurpleBoss(sIceBallPieceModels[take - 1]);
		mPos[mCount].x  = pos->x;
		mPos[mCount].y  = pos->y;
		mPos[mCount].z  = pos->z;
		f32 across      = speed * (0.5f * (2.0f * (randUnit() - 0.5f)));
		f32 vx          = sideX * across;
		f32 vz          = sideZ * across;
		f32 ahead       = speed - __fabs(across);
		vx              = dir.x * ahead + vx;
		vz              = dir.z * ahead + vz;
		mVel[mCount].x  = vx;
		mVel[mCount].y  = 1.0f * randUnit() + 1.0f;
		mVel[mCount].z  = vz;
		mCount++;
	}
	mWait     = 0x2A;
	mColor    = 0xFFFFFFFF;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
}

// 0x800D1EDC
IceBallFragments::~IceBallFragments()
{
	removeDrawTask(mDrawTask);
}

// 0x800D1F44
void IceBallFragments::draw()
{
	if (mWait > 0) {
		mWait--;
	} else if (addColorAlpha(&mColor, -0x12) != 0) {
		mFlags |= 1;
	}
	for (int i = 0; i < mCount; i++) {
		mVel[i].y += -0.1f;
		njAddVector(&mPos[i], &mVel[i]);
	}
}

// 0x800D1FF4
void IceBallFragments::vfn_10()
{
}

// 0x800D1FF8
void IceBallFragments::render()
{
	u8 rgba[4];
	njSetTexture(shapeTEnePurpleBoss());
	setIceColorIn(rgba, mColor);
	void* funcs[8];
	funcs[5] = (void*)iceChanCtrl;
	gjChangeDrawObjIF(funcs, 0x80);
	for (int i = 0; i < mCount; i++) {
		njPushMatrixEx();
		njTranslateEx(&mPos[i]);
		gjDrawModel(mModels[i]->model);
		njPopMatrixEx();
	}
	funcs[5] = 0;
	gjChangeDrawObjIF(funcs, 0x80);
}

// 0x800D20D4
extern "C" void iceBallLoad()
{
	sIceBallHead  = 0;
	sIceBallLimit = 5;
}

// 0x800D20E8
extern "C" void iceBallFree()
{
	sIceBallHead = 0;
}

// 0x800D20F4
extern "C" void setIceBallLimit(int limit)
{
	if (sIceBallLimit != limit) {
		sIceBallLimit = limit;
		enforceIceBallLimit();
	}
}

// 0x800D2124
// Breaks every ball keeping the limit
extern "C" void breakAllIceBalls()
{
	int limit = sIceBallLimit;
	setIceBallLimit(0);
	setIceBallLimit(limit);
}

// 0x800D215C
// Throws count balls in a fan of 0x5000 around the way to the middle of the arena each with a speed picked from speeds
extern "C" void throwIceBalls(TObject* parent, const Vec3f* pos, const f32* speeds, int speedCount, int count, f32 radius)
{
	int angle = (int)(10430.38043493439 * (f32)atan2(-pos->x, -pos->z));
	angle -= 0x2800;
	int step = 0x5000 / count;
	for (int i = 0; i < count; i++) {
		Vec3f vel;
		njSinCos(angle, &vel.x, &vel.z);
		int pick = (int)((f32)speedCount * randUnit());
		if (pick >= speedCount) {
			pick = speedCount - 1;
		}
		f32 speed = speeds[pick];
		vel.x *= speed;
		vel.y = randSpan(2.0f, 0.0f);
		vel.z *= speed;
		new TEnePurpleBoss_IceBall(parent, pos, &vel, radius);
		angle += step;
	}
}

const char* gTEnePurpleBoss_IceBallName = "TEnePurpleBoss_IceBall";

// 0x800D2328
TEnePurpleBoss_IceBall::TEnePurpleBoss_IceBall(TObject* parent, const Vec3f* pos, const Vec3f* vel, f32 radius)
    : TObjAction(parent)
{
	_00   = (u32)gTEnePurpleBoss_IceBallName;
	mSize = 0x78;
	setObjId(this, 0xFFFF);
	mPrev   = 0;
	mNext   = 0;
	mPos.x  = pos->x;
	mPos.y  = pos->y;
	mPos.z  = pos->z;
	mVel.x  = vel->x;
	mVel.y  = vel->y;
	mVel.z  = vel->z;
	mSpeed  = vecLenXZ(&mVel);
	mRadius = radius;
	njCreateQuaternion(0, 0.0f, 0.0f, 1.0f, (NJS_QUATERNION*)mRot);
	mWait  = 0x30;
	mState = 0;
	mColli = (CharaColli*)colliCreate(1, 1);
	colliCopyShapes(mColli, sPurpleBoss_IceBallShape);
	colliSetOwner(mColli, (u16)mObjId);
	colliSetMasks(mColli, 2, 4, 0, 0x14);
	colliDisable((CharaColli*)mColli);
	colliShapeSetBit8(mColli, 0);
	syncCollider();
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	link();
}

// 0x800D2498
TEnePurpleBoss_IceBall::~TEnePurpleBoss_IceBall()
{
	unlink();
	colliRetire(mColli);
	mColli = 0;
	removeDrawTask(mDrawTask);
	releaseObjId(this);
}

// 0x800D251C
// Past the newest balls up to the limit the rest break
extern "C" void enforceIceBallLimit()
{
	TEnePurpleBoss_IceBall* ball = sIceBallHead;
	int i                        = 0;
	while (ball != 0 && i < sIceBallLimit) {
		ball = ball->mNext;
		i++;
	}
	if (i >= sIceBallLimit) {
		while (ball != 0) {
			TEnePurpleBoss_IceBall* next = ball->mNext;
			ball->markBroken();
			ball->unlink();
			ball = next;
		}
	}
}

// 0x800D2594
void TEnePurpleBoss_IceBall::link()
{
	mPrev = 0;
	mNext = sIceBallHead;
	if (sIceBallHead != 0) {
		sIceBallHead->mPrev = this;
	}
	sIceBallHead = this;
	enforceIceBallLimit();
}

// 0x800D25D8
void TEnePurpleBoss_IceBall::unlink()
{
	if (mNext != 0) {
		mNext->mPrev = mPrev;
	}
	if (mPrev != 0) {
		mPrev->mNext = mNext;
	}
	if (sIceBallHead == this) {
		sIceBallHead = mNext;
	}
	mPrev = 0;
	mNext = 0;
}

// 0x800D2624
void TEnePurpleBoss_IceBall::vfn_6C(const Vec3f* pos)
{
	mPos.x = pos->x;
	mPos.y = pos->y;
	mPos.z = pos->z;
}

// 0x800D2640
int TEnePurpleBoss_IceBall::onHit(const TObjAction* other, int arg, int* result)
{
	HitInfo* hit = (HitInfo*)result;
	int took     = 0;
	if (isPlayerObj(other) != 0) {
		hit->mDamage = 0xA;
		took         = 1;
		hit->mFlags.clear();
		hit->mPos.x = 0.0f;
		hit->mPos.y = 0.0f;
		hit->mPos.z = 0.0f;
	}
	return took;
}

// 0x800D26A4
void TEnePurpleBoss_IceBall::syncCollider()
{
	ColliShape* shapes = colliGetShapes(mColli);
	colliShapeSetPos(shapes, &mPos);
	colliShapeSetRadius((ColliShape*)shapes, mRadius);
}

// 0x800D26EC
void TEnePurpleBoss_IceBall::markBroken()
{
	mState = 2;
	colliDisable(mColli);
}

// 0x800D2718
void TEnePurpleBoss_IceBall::roll()
{
	Vec3f from;
	from.x = mPos.x;
	from.y = mPos.y;
	from.z = mPos.z;
	Vec3f next;
	next.x = mPos.x;
	next.y = mPos.y;
	next.z = mPos.z;
	Vec3f push;
	if (colliGetContactCount(mColli) > 0) {
		int pushed = 0;
		for (ColliContact* it = mColli->mContacts; it != 0; it = it->mNext) {
			if ((colliContactShape(it)->mFlags & 0x10) == 0) {
				pushed = 1;
				njAddVector(&push, &it->mPush);
			}
		}
		if (pushed != 0) {
			push.y  = 0.0f;
			f32 len = njUnitVector(&push);
			if (len < 1e-06f) {
				push = gVecZero;
			}
			if (len > 0.001f) {
				mVel.x = push.x * mSpeed;
				mVel.z = push.z * mSpeed;
			}
			Vec3f at;
			at.x = mPos.x;
			at.y = mPos.y;
			at.z = mPos.z;
			gjMulAddVector(mRadius, &at, &at, &push);
			bounceEffect(&at, &push);
		}
	}
	Vec3f dir;
	dir.x     = mVel.x;
	dir.y     = mVel.y;
	dir.z     = mVel.z;
	dir.y     = 0.0f;
	f32 speed = njUnitVector(&dir);
	if (speed < 1e-06f) {
		dir = gVecZero;
	}
	gjMulAddVector(speed + mRadius, &dir, &from, &dir);
	Vec3f wall;
	Vec3f normal;
	if (probeGroundLine(&from, &dir, &wall, &normal) != 0) {
		dir.x = from.x;
		dir.y = from.y;
		dir.z = from.z;
		njSubVector(&dir, &wall);
		dir.y = 0.0f;
		if (njUnitVector(&dir) < 1e-06f) {
			dir = gVecZero;
		}
		gjMulAddVector(mRadius, &next, &wall, &dir);
		dir.x    = mVel.x;
		dir.y    = mVel.y;
		dir.z    = mVel.z;
		dir.y    = 0.0f;
		f32 flat = njUnitVector(&dir);
		if (flat < 1e-06f) {
			dir = gVecZero;
		}
		normal.y = 0.0f;
		if (njUnitVector(&normal) < 1e-06f) {
			normal = gVecZero;
		}
		vecReflect(&dir, &normal, &dir);
		mVel.x = dir.x * flat;
		mVel.z = dir.z * flat;
		bounceEffect(&wall, &normal);
	} else {
		next.x += mVel.x;
		next.z += mVel.z;
	}
	mVel.y += -0.1f;
	f32 y = next.y + mVel.y;
	if (50.0f <= y - mRadius) {
		next.y = y;
	} else {
		next.y = 50.0f + mRadius;
		mVel.y = 0.0f;
		dir.x  = next.x;
		dir.y  = 50.0f + mRadius;
		dir.z  = next.z;
		njSubVector(&dir, &from);
		dir.y = 0.0f;
		rollQuat(&dir, (Quat*)mRot, mRadius);
	}
	mPos.x = next.x;
	mPos.y = next.y;
	mPos.z = next.z;
	syncCollider();
}

// 0x800D2AEC
void TEnePurpleBoss_IceBall::checkHits()
{
	HitInfo hit;
	TObjAction* obj;
	int broke = 0;
	int drop  = 0;
	for (ColliContact* it = mColli->mContacts; it != 0; it = it->mNext) {
		u32 id = colliContactOwner(it);
		if (colliContactHasBit8(it) != 0) {
			obj = findObjById(id);
			if (obj != 0 && obj->onHit(this, colliContactTag(it), (int*)&hit) != 0) {
				broke = 1;
				if (hit.mDamage > 0) {
					drop = 1;
					break;
				}
			}
		}
	}
	if (drop != 0) {
		spawnDrop(&mPos, 1, 1);
	}
	if (broke != 0) {
		markBroken();
	}
}

// 0x800D2BD8
void TEnePurpleBoss_IceBall::bounceEffect(const Vec3f* at, const Vec3f* normal)
{
	particleOnceByIdRotDefault(at, 1, 0x47, 0x4000, (int)(10430.38043493439 * (f32)atan2(normal->x, normal->z)), 0);
	int pitch = (int)(12288.0f * randUnit());
	if (pitch >= 0x3000) {
		pitch = 0x2FFF;
	}
	seStartAtWith(0x13C, at, (s16)(pitch + 0x3000));
}

// 0x800D2C98
void TEnePurpleBoss_IceBall::shatter()
{
	particleOnceByIdDefault(&mPos, 1, 0x4B);
	particleOnceByIdDefault(&mPos, 1, 0x4C);
	new IceBallFragments(objParent(this), &mPos, &mVel);
	int pitch = (int)(12288.0f * randUnit());
	if (pitch >= 0x3000) {
		pitch = 0x2FFF;
	}
	seStartAtWith(0x13D, &mPos, (s16)(pitch + 0x3000));
}

// 0x800D2D60
void TEnePurpleBoss_IceBall::draw()
{
	checkHits();
	switch (mState) {
	case 0:
		mWait--;
		if (mWait <= 0) {
			colliEnable(mColli);
			mState = 1;
		}
		roll();
		break;
	case 1:
		roll();
		break;
	case 2:
		shatter();
		mFlags |= 1;
		break;
	}
}

// 0x800D2E08
void TEnePurpleBoss_IceBall::vfn_10()
{
	if (cameraIsVisible(3000.0f, mRadius, &mPos, 0) != 0) {
		mFlags &= ~0x10;
	} else {
		mFlags |= 0x10;
	}
}

// 0x800D2E68
void TEnePurpleBoss_IceBall::render()
{
	njSetTexture(shapeTEnePurpleBoss());
	njPushMatrixEx();
	njTranslateEx(&mPos);
	njQuaternionEx((const NJS_QUATERNION*)mRot);
	f32 scale = mRadius / 7.5f;
	njScale(0, scale, scale, scale);
	gjDrawObject(modelAtTEnePurpleBoss(0x1F));
	njPopMatrixEx();
}

// 0x800D2ED8
int TEnePurpleBoss_IceBall::getKind()
{
	return 2;
}
