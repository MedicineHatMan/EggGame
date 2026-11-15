#include "object/TObjFallThing.h"
#include "Ninja/ninja.h"
#include "egg/ObjEgg.h"
#include "stage/StageObjUtil.h"
#include "sdk/stl/math.h"
#include "object/TObjLauncher.h"
#include "engine/Colli.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "animal/AniMount.h"
#include "egg/EggCreature.h"
#include "object/ObjShadow.h"
#include "sound/Sound3DPlay.h"
#include "engine/DrawTask.h"
#include "object/ObjAction.h"
#include "core/MathUtil.h"
#include "engine/GroundQuery.h"
#include "engine/TParticle.h"
#include "core/VecMath.h"
struct TParticleGenerator;
struct DrawTask;

// The point at pos moved by x y z
static inline Vec3f offsetPos(const Vec3f* pos, f32 x, f32 y, f32 z)
{
	Vec3f sum;
	Vec3f delta;
	delta.x = x;
	delta.y = y;
	delta.z = z;
	gjAddVector(&sum, pos, &delta);
	return sum;
}

// 0x800A39E8
extern "C" TObjFallThing* spawnFallThing(TObject* parent, const FallSpawn* spawn)
{
	return new TObjFallThing(parent, spawn);
}

// 0x800A3A3C
TObjFallThing::TObjFallThing(TObject* parent, const FallSpawn* spawn)
    : TObjAction(parent)
    , mParticles(NULL)
{
	mVel.x  = 0.0f;
	mVel.y  = 0.0f;
	mVel.z  = 0.0f;
	mSpin.x = 0;
	mSpin.y = 0;
	mSpin.z = 0;
	mState  = FALL_FALLING;
	mLife   = 30;
	_00     = sFallThingName[0];
	mSize   = 0x118;
	setObjId(this, 0xFFFF);
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mObj.setModel(spawn->mModel);
	mObj.mTexList = spawn->mTexList;
	mObj.mState   = getDefaultDrawState();
	mType         = spawn->mType;
	mPos          = spawn->mPos;
	mRot          = spawn->mRot;
	if (spawn->mRadius < 0.0f) {
		mRadius = spawn->mRadius;
	} else {
		mRadius = spawn->mModel->model->r;
	}
	mDamage       = spawn->mDamage;
	mPieceModels  = spawn->mPieceModels;
	mPieceFirst   = spawn->mPieceFirst;
	mPieceCount   = spawn->mPieceCount;
	mPieceTexList = spawn->mPieceTexList;
	mGravity      = spawn->mGravity;
	mLandSe       = spawn->mLandSe;
	shadowSetGround(&mShadow, &mPos, 0);
	mShadow._34 = mShadow._30 = 2.0f * mRadius;
	shadowEnable(&mShadow);
	initColli();
	if (spawn->mParticleBank != 2) {
		mParticles = particleGenById(this, &mPos, spawn->mParticleBank, spawn->mParticleId);
	}
}

// 0x800A3C04
TObjFallThing::~TObjFallThing()
{
	termColli();
	removeDrawTask(mDraw);
	releaseObjId(this);
}

// 0x800A3C90
void TObjFallThing::draw()
{
	if (!isInRange(this, &mPos, 1000.0f)) {
		mFlags |= 1;
		return;
	}
	switch (mState) {
	case FALL_FALLING:
		fall();
		break;
	case FALL_LANDED:
		land();
		break;
	}
}

