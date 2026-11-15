#include "engine/ParticleInternal.h"

// 0x80028020
void ParticleSpiral::init(const ParticleDesc* desc, f32 scale)
{
	Particle::init(desc, scale);
	mCentre     = mPos;
	mOrbit      = desc->mOrbitRange * (2.0f * (partRand() - 0.5f)) + desc->mOrbit;
	mAngle      = (int)(65536.0f * partRand());
	mOrbitSpeed = desc->mSpeedRange * partRand() + desc->mSpeed;
	f32 range   = (f32)desc->mTurnRange;
	range *= 2.0f * (partRand() - 0.5f);
	mTurn       = desc->mTurn + (int)range;
	mUp         = desc->mUpRange * partRand() + desc->mUp;
	mWobble     = 0;
	mWobbleStep = (int)(182.04445f * desc->mWobbleSpeed);
}

// 0x800281F8
void ParticleSpiral::getPos(Vec3f* out) const
{
	*out = mCentre;
	f32 s, c;
	njSinCos(mAngle, &s, &c);
	out->x += mOrbit * s;
	out->z += mOrbit * c;
}

// 0x80028270
int ParticleSpiral::step()
{
	Sprite* sprite = (Sprite*)mSprite;
	mCentre.y += mUp;
	mAngle += mTurn;
	mOrbit += mOrbitSpeed;
	mOrbitSpeed *= mDesc->mSpeedDamp;
	mUp += mDesc->mGravity;
	if (!(mFlags & PART_Stretch)) {
		f32 damp = mDesc->mScaleDamp;
		mWobble += mWobbleStep;
		f32 wobble = mDesc->mWobble * njSin(mWobble);
		mBaseX *= damp;
		mBaseY *= damp;
		sprite->mWidth  = mBaseX * (1.0f + wobble);
		sprite->mHeight = mBaseY * (1.0f - wobble);
		if (mFlags & PART_Spin) {
			spin();
		}
	}
	updateColour();
	if (advance()) {
		return 1;
	}
	updateFrame();
	spriteAdd(sprite);
	return 0;
}

// 0x800283C0
extern "C" void* particleSpiralGNew(TObject* parent, const ParticleDesc* desc, const Vec3f* pos, f32 scale)
{
	return new TParticleSpiralG(parent, desc, pos, scale);
}

const char* gTParticleSpiralGName = "TParticleSpiralG";

// 0x8002842C
TParticleSpiralG::TParticleSpiralG(TObject* parent, const ParticleDesc* desc, const Vec3f* pos, f32 scale)
    : TObject(parent)
{
	_00   = (u32)gTParticleSpiralGName;
	mSize = 0xC8;
	mPart.init(desc, scale);
	njAddVector(&mPart.mCentre, pos);
	mPart.getPos(&mPart.mPos);
	mPart.setLast();
}

// 0x800284BC
void TParticleSpiralG::draw()
{
	mPart.setLast();
	if (mPart.step()) {
		mFlags |= 1;
	} else {
		mPart.getPos(&mPart.mPos);
	}
}

// 0x80028518
void TParticleSpiralG::vfn_10() { mPart.updateStretch(); }

// 0x8002853C
extern "C" void* particleSpiralLNew(TObject* parent, const ParticleDesc* desc, int slot, f32 scale)
{
	return new TParticleSpiralL(parent, desc, slot, scale);
}

const char* gTParticleSpiralLName = "TParticleSpiralL";

// 0x800285A8
TParticleSpiralL::TParticleSpiralL(TObject* parent, const ParticleDesc* desc, int slot, f32 scale)
    : TObject(parent)
{
	_00   = (u32)gTParticleSpiralLName;
	mSize = 0xD8;
	mPart.init(desc, scale);
	mSlot = slot;
	mPart.getPos(&mLocal);
	mPart.setLastAt(&mLocal);
	mPart.setLocalPos(mSlot, &mLocal);
}

// 0x80028644
void TParticleSpiralL::draw()
{
	mPart.setLastAt(&mLocal);
	if (mPart.step()) {
		mFlags |= 1;
	} else {
		mPart.getPos(&mLocal);
		mPart.setLocalPos(mSlot, &mLocal);
	}
}

// 0x800286B4
void TParticleSpiralL::vfn_10() { mPart.updateStretchAt(&mLocal); }

// 0x800286E0
// ~TParticleSpiralL and 0x8002873C ~TParticleSpiralG are inline in the classes draw is the key function so L's vtable
// queued last comes out first its dtor with it
