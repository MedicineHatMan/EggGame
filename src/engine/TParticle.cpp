#include "engine/ParticleInternal.h"

// 0x8002616C
void ParticleEmit::init(const ParticleDesc* desc, int rotX, int rotY, f32 scale)
{
	Particle::init(desc, scale);

	f32 speed = desc->mSpeedRange * partRand() + desc->mSpeed;
	if (desc->mFlags & PARTDESC_Fixed) {
		mVel.x = 0.0f;
		mVel.z = speed;
	} else {
		f32 s, c;
		njSinCos((s32)(65536.0f * partRand()), &s, &c);
		mVel.x = s * speed;
		mVel.z = c * speed;
	}
	mVel.y = desc->mUpRange * partRand() + desc->mUp;

	mAccel.x = 0.0f;
	mAccel.y = desc->mGravity;
	mAccel.z = 0.0f;
	mAxis.x  = 0.0f;
	mAxis.y  = 1.0f;
	mAxis.z  = 0.0f;

	if (rotX != 0 || rotY != 0) {
		mFlags |= PART_Bit100;
		njCalcVectorEx(&mAccel, &mAccel);
		njCalcVectorEx(&mAxis, &mAxis);
		njCalcPointEx(&mVel, &mVel);
		njCalcPointEx(&mPos, &mPos);
	}
	if (desc->mFlags & PARTDESC_KeepRot) {
		mRotX = rotX | 1;
		mRotY = rotY;
	}
}

// 0x80026324
int ParticleEmit::step()
{
	Sprite* sprite = (Sprite*)mSprite;
	f32 along;
	f32 damp = mDesc->mSpeedDamp;
	if (mFlags & PART_Bit100) {
		njAddVector(&mVel, &mAccel);
		along = njInnerProduct(&mVel, &mAxis);
		Vec3f flat;
		gjMulAddVector(-along, &flat, &mVel, &mAxis);
		gjScaleVector(damp, &flat, &flat);
		gjMulAddVector(along, &mVel, &flat, &mAxis);
	} else {
		mVel.y += mAccel.y;
		mVel.x *= damp;
		mVel.z *= damp;
	}
	updateScale();
	updateColour();
	updateFrame();
	if (advance()) {
		return 1;
	}
	spriteAdd(sprite);
	return 0;
}

// 0x80026440
extern "C" void* particleEmitGNew(TObject* parent, const ParticleDesc* desc, const Vec3f* pos, f32 scale, int rotX, int rotY)
{
	return new TParticleEmitG(parent, desc, pos, rotX, rotY, scale);
}

const char* gTParticleEmitGName = "TParticleEmitG";

// 0x800264BC
TParticleEmitG::TParticleEmitG(TObject* parent, const ParticleDesc* desc, const Vec3f* pos, int rotX, int rotY, f32 scale)
    : TObject(parent)
{
	_00   = (u32)gTParticleEmitGName;
	mSize = 0xC4;
	mPart.init(desc, rotX, rotY, scale);
	mPart.addPos(pos);
	mPart.setLast();
}

// 0x80026550
TParticleEmitG::~TParticleEmitG()
{
}

// 0x800265AC
void TParticleEmitG::draw()
{
	mPart.setLast();
	mPart.addPos(&mPart.mVel);
	if (mPart.step()) {
		mFlags |= 1;
	}
}

// 0x80026604
void TParticleEmitG::vfn_10()
{
	mPart.updateStretch();
}

// 0x80026628
extern "C" void* particleEmitLNew(TObject* parent, const ParticleDesc* desc, int slot, f32 scale, int rotX, int rotY)
{
	return new TParticleEmitL(parent, desc, slot, rotX, rotY, scale);
}

const char* gTParticleEmitLName = "TParticleEmitL";

// 0x800266A4
TParticleEmitL::TParticleEmitL(TObject* parent, const ParticleDesc* desc, int slot, int rotX, int rotY, f32 scale)
    : TObject(parent)
{
	_00   = (u32)gTParticleEmitLName;
	mSize = 0xD4;
	mPart.init(desc, rotX, rotY, scale);
	mSlot  = slot;
	mLocal = mPart.mPos;
	mPart.setLocalPos(mSlot, &mLocal);
}

// 0x80026758
TParticleEmitL::~TParticleEmitL()
{
}

// 0x800267B4
void TParticleEmitL::draw()
{
	mPart.setLastAt(&mLocal);
	njAddVector(&mLocal, &mPart.mVel);
	mPart.setLocalPos(mSlot, &mLocal);
	if (mPart.step()) {
		mFlags |= 1;
	}
}

// 0x80026820
void TParticleEmitL::vfn_10()
{
	mPart.updateStretchAt(&mLocal);
}
