#include "stage/Stage.h"
#include "engine/Fx.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "engine/FxEmitter.h"
#include "engine/FxParticle.h"
#include "Ninja/ninja.h"
#include "stl/math.h"
#include "engine/FxSystem.h"
struct Vec3f;

#define FX_ANGLE 10430.38043493439
#define FX_PI 3.141592653589793
#define FX_TWO_PI 6.283185307179586
#define FX_HALF_PI 1.5707963267948966

static inline f32 fxRandom()
{
	return (f32)rand() * 3.0517578E-5f;
}

static inline f32 fxSqrtHeld(f32 x)
{
	volatile f32 y;
	if (x > 0.0f) {
		double g = __frsqrte(x);
		g = 0.5 * g * (3.0 - g * g * x);
		g = 0.5 * g * (3.0 - g * g * x);
		g = 0.5 * g * (3.0 - g * g * x);
		y = (f32)(x * g);
		return y;
	}
	return x;
}

static inline f32 fxSnap(f32 v)
{
	return v > 0.5f ? 1.0f : 0.0f;
}

/** A random number in [0 1) the constant first */
#define FX_RAND() (3.0517578E-5f * (f32)rand())

// 0x800E2814
// Starts the new particles of this frame in the emitter's own space
void fxSpawnLocal(FxEmitter* fx, FxEmitter* self)
{
	f32 speed;
	f32 left = self->mAccum;
	f32 lift;
	f32 angle;
	f32 step;
	f32 radius;
	if (left < 1.0f) {
		return;
	}
	f32 vec[3];
	f32 pos[3];
	vec[0] = self->mDir[0];
	vec[1] = self->mDir[1];
	vec[2] = self->mDir[2];
	speed = njUnitVector((Vec3f*)vec);
	if (speed > 1.0E-5) {
		f32 s, c, s1, c1;
		f32 a = atan2(vec[1], vec[2]);
		njSinCos((int)(FX_ANGLE * a), &s, &c);
		a = atan2(vec[0], vec[1] * s + vec[2] * c);
		njSinCos((int)(FX_ANGLE * a), &s1, &c1);
	}

	if (self->_A4 < 0.0f) {
		switch (self->mKind) {
		case 0: {
			f32 low = self->mShapeB;
			angle   = (self->mShapeA - low) / (int)self->mAccum * FX_RAND() + low;
			step    = (self->mShapeA - low) / (int)self->mAccum;
			break;
		}
		case 3:
		case 4: {
			f32 low = self->mShapeB;
			angle   = (self->mShapeA - low) / (int)self->mAccum * FX_RAND() + low;
			step    = (self->mShapeA - low) / (int)self->mAccum;
			break;
		}
		default:
			angle = 2.0 * (FX_PI * FX_RAND());
			step  = FX_TWO_PI / (int)self->mAccum;
			break;
		}
	}

	for (; left >= 1.0f; left -= 1.0f) {
		if ((fx->mFlags & 0x80) && (u32)self->mCount >= self->mMax) {
			break;
		}
		switch (self->mKind) {
		case 0:
		case 3:
		case 4: {
			if (self->_A0 < 0.0f) {
				radius = -self->_A0;
				lift   = 1.0f;
			} else {
				lift   = FX_RAND();
				radius = lift * self->_A0;
			}
			switch (self->mKind) {
			case 3:
				if (self->_A4 < 0.0f) {
					angle += step;
					lift = (FX_HALF_PI - (f32)atan2(self->mShapeC, radius)) - self->_A4;
				} else {
					f32 low = self->mShapeB;
					angle = (self->mShapeA - low) * FX_RAND() + low;
					f32 tilt = atan2(self->mShapeC, radius);
					lift  = self->_A4 + (FX_HALF_PI - tilt);
				}
				break;
			case 4:
				if (self->_A4 < 0.0f) {
					angle += step;
					lift = FX_HALF_PI - self->_A4;
				} else {
					f32 low = self->mShapeB;
					angle = (self->mShapeA - low) * FX_RAND() + low;
					lift  = FX_HALF_PI + self->_A4;
				}
				break;
			default:
				if (self->_A4 < 0.0f) {
					angle += step;
					lift *= -self->_A4;
				} else {
					f32 low = self->mShapeB;
					angle = (self->mShapeA - low) * FX_RAND() + low;
					lift *= self->_A4;
				}
				break;
			}
			njSinCos((int)(FX_ANGLE * angle), &pos[1], &pos[0]);
			pos[0] *= radius;
			pos[1] *= radius;
			if (self->mKind == 3 || self->mKind == 4) {
				pos[2] = FX_RAND();
				if (self->mKind == 3) {
					pos[0] *= 1.0f - pos[2];
					pos[1] *= 1.0f - pos[2];
				}
				pos[2] *= self->mShapeC;
			} else {
				pos[2] = 0.0f;
			}
			f32 s0, c0, s1, c1;
			njSinCos((int)(FX_ANGLE * lift), &s0, &c0);
			njSinCos((int)(FX_ANGLE * angle), &s1, &c1);
			s0 *= speed;
			vec[0] = c1 * s0;
			vec[1] = s1 * s0;
			vec[2] = c0 * speed;
			fxSpriteSpawn(self->mPartFlags, self->mPartLife, self->mPartScript, self, pos[0], pos[1], pos[2], vec[0], vec[1], vec[2], self->mGravity,
			            self->mDamp, self->mScale);
			break;
		}
		case 2: {
			pos[0] = FX_RAND();
			pos[1] = FX_RAND();
			pos[2] = FX_RAND();
			switch (*(u16*)&self->mShapeA) {
			case 1:
				pos[0] = fxSnap(pos[0]);
				break;
			case 2:
				pos[1] = fxSnap(pos[1]);
				break;
			case 3:
				if (FX_RAND() > self->mShapeB / (self->mShapeB + self->mShapeC)) {
					pos[1] = fxSnap(pos[1]);
				} else {
					pos[0] = fxSnap(pos[0]);
				}
				break;
			case 4:
				pos[2] = fxSnap(pos[2]);
				break;
			case 5:
				if (FX_RAND() > self->mShapeB / (self->mShapeB + self->mShapeD)) {
					pos[2] = fxSnap(pos[2]);
				} else {
					pos[0] = fxSnap(pos[0]);
				}
				break;
			case 6:
				if (FX_RAND() > self->mShapeC / (self->mShapeC + self->mShapeD)) {
					pos[2] = fxSnap(pos[2]);
				} else {
					pos[1] = fxSnap(pos[1]);
				}
				break;
			case 7: {
				f32 r = FX_RAND();
				f32 k = 1.0f / (self->mShapeB * (self->mShapeC + self->mShapeD) + self->mShapeC * self->mShapeD);
				if (r < k * (self->mShapeB * self->mShapeC)) {
					pos[2] = fxSnap(pos[2]);
				} else if (r > 1.0f - k * (self->mShapeB * self->mShapeD)) {
					pos[1] = fxSnap(pos[1]);
				} else {
					pos[0] = fxSnap(pos[0]);
				}
				break;
			}
			}
			pos[0] -= 0.5f;
			pos[1] -= 0.5f;
			pos[2] -= 0.5f;
			pos[0] *= self->mShapeB;
			pos[1] *= self->mShapeC;
			pos[2] *= self->mShapeD;
			vec[0] = 0.0f;
			vec[1] = 0.0f;
			vec[2] = speed;
			fxSpriteSpawn(self->mPartFlags, self->mPartLife, self->mPartScript, self, pos[0], pos[1], pos[2], vec[0], vec[1], vec[2], self->mGravity,
			            self->mDamp, self->mScale);
			break;
		}
		case 5: {
			f32 polar;
			if (0.0f == self->mShapeC) {
				polar = FX_PI * fxSqrtHeld(FX_RAND());
			} else {
				polar = self->mShapeC * fxSqrtHeld(FX_RAND());
			}
			f32 around = 2.0 * (FX_PI * FX_RAND());
			f32 radius = self->_A0;
			if (radius < 0.0f) {
				radius = -radius;
			} else {
				radius *= fxSqrtHeld(FX_RAND());
			}
			f32 s0, c0, s1, c1;
			njSinCos((int)(FX_ANGLE * polar), &s0, &c0);
			njSinCos((int)(FX_ANGLE * around), &s1, &c1);
			vec[0] = s0 * c1;
			vec[1] = s0 * s1;
			vec[2] = c0;
			pos[0] = s0 * c1;
			pos[1] = s0 * s1;
			pos[2] = c0;
			gjScaleVector(self->mShapeA, (Vec3f*)vec, (const Vec3f*)pos);
			if (self->_A0 >= 0.0f && self->mShapeA < 0.0f) {
				gjScaleVector(radius / self->_A0, (Vec3f*)vec, (const Vec3f*)vec);
			}
			pos[0] *= radius;
			pos[1] *= radius;
			pos[2] *= radius;
			fxSpriteSpawn(self->mPartFlags, self->mPartLife, self->mPartScript, self, pos[0], pos[1], pos[2], vec[0], vec[1], vec[2], self->mGravity,
			            self->mDamp, self->mScale);
			break;
		}
		}
	}
	self->mAccum = left;
}

