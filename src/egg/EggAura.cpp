#include "egg/ObjEgg.h"
#include "egg/EggCreature.h"
#include "core/MathUtil.h"
#include "engine/FxSystem.h"
#include "engine/TParticle.h"
struct Fx;

// 0x801CFF0C
EggAura::EggAura(void* owner)
{
	mOwner = (TObject*)owner;
	mBody  = NULL;
	mGen   = NULL;
	mFrame = 0;
}

// 0x801CFF24
EggAura::~EggAura()
{
	if (mGen != NULL) {
		mGen->mFlags |= 1;
	}
}

// 0x801CFF7C
void EggAura::start()
{
	if (mGen == NULL) {
		mGen = (EggParticle*)particleGenById(mOwner, &gVecZero, 0, 15);
	}
}

// 0x801CFFCC
void EggAura::stop()
{
	if (mGen != NULL) {
		mGen->mFlags |= 1;
		mGen = NULL;
	}
}

// 0x801CFFF0
void EggAura::update()
{
	f32 mtx[12];
	Vec3f pos;
	Vec3f vel;
	f32 scale;

	if (mGen != NULL) {
		f32 x = mBody->mPos.x;
		f32 y = mBody->mPos.y;
		f32 z = mBody->mPos.z;
		y -= mBody->mRadius;
		mGen->mPos.set(x, y, z);
	}
	if ((mFrame & 7) == 0) {
		pos   = mBody->mPos;
		vel   = mBody->mVel;
		scale = mBody->mRadius / 5.0f;
		pos.y = pos.y - mBody->mRadius;
		mtxSetTransV(mtx, &pos);
		mtxRotateY(mtx, mtx, vecAngleXZ(&vel));
		mtxScale(scale, scale, scale, mtx, mtx);
		fxSpawnAt(0, 0, 15, mtx);
	}
	mFrame++;
}
