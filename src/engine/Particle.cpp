#include "engine/ParticleInternal.h"

// 0x800277C0
void Particle::init(const ParticleDesc* desc, f32 scale)
{
	mDesc  = desc;
	mFlags = 0;
	particleRandomOffset(desc, &mPos);
	mColourNow = *(const u32*)desc->mColour;
	mRot       = 0;
	mTex       = *(const u32*)&desc->mTex;
	mFrame     = 0;
	setSpriteScale(scale);
	mColour = *(const u32*)desc->mColour;

	mBaseX  = desc->mScaleXRange * partRand() + desc->mScaleX;
	mBaseY  = desc->mScaleYRange * partRand() + desc->mScaleY;
	mScaleX = mBaseX;
	mScaleY = mBaseY;

	mFadeIn = desc->mFadeIn;
	if (mFadeIn > 0.0f) {
		mFlags |= PART_Fading;
		mFadeInScale = 1.0f / mFadeIn;
	} else {
		mFadeInScale = 0.0f;
	}
	mFadeOut = desc->mFadeOut;
	if (mFadeOut > 0.0f) {
		mFlags |= PART_Fading;
		mFadeOutScale = 1.0f / mFadeOut;
	} else {
		mFadeOutScale = 0.0f;
	}

	if (desc->mFlags & (PARTDESC_StretchX | PARTDESC_StretchY)) {
		mFlags |= PART_Stretch;
		mLast = mPos;
	}
	if (desc->mSpin != 0) {
		if ((desc->mFlags & PARTDESC_SpinAny) && partRand() > 0.5f) {
			mFlags |= PART_SpinBack;
		}
		mFlags |= PART_Spin;
	}
	mRot  = desc->mRot;
	mRotX = 0;
	mRotY = 0;

	mAnimRate = (s16)(int)(256.0f * desc->mAnimRate);
	if (getSpriteAnim(&desc->mTex.mMode) == 2) {
		int frame = (int)(16.0f * partRand());
		if (frame >= 0x10) {
			frame = 0xF;
		}
		mAnimPhase = frame << 8;
	} else {
		mAnimPhase = 0;
	}
	mAnimPhase -= mAnimRate;

	int life;
	if (getSpriteAnim(&desc->mTex.mMode) == 1) {
		life = (int)(16.0f / desc->mAnimRate);
	} else {
		life = desc->mLife;
	}
	mAge     = 0.0f;
	mAgeStep = 1.0f / (f32)life;

	if (desc->mFlags & PARTDESC_Fade) {
		const u8* from = desc->mColour;
		const u8* to   = desc->mColourEnd;
		for (int i = 0; i < 3; i++) {
			int c    = *from++;
			int diff = *to++ - c;
			if (diff != 0) {
				mFlags |= PART_Colour;
				mColourStep[i] = (diff << 8) / life;
			} else {
				mColourStep[i] = 0;
			}
			mColourValue[i] = c << 8;
		}
	} else {
		for (int i = 0; i < 3; i++) {
			mColourStep[i]  = 0;
			mColourValue[i] = 0;
		}
	}
}

// 0x80027B80
void Particle::setSpriteScale(f32 scale)
{
	mSpriteScale = scale;
}

// 0x80027B88
f32 Particle::getFade()
{
	f32 in   = 1.0f;
	f32 out  = 1.0f;
	f32 left = 1.0f - mAge;
	if (mFadeIn > mAge) {
		in = mAge * mFadeInScale;
	}
	if (left < mFadeOut) {
		out = left * mFadeOutScale;
	}
	if (in < out) {
		return in;
	}
	return out;
}

// 0x80027BD0
void Particle::setAlpha(f32 alpha)
{
	u32 keep = mColour;
	f32 a    = (f32)(mColour & 0xFF);
	a *= alpha;
	keep &= 0xFFFFFF00;
	mColourNow = keep | ((u32)a & 0xFF);
}

// 0x80027C34
void Particle::updateScale()
{
	if (mFlags & PART_Stretch) {
		return;
	}
	f32 damp = mDesc->mScaleDamp;
	mScaleX *= damp;
	mScaleY *= damp;
	if (mFlags & PART_Spin) {
		spin();
	}
}

// 0x80027C8C
void Particle::updateColour()
{
	f32 alpha = 1.0f;
	if (mFlags & PART_Fading) {
		alpha = getFade();
	}
	if (mFlags & PART_Colour) {
		u32 colour = 0;
		for (int i = 0; i < 3; i++) {
			u32 value = mColourValue[i];
			value += mColourStep[i];
			value           = value > 0xFF00 ? 0xFF00 : value;
			mColourValue[i] = value;
			colour <<= 8;
			colour |= value & 0xFF00;
		}
		if (alpha != 1.0f) {
			f32 a = (f32)(mColour & 0xFF);
			a *= alpha;
			colour     = __rlwimi(colour, (u32)a, 0, 24, 31);
			mColourNow = colour;
		} else {
			colour |= mColour & 0xFF;
			mColourNow = colour;
		}
	} else if (alpha != 1.0f) {
		setAlpha(alpha);
	}
}

// 0x80027D8C
void Particle::spin()
{
	if (mFlags & PART_SpinBack) {
		mRot -= mDesc->mSpin;
	} else {
		mRot += mDesc->mSpin;
	}
}

// 0x80027DC8
void Particle::updateFrame()
{
	if (getSpriteAnim(&mDesc->mTex.mMode) != 0) {
		mAnimPhase += mAnimRate;
		mFrame = (mAnimPhase >> 8) & 0xF;
	}
}

// 0x80027E20
int Particle::advance()
{
	mAge += mAgeStep;
	return mAge >= 1.0f;
}

// 0x80027E4C
void Particle::updateStretch()
{
	updateStretchAt(&mPos);
}

// 0x80027E70
void Particle::updateStretchAt(const Vec3f* pos)
{
	if (!(mFlags & PART_Stretch)) {
		return;
	}
	Vec3f move;
	move = *pos;
	njSubVector(&move, &mLast);
	f32 len = njUnitVector(&move);
	if (len < gUnitVectorMin) {
		move = gVecZero;
	}
	njCalcVector((f32*)0, (const Vec3f*)(const NJS_VECTOR*)&move, (Vec3f*)(NJS_VECTOR*)&move);
	mRot = -(int)(10430.38043493439 * (f32)atan2(move.x, move.y));
	if (mDesc->mFlags & PARTDESC_StretchX) {
		mScaleX = len * mBaseX;
	}
	if (mDesc->mFlags & PARTDESC_StretchY) {
		mScaleY = len * mBaseY;
	}
}

// 0x80027F80
void Particle::addPos(const Vec3f* move)
{
	njAddVector(&mPos, move);
}

// 0x80027FA4
void Particle::setLocalPos(int slot, const Vec3f* local)
{
	mPos = *local;
	particlePosSlotAdd(slot, &mPos);
}

// 0x80027FE8
void Particle::setLastAt(const Vec3f* pos)
{
	mLast = *pos;
}

// 0x80028004
void Particle::setLast()
{
	mLast = mPos;
}