// 0x800E359C
// Reads the shape fields from the description by kind
void fxEmitterReadShape(FxEmitter* fx, FxEmitterDesc* desc)
{
	switch (desc->mKind) {
	case 0:
		if (0.0f == desc->mShapeB && 0.0f == desc->mShapeC) {
			fx->mShapeB = 0.0f;
			fx->mShapeA = 6.2831855f;
		} else {
			fx->mShapeB = desc->mShapeB;
			fx->mShapeA = desc->mShapeC;
		}
		break;
	case 3:
	case 4:
		if (0.0f == desc->mShapeB && 0.0f == desc->mShapeC) {
			fx->mShapeB = 0.0f;
			fx->mShapeA = 6.2831855f;
		} else {
			fx->mShapeB = desc->mShapeB;
			fx->mShapeA = desc->mShapeC;
		}
		fx->mShapeC = desc->mShapeD;
		break;
	case 2:
		fx->mShapeB = desc->mShapeB;
		fx->mShapeC = desc->mShapeC;
		fx->mShapeD = desc->mShapeD;
		*(u16*)&fx->mShapeA = 0;
		if (desc->mShapeB < 0.0f) {
			*(u16*)&fx->mShapeA |= 1;
		}
		if (desc->mShapeC < 0.0f) {
			*(u16*)&fx->mShapeA |= 2;
		}
		if (desc->mShapeD < 0.0f) {
			*(u16*)&fx->mShapeA |= 4;
		}
		break;
	case 5:
		fx->mShapeA = njScalor((Vec3f*)fx->mDir);
		fx->mShapeB = atan2(fx->mDir[0], fxSqrtHeld(fx->mDir[0] * fx->mDir[0] + fx->mDir[2] * fx->mDir[2]));
		fx->mShapeC = desc->mShapeB;
		if (fx->mShapeC < 0.0f) {
			fx->mShapeC = -fx->mShapeC;
			fx->mShapeA = -fx->mShapeA;
		}
		break;
	}
}

