#include "engine/ParticleInternal.h"

void ParticleVacuum::init(const ParticleDesc* desc, int rotX, int rotY, f32 scale)
{
	Particle::init(desc, scale);
	if (desc->mFlags & PARTDESC_Fixed) {
		f32 r = desc->mRadius;
		f32 s, c;
		njSinCos((s32)(65536.0f * partRand()), &s, &c);
		f32 y = s * r;
		r     = r * c;
		njSinCos((s32)(65536.0f * partRand()), &s, &c);
		mPos.set(s * r, y, c * r);
	}
	mVel.x = 0.0f;
	mVel.y = desc->mUpRange * partRand() + desc->mUp;
	mVel.z = 0.0f;
	// The DOL has CW's doubled branch here bne then beq
	if (rotX != 0 || rotX != 0) {
		mFlags |= PART_Bit200;
		njCalcVectorEx(&mVel, &mVel);
		njCalcPointEx(&mPos, &mPos);
	}
	mDir.x = 0.0f;
	mDir.y = 0.0f;
	mDir.z = 0.0f;
	njSubVector(&mDir, &mPos);
	f32 len = njUnitVector(&mDir);
	if (len < gUnitVectorMin) {
		mDir = gVecZero;
	}
	mDist  = len;
	mSpeed = desc->mSpeedRange * partRand() + desc->mSpeed;
}

// 0x800289B8
int ParticleVacuum::step(Vec3f* pos)
{
	Sprite* sprite = (Sprite*)mSprite;
	if (!(mFlags & PART_Bit100)) {
		if (mDist <= mSpeed) {
			mSpeed = mDist;
			mFlags |= PART_Bit100;
		}
		gjMulAddVector(mSpeed, pos, pos, &mDir);
		mDist -= mSpeed;
		mSpeed *= mDesc->mSpeedDamp;
	}
	if (mFlags & PART_Bit200) {
		njAddVector(pos, &mVel);
	} else {
		pos->y += mVel.y;
	}
	mVel.y += mDesc->mGravity;
	updateScale();
	updateColour();
	if (advance()) {
		return 1;
	}
	updateFrame();
	spriteAdd(sprite);
	return 0;
}

// 0x80028ACC
extern "C" void* particleVacuumGNew(TObject* parent, const ParticleDesc* desc, const Vec3f* pos, f32 scale, int rotX, int rotY)
{
	return new TParticleVacuumG(parent, desc, pos, rotX, rotY, scale);
}

const char* gTParticleVacuumGName = "TParticleVacuumG";

// 0x80028B48
TParticleVacuumG::TParticleVacuumG(TObject* parent, const ParticleDesc* desc, const Vec3f* pos, int rotX, int rotY, f32 scale)
    : TObject(parent)
{
	_00   = (u32)gTParticleVacuumGName;
	mSize = 0xC0;
	mPart.init(desc, rotX, rotY, scale);
	mPart.addPos(pos);
}

// 0x80028BD4
TParticleVacuumG::~TParticleVacuumG()
{
}

// 0x80028C30
void TParticleVacuumG::draw()
{
	mPart.setLast();
	if (mPart.step(&mPart.mPos)) {
		mFlags |= 1;
	}
}

// 0x80028C80
void TParticleVacuumG::vfn_10()
{
	mPart.updateStretch();
}

// 0x80028CA4
extern "C" void* particleVacuumLNew(TObject* parent, const ParticleDesc* desc, int slot, f32 scale, int rotX, int rotY)
{
	return new TParticleVacuumL(parent, desc, slot, rotX, rotY, scale);
}

const char* gTParticleVacuumLName = "TParticleVacuumL";

// 0x80028D20
TParticleVacuumL::TParticleVacuumL(TObject* parent, const ParticleDesc* desc, int slot, int rotX, int rotY, f32 scale)
    : TObject(parent)
{
	_00   = (u32)gTParticleVacuumLName;
	mSize = 0xD0;
	mPart.init(desc, rotX, rotY, scale);
	mSlot  = slot;
	mLocal = mPart.mPos;
	mPart.setLastAt(&mLocal);
	mPart.setLocalPos(mSlot, &mLocal);
}

// 0x80028DE0
TParticleVacuumL::~TParticleVacuumL()
{
}

// 0x80028E3C
void TParticleVacuumL::draw()
{
	mPart.setLastAt(&mLocal);
	if (mPart.step(&mLocal)) {
		mFlags |= 1;
	} else {
		mPart.setLocalPos(mSlot, &mLocal);
	}
}

// 0x80028EA4
void TParticleVacuumL::vfn_10()
{
	mPart.updateStretchAt(&mLocal);
}
