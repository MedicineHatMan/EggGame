#include "enemy/boss/EneBoss.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "enemy/EnemyCommon.h"
#include "enemy/TEneAntsBomb.h"
#include "object/ObjAction.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"
#include "core/MathUtil.h"
#include "stl/math.h"
#include "camera/TCameraCtrl.h"
#include "enemy/EnemyFx.h"
struct NJS_OBJECT;
struct CharaColli;
struct ColliShape;

extern "C" {
extern const ColliShape sAntsBombShape[] = {
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 5.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
extern const ColliShape sAntsBombBlastShape[] = {
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 5.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

const char* gTEneAntsBombName = "TEneAntsBomb";

// 0x80091BC4
TEneAntsBomb::TEneAntsBomb(TObject* parent, Vec3f* pos, Vec3f* target, Vec3f* vel)
    : TObjAction(parent)
{
	_00   = (u32)gTEneAntsBombName;
	mSize = 0x80;
	setObjId(this, 0xFFFF);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mColli    = colliCreate(1, 1);
	colliCopyShapes((CharaColli*)mColli, (const ColliShape*)sAntsBombShape);
	colliSetOwner(mColli, (u16)mObjId);
	colliSetMasks((CharaColli*)mColli, 2, 0xFF, 0, 0x14);
	colliShapeSetBit8((CharaColli*)mColli, 0);
	colliShapeSetTag(mColli, 0, 0);
	mBody.mPos = *pos;
	mLife      = 600;
	Vec3f to   = *target;
	njSubVector(&to, pos);
	f32 height     = to.y;
	f32 dist       = vecLenXZ(&to);
	f32 velY       = vel->y;
	f32 speed      = vecLenXZ(vel);
	mBody.mRot.y   = (int)(10430.38043493439 * (f32)atan2(vel->x, vel->z));
	f64 angle      = atan2(speed, velY);
	f32 a          = 2.0f * (dist * (velY / speed) - height);
	f32 b          = -0.24f * dist;
	f32 c          = dist * (-0.12f * dist);
	mBody.mRot.x   = (int)(10430.38043493439 * angle);
	mBody.mRot.z   = 0;
	mBody.mForce.z = speed;
	f32 scale;
	if (quadDiscriminant(a, b, c) < 0.0f) {
		scale = 1.5f;
	} else {
		scale = quadRoot(a, b, c) / speed;
	}
	mBody.mVel = *vel;
	gjScaleVector(scale, &mBody.mVel, &mBody.mVel);
	njSubVector(&mBody.mPos, &mBody.mVel);
}

// 0x80091E3C
TEneAntsBomb::~TEneAntsBomb()
{
	colliRetire((CharaColli*)mColli);
	mColli = 0;
	removeDrawTask(mDrawTask);
	releaseObjId(this);
}

// 0x80091EE4
int TEneAntsBomb::onHit(const TObjAction* other, int arg, int* result)
{
	int hurt = 0;
	if (isPlayerObj(other) != 0) {
		HitInfo* hit = (HitInfo*)result;
		hit->mDamage = 0x14;
		hurt         = 1;
		hit->mFlags.clear();
	}
	return hurt;
}

// 0x80091F38
void TEneAntsBomb::draw()
{
	Vec3f from;
	from = mBody.mPos;
	njAddVector(&mBody.mPos, &mBody.mVel);
	Vec3f hit;
	if (probeGroundLine(&from, &mBody.mPos, &hit, 0) != 0) {
		mBody.mPos = hit;
		new TEneAntsBomb_Blast(objParent(this), &mBody.mPos);
		mFlags |= 1;
	}
	mBody.mVel.y += -0.12f;
	if (mColli != 0) {
		ColliShape* shape = colliGetShapes(mColli);
		*(Vec3f*)shape    = mBody.mPos;
	}
	int rotX = mBody.mRot.x;
	rotX -= 0x100;
	if (rotX < 0) {
		rotX = 0;
	}
	mBody.mRot.x = rotX;
	mLife--;
	if (mLife <= 0) {
		mFlags |= 1;
	}
}

// 0x80092054
void TEneAntsBomb::vfn_10()
{
	if (cameraIsVisible(700.0f, 7.0f, &mBody.mPos, 0)) {
		setTaskHidden(mDrawTask, 0);
	} else {
		setTaskHidden(mDrawTask, 1);
	}
}

// 0x800920BC
void TEneAntsBomb::render()
{
	enemyCommonSetTexture();
	njPushMatrixEx();
	njTranslateEx(&mBody.mPos);
	gjRotateY(mBody.mRot.y);
	gjRotateX(mBody.mRot.x);
	gjDrawObject(enemyBombModel());
	njPopMatrixEx();
}

const char* gTEneAntsBomb_BlastName = "TEneAntsBomb_Blast";

// 0x80092110
TEneAntsBomb_Blast::TEneAntsBomb_Blast(TObject* parent, Vec3f* pos)
    : TObjAction(parent)
{
	_00   = (u32)gTEneAntsBomb_BlastName;
	mSize = 0x48;
	setObjId(this, 0xFFFF);
	mColli = colliCreate(1, 1);
	colliCopyShapes(mColli, (const ColliShape*)sAntsBombBlastShape);
	colliSetOwner(mColli, (u16)mObjId);
	colliSetMasks(mColli, 2, 0xFF, 0xFF, 0x14);
	colliShapeSetBit8(mColli, 0);
	colliShapeSetTag(mColli, 0, 0);
	ColliShape* shape = colliGetShapes(mColli);
	f32 z;
	f32 y;
	f32 x;
	x                  = pos->x;
	y                  = pos->y;
	z                  = pos->z;
	((Vec3f*)shape)->x = x;
	((Vec3f*)shape)->y = y;
	((Vec3f*)shape)->z = z;
	mPos.x             = x;
	mPos.y             = y;
	mPos.z             = z;
	mRadius            = 0.0f;
	mTime              = -8;
	fxSpawnFlat(5, (void*)8, pos);
}

// 0x8009221C
TEneAntsBomb_Blast::~TEneAntsBomb_Blast()
{
	colliRetire(mColli);
	mColli = 0;
	releaseObjId(this);
}

// 0x80092294
int TEneAntsBomb_Blast::onHit(const TObjAction* other, int arg, int* result)
{
	int hurt = 0;
	if (mTime >= 0 && isPlayerObj(other) != 0) {
		HitInfo* hit = (HitInfo*)result;
		hit->mDamage = 0x14;
		hurt         = 1;
		hit->mFlags.clear();
	}
	return hurt;
}

// 0x800922F4
void TEneAntsBomb_Blast::draw()
{
	mTime++;
	if (mTime <= 0) {
		mRadius = 4.375f * (f32)(mTime + 8);
	} else {
		mRadius = (f32)mTime;
		if (mRadius >= 30.0f) {
			mRadius = 30.0f;
			mFlags |= 1;
		}
	}
	if (mColli != 0) {
		colliShapeSetRadius(colliGetShapes(mColli), mRadius);
	}
}

// 0x800923C0
Vec3f* TEneAntsBomb_Blast::getPos()
{
	return &mPos;
}

// 0x800923C8
int TEneAntsBomb_Blast::getKind()
{
	return 2;
}

// 0x800923D0
Vec3f* TEneAntsBomb::getPos()
{
	return &mBody.mPos;
}

// 0x800923D8
int TEneAntsBomb::getKind()
{
	return 2;
}