// 0x800E37A0
// Takes a particle with no extra
FxParticle* fxPartAlloc()
{
	FxParticle* part = gFxParticleFreeList;
	if (part != NULL) {
		gFxParticleFreeList = part->mNext;
		part->mExtraCount = 0;
		part->mExtra = NULL;
		gFxParticleCount++;
	}
	return part;
}

// 0x800E37D0
// Takes a particle with an extra or nothing if there are none
FxParticle* fxPartAllocExtra()
{
	FxParticle* part;
	if ((part = gFxParticleFreeList) != NULL) {
		part->mExtraCount = 0;
		part->mExtra = NULL;
		if (fxExtraAlloc(part) == NULL) {
			return NULL;
		}
		gFxParticleFreeList = part->mNext;
		gFxParticleCount++;
	}
	return part;
}

// 0x800E383C
void fxPartFree(FxParticle* part)
{
	if (part->mExtra != NULL) {
		part->mExtra->mPrev->mNext = gFxExtraFreeList;
		gFxExtraFreeList = part->mExtra;
		gFxParticleExtraCount -= part->mExtraCount;
	}
	part->mNext = gFxParticleFreeList;
	gFxParticleFreeList = part;
	gFxParticleCount--;
}

// 0x800E3888
// Frees a whole list
void fxPartFreeList(FxParticle* list)
{
	FxParticle* part;
	FxParticle* last;

	last = list;
	part = list;
	while (part != NULL) {
		if (part->mExtra != NULL) {
			part->mExtra->mPrev->mNext = gFxExtraFreeList;
			gFxExtraFreeList = part->mExtra;
			gFxParticleExtraCount -= part->mExtraCount;
		}
		gFxParticleCount--;
		last = part;
		part = part->mNext;
	}
	last->mNext = gFxParticleFreeList;
	gFxParticleFreeList = list;
}