// 0x800A3D0C
void TObjFallThing::fall()
{
	EggHits hits;
	ShatterList pieces;
	Vec3f kick;
	Vec3f size;

	mVel.y += mGravity;
	if (mVel.y < -240.0f) {
		mVel.y = -240.0f;
	}
	if (mVel.y < 0.0f) {
		Vec3f probe = offsetPos(&mPos, 0.0f, 20.0f, 0.0f);
		if (meshColliProbe(hits.mHits, &probe, -1) != NULL && (hits.mMask & 1)) {
			EggHit* hit = hits.mHits;
			shadowSetFromContact(&mShadow, (u8*)hit);
			switch (mType) {
			case FALL_BOUNCE:
				if (mPos.y <= hit->mPos.y + mRadius) {
					mPos = hit->mPos;
					mPos.y += mRadius;
					kick.x = 0.2f * mVel.y;
					kick.y = kick.z = 0.0f;
					vecRotateY(&kick, &kick, (s16)(65536.0f * (0.000030517578f * rand())));
					mVel.x = 0.5f * mVel.x + kick.x;
					mVel.z = 0.5f * mVel.z + kick.z;
					mVel.y *= -0.5f;
					mSpin.x = 182.04445f * kick.z;
					mSpin.z = 182.04445f * -kick.x;
					if ((f32)fabs(mVel.y) < 0.4f) {
						mVel.x = 0.0f;
						mVel.y = 0.0f;
						mVel.z = 0.0f;
						colliDisable(mColli);
						if (mParticles != NULL) {
							mParticles->mFlags |= 4;
						}
						mState = FALL_LANDED;
					}
				}
				break;
			case FALL_SHATTER:
				if (mPos.y <= 0.2f * mRadius + hit->mPos.y) {
					mPos   = hit->mPos;
					mVel.x = 0.0f;
					mVel.y = 0.0f;
					mVel.z = 0.0f;
					mDraw->mFlags |= 1;
					if (mParticles != NULL) {
						mParticles->mFlags |= 4;
					}
					colliDisable(mColli);
					pieces.mModels     = mPieceModels;
					pieces.mTexture    = mPieceTexList;
					pieces.mFirst      = mPieceFirst;
					pieces.mModelCount = mPieceCount;
					pieces.mCount      = mPieceCount;
					pieces.mScale      = 1.0f;
					pieces.mSpeed      = 1.0f;
					size.x             = mRadius;
					size.y             = mRadius;
					size.z             = mRadius;
					shatterList(NULL, &mPos, 0, &mPos, &size, &pieces);
					if (mLandSe != 0xFFFFFFFF) {
						sePlay3D(mLandSe, &mPos);
					}
					mState = FALL_LANDED;
				}
				break;
			}
		}
	}
	vecAdd(&mPos, &mPos, &mVel);
	mRot.x += mSpin.x;
	mRot.y += mSpin.y;
	mRot.z += mSpin.z;
	updateMatrix();
	placeColli();
	if (mParticles != NULL) {
		mParticles->mPos = mPos;
	}
}

// 0x800A40DC
void TObjFallThing::land()
{
	switch (mType) {
	case FALL_BOUNCE:
		if (mLife & 1) {
			mDraw->mFlags &= ~1;
		} else {
			mDraw->mFlags |= 1;
		}
		break;
	case FALL_SHATTER:
		break;
	}
	if (mLife == 0) {
		mFlags |= 1;
	}
	mLife--;
}

// 0x800A4150
// One sphere of mRadius
void TObjFallThing::initColli()
{
	mColli = colliCreate(1, 1);
	colliSetOwner(mColli, (u16)mObjId);
	colliSetMasks(mColli, 4, 0xFF, 0xFF, 0xFE);
	ColliShape shape;
	colliShapeInit(&shape);
	shape.mPos   = mPos;
	shape.mType  = COLLISHAPE_SPHERE;
	shape.mSizeX = mRadius;
	shape.mFlags |= 8;
	colliCopyShapes(mColli, &shape);
}

// 0x800A41F4
void TObjFallThing::placeColli()
{
	ColliShape shape;
	shape.mPos.x = 0.0f;
	shape.mPos.y = 0.0f;
	shape.mPos.z = 0.0f;
	colliPlaceShapes(mColli, &mPos, &shape);
}

// 0x800A4230
void TObjFallThing::termColli()
{
	colliRetire(mColli);
	mColli = NULL;
}

// 0x800A4268
void TObjFallThing::updateMatrix()
{
	mtxSetTransV(mMtx, &mPos);
	mtxRotateZYX((f32*)mMtx, (const f32*)mMtx, (const int*)&mRot);
}

// 0x800A42AC
int TObjFallThing::onHit(const TObjAction*, int, int* result)
{
	HitInfo* hit = (HitInfo*)result;
	resetHitInfo(hit);
	hit->mDamage = mDamage;
	return 1;
}

// 0x800A42EC
void TObjFallThing::vfn_10()
{
}

// 0x800A42F0
void TObjFallThing::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mMtx);
	mObj.draw();
	njPopMatrixEx();
	shadowSetAngle(&mShadow, 0);
}

// 0x800A4340
int TObjFallThing::getKind()
{
	return 3;
}