// 0x800E38F0
FxExtra* fxExtraAlloc(FxParticle* part)
{
	FxExtra* extra = gFxExtraFreeList;
	if (extra != NULL) {
		gFxExtraFreeList = extra->mNext;
		FxExtra* head = part->mExtra;
		if (head == NULL) {
			extra->mPrev = extra;
			part->mExtra = extra;
			extra->mNext = NULL;
		} else {
			extra->mPrev = head->mPrev;
			extra->mNext = part->mExtra;
			part->mExtra->mPrev = extra;
			part->mExtra = extra;
		}
		part->mExtraCount++;
		gFxParticleExtraCount++;
	}
	return extra;
}

// 0x800E3960
void fxExtraFree(FxParticle* part, FxExtra* extra)
{
	if (extra == NULL) {
		return;
	}
	if (extra->mPrev == extra) {
		part->mExtra = NULL;
	} else if (part->mExtra == extra) {
		part->mExtra = extra->mNext;
		if (extra->mNext != NULL) {
			extra->mNext->mPrev = extra->mPrev;
		}
	} else {
		extra->mPrev->mNext = extra->mNext;
		if (extra->mNext == NULL) {
			part->mExtra->mPrev = extra->mPrev;
		} else {
			extra->mNext->mPrev = extra->mPrev;
		}
	}
	part->mExtraCount--;
	extra->mNext = gFxExtraFreeList;
	gFxExtraFreeList = extra;
	gFxParticleExtraCount--;
}

// 0x800E3A00
// The colour now part way between the two
void fxPartColor(FxParticle* part, u8* out)
{
	if (part->mColorFrames != 0) {
		int t = (part->mColorLeft << 16) / part->mColorFrames;
		out[0] = ((part->mColorTo[0] << 16) + t * (part->mColor[0] - part->mColorTo[0])) >> 16;
		out[1] = ((part->mColorTo[1] << 16) + t * (part->mColor[1] - part->mColorTo[1])) >> 16;
		out[2] = ((part->mColorTo[2] << 16) + t * (part->mColor[2] - part->mColorTo[2])) >> 16;
	} else {
		out[0] = part->mColor[0];
		out[1] = part->mColor[1];
		out[2] = part->mColor[2];
	}
	if (part->mAlphaFrames != 0) {
		int t = (part->mAlphaLeft << 16) / part->mAlphaFrames;
		out[3] = ((part->mColorTo[3] << 16) + t * (part->mColor[3] - part->mColorTo[3])) >> 16;
	} else {
		out[3] = part->mColor[3];
	}
}

// 0x800E3ADC
// Turns the velocity by a random spin around itself at the given angle away from it
void fxPartDeflect(FxParticle* part, int angle)
{
	f32 vx;
	f32 vy = part->mVel[1];
	f32 vz;
	vx = part->mVel[0];
	vz = part->mVel[2];
	f32 s0, c0, s1, c1, s2, c2, s3, c3;
	f32 a;
	a = atan2(vy, vz);
	njSinCos((int)(FX_ANGLE * a), &s0, &c0);
	a = atan2(vx, vy * s0 + vz * c0);
	njSinCos((int)(FX_ANGLE * a), &s1, &c1);
	f32 speed = njScalor((Vec3f*)part->mVel);
	njSinCos((int)(65536.0f * fxRandom()), &s2, &c2);
	njSinCos(angle, &s3, &c3);
	f32 sx;
	f32 t = s3 * speed;
	f32 sy = s2 * t;
	sx = c2 * t;
	f32 fw = c3 * speed;
	part->mVel[0] = sx * c1 + fw * s1;
	part->mVel[1] = c1 * (fw * s0) + (s1 * (-sx * s0) + sy * c0);
	part->mVel[2] = c1 * (fw * c0) + (s1 * (-sx * c0) - sy * s0);
}
