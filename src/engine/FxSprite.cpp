#include "stage/Stage.h"
#include "engine/Fx.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "engine/FxSystem.h"
#include "engine/FxAnim.h"
#include "engine/FxEmitter.h"
#include "engine/FxParticle.h"
#include "engine/FxParticle2.h"
#include "engine/FxSprite.h"
#include "stl/math.h"
#include "Ninja/ninja.h"
#include "engine/FxShape.h"
#include "io/NinjaFile.h"
#include "core/MathUtil.h"
struct FxEmitter;
struct FxShape;

#define FX_PIPE    (*(volatile f32*)0xCC008000)
#define FX_PIPE8   (*(volatile u8*)0xCC008000)
#define FX_ANGLE   10430.38043493439
#define FX_PI      3.141592653589793
#define FX_TWO_PI  6.283185307179586
#define FX_HALF_PI 1.5707963267948966

static inline f32 fxRandom()
{
	return (f32)rand() * 3.0517578E-5f;
}

static inline f32 fxSqrt(f32 x)
{
	if (x > 0.0f) {
		double g = __frsqrte(x);
		g        = 0.5 * g * (3.0 - g * g * x);
		g        = 0.5 * g * (3.0 - g * g * x);
		g        = 0.5 * g * (3.0 - g * g * x);
		return (f32)(x * g);
	}
	return x;
}

static inline f32 fxSnap(f32 v)
{
	return v > 0.5f ? 1.0f : 0.0f;
}

// 0x800E0938
// Starts the new particles of this frame somewhere in the emitter's shape
void fxSpawnInWorld(FxEmitter* fx, FxEmitter* self, FxSpawnFn spawn)
{
	f32 tilt0 = 0.0f;
	f32 tilt1 = 0.0f;
	f32 left  = self->mAccum;
	if (left < 1.0f) {
		return;
	}
	f32 dir[3];
	dir[0]    = self->mDir[0];
	dir[1]    = self->mDir[1];
	dir[2]    = self->mDir[2];
	f32 speed = njUnitVector((Vec3f*)dir);
	f32 m[3][4];
	if (fx->mFollow != NULL) {
		PSMTXCopy((const float*)fx->mFollow, (float*)m);
	} else {
		PSMTXCopy((const float*)fx->mMatrix, (float*)m);
	}
	if (speed > 1.0E-5) {
		f32 s0, c0, s1, c1;
		f32 a = atan2(dir[1], dir[2]);
		njSinCos((int)(FX_ANGLE * a), &s0, &c0);
		a = atan2(dir[0], dir[1] * s0 + dir[2] * c0);
		njSinCos((int)(FX_ANGLE * a), &s1, &c1);
		f32 r[3][4];
		r[0][0] = c1;
		r[0][1] = 0.0f;
		r[0][2] = s1;
		r[0][3] = 0.0f;
		r[1][0] = -s0 * s1;
		r[1][1] = c0;
		r[1][2] = s0 * c1;
		r[1][3] = 0.0f;
		r[2][0] = -c0 * s1;
		r[2][1] = -s0;
		r[2][2] = c0 * c1;
		r[2][3] = 0.0f;
		PSMTXConcat((const float*)m, (const float*)r, (float*)m);
	}
	if (self->mKind == 6) {
		f32 s, c;
		tilt0 = atan2(m[1][2], m[2][2]);
		njSinCos((int)(FX_ANGLE * tilt0), &s, &c);
		tilt1 = atan2(m[0][2], m[1][2] * s + m[2][2] * c);
	}

	f32 step  = 0.0f;
	f32 angle = 0.0f;
	if (self->_A4 < 0.0f) {
		if (self->mKind == 0 || self->mKind == 3 || self->mKind == 4) {
			f32 low = self->mShapeB;
			step    = (self->mShapeA - low) / (f32)(int)left;
			angle   = step * fxRandom() + low;
		} else {
			step  = FX_TWO_PI / (f32)(int)left;
			angle = 2.0 * (FX_PI * fxRandom());
		}
	}

	f32 pos[3];
	f32 vel[3];
	for (; left >= 1.0f; left -= 1.0f) {
		if ((fx->mFlags & 0x80) && self->mCount >= self->mMax) {
			break;
		}
		switch (self->mKind) {
		case 0:
		case 3:
		case 4: {
			f32 radius;
			f32 lift;
			if (self->_A0 < 0.0f) {
				radius = -self->_A0;
				lift   = 1.0f;
			} else {
				lift   = fxRandom();
				radius = lift * self->_A0;
			}
			if (self->mKind == 3) {
				if (self->_A4 < 0.0f) {
					angle += step;
					lift = (FX_HALF_PI - atan2(self->mShapeC, radius)) - self->_A4;
				} else {
					f32 low = self->mShapeB;
					angle   = (self->mShapeA - low) * fxRandom() + low;
					lift    = self->_A4 + (FX_HALF_PI - atan2(self->mShapeC, radius));
				}
			} else if (self->mKind == 4) {
				if (self->_A4 < 0.0f) {
					angle += step;
					lift = FX_HALF_PI - self->_A4;
				} else {
					f32 low = self->mShapeB;
					angle   = (self->mShapeA - low) * fxRandom() + low;
					lift    = FX_HALF_PI + self->_A4;
				}
			} else if (self->_A4 < 0.0f) {
				angle += step;
				lift *= -self->_A4;
			} else {
				f32 low = self->mShapeB;
				angle   = (self->mShapeA - low) * fxRandom() + low;
				lift *= self->_A4;
			}
			f32 turn = FX_ANGLE * angle;
			njSinCos((int)turn, &pos[1], &pos[0]);
			pos[0] *= radius;
			pos[1] *= radius;
			if (self->mKind == 3 || self->mKind == 4) {
				pos[2] = fxRandom();
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
			njSinCos((int)turn, &s1, &c1);
			s0 *= speed;
			vel[0] = c1 * s0;
			vel[1] = s1 * s0;
			vel[2] = c0 * speed;
			PSMTXMultVec((const float*)m, (const NJS_VECTOR*)pos, (NJS_VECTOR*)pos);
			PSMTXMultVecSR((const float*)m, (const NJS_VECTOR*)vel, (NJS_VECTOR*)vel);
			spawn(self->mPartFlags, self->mPartLife, self->mPartScript, self, pos[0], pos[1], pos[2], vel[0], vel[1], vel[2],
			      self->mGravity, self->mDamp, self->mScale);
			break;
		}
		case 2: {
			pos[0] = fxRandom();
			pos[1] = fxRandom();
			pos[2] = fxRandom();
			f32 x  = self->mShapeB;
			f32 y  = self->mShapeC;
			f32 z  = self->mShapeD;
			switch (*(s16*)&self->mShapeA) {
			case 1:
				pos[0] = fxSnap(pos[0]);
				break;
			case 2:
				pos[1] = fxSnap(pos[1]);
				break;
			case 3:
				if (fxRandom() <= x / (x + y)) {
					pos[0] = fxSnap(pos[0]);
				} else {
					pos[1] = fxSnap(pos[1]);
				}
				break;
			case 4:
				pos[2] = fxSnap(pos[2]);
				break;
			case 5:
				if (fxRandom() <= x / (x + z)) {
					pos[0] = fxSnap(pos[0]);
				} else {
					pos[2] = fxSnap(pos[2]);
				}
				break;
			case 6:
				if (fxRandom() <= y / (y + z)) {
					pos[1] = fxSnap(pos[1]);
				} else {
					pos[2] = fxSnap(pos[2]);
				}
				break;
			case 7: {
				f32 k = 1.0f / (x * (y + z) + y * z);
				f32 r = fxRandom();
				if (r < k * (x * y)) {
					pos[2] = fxSnap(pos[2]);
				} else if (r <= 1.0f - k * (x * z)) {
					pos[0] = fxSnap(pos[0]);
				} else {
					pos[1] = fxSnap(pos[1]);
				}
				break;
			}
			}
			pos[0] = (pos[0] - 0.5f) * x;
			pos[1] = (pos[1] - 0.5f) * y;
			pos[2] = (pos[2] - 0.5f) * z;
			PSMTXMultVec((const float*)m, (const NJS_VECTOR*)pos, (NJS_VECTOR*)pos);
			vel[0] = 0.0f;
			vel[1] = 0.0f;
			vel[2] = speed;
			PSMTXMultVecSR((const float*)m, (const NJS_VECTOR*)vel, (NJS_VECTOR*)vel);
			spawn(self->mPartFlags, self->mPartLife, self->mPartScript, self, pos[0], pos[1], pos[2], vel[0], vel[1], vel[2],
			      self->mGravity, self->mDamp, self->mScale);
			break;
		}
		case 5: {
			f32 polar;
			if (0.0f == self->mShapeC) {
				polar = FX_PI * fxSqrt(fxRandom());
			} else {
				polar = self->mShapeC * fxSqrt(fxRandom());
			}
			f32 around = 2.0 * (FX_PI * fxRandom());
			f32 radius = self->_A0;
			if (radius < 0.0f) {
				radius = -radius;
			} else {
				radius *= fxSqrt(fxRandom());
			}
			f32 s0, c0, s1, c1;
			njSinCos((int)(FX_ANGLE * polar), &s0, &c0);
			njSinCos((int)(FX_ANGLE * around), &s1, &c1);
			vel[2] = c0;
			vel[0] = s0 * c1;
			vel[1] = s0 * s1;
			PSMTXMultVecSR((const float*)m, (const NJS_VECTOR*)vel, (NJS_VECTOR*)pos);
			gjScaleVector(self->mShapeA, (Vec3f*)vel, (const Vec3f*)pos);
			if (self->_A0 >= 0.0f && self->mShapeA < 0.0f) {
				gjScaleVector(radius / self->_A0, (Vec3f*)vel, (const Vec3f*)vel);
			}
			pos[0] = radius * pos[0] + m[0][3];
			pos[1] = radius * pos[1] + m[1][3];
			pos[2] = radius * pos[2] + m[2][3];
			spawn(self->mPartFlags, self->mPartLife, self->mPartScript, self, pos[0], pos[1], pos[2], vel[0], vel[1], vel[2],
			      self->mGravity, self->mDamp, self->mScale);
			break;
		}
		case 6: {
			f32 radius = self->_A0 < 0.0f ? 1.0f : fxRandom();
			f32 at;
			if (self->_A4 < 0.0f) {
				at = angle + step;
			} else {
				at = 2.0 * (FX_PI * fxRandom());
			}
			self->mShapeA = speed;
			angle         = at;
			spawn(self->mPartFlags | 0x200, self->mPartLife, self->mPartScript, self, 0.0f, 0.0f, 0.0f, at, radius, 0.0f, tilt0, tilt1,
			      self->mScale);
			break;
		}
		}
	}
	self->mAccum = left;
}

// 0x800E193C
void fxSpriteUpdate(FxEmitter* fx)
{
	if (fx->mRate < 0.0f) {
		fx->mAccum -= fx->mRate;
	} else {
		fx->mAccum += fx->mRate * fxRandom();
	}
	fxSpawnInWorld(fx, fx, fxSpriteSpawn);
	if (fx->mStop != 0 && --fx->mStop == 0) {
		fxEmitterStop(fx, fx);
	}
	fxStepParticles(fx, fxSpritePartStep);
}

// 0x800E1A08
// Where a particle on a spinning cone is now
void fxConePos(FxParticle* part, f32* x, f32* y, f32* z)
{
	FxEmitter* owner = part->mOwner;
	f32 px, py;
	f32 s0, s1, c0, c1;
	njSinCos((int)(FX_ANGLE * part->mGravity), &s0, &c0);
	njSinCos((int)(FX_ANGLE * part->mDamp), &s1, &c1);
	f64 tilt   = fabs(owner->_A4);
	f32 height = part->mVel[2] - owner->mShapeA;
	f32 radius = fabs(owner->_A0);
	f32 around = part->mVel[0] - owner->mGravity;
	radius     = (height * (njSin((int)(FX_ANGLE * (f32)tilt)) / njCos((int)(FX_ANGLE * (f32)tilt))) + radius) * part->mVel[1];
	njSinCos((int)(FX_ANGLE * around), &py, &px);
	px *= radius;
	py *= radius;
	const f32* m = owner->mFollow != NULL ? owner->mFollow : owner->mMatrix;
	*x           = m[3] + (px * c1 + height * s1);
	*y           = m[7] + (c1 * (height * s0) + (s1 * (-px * s0) + py * c0));
	*z           = m[11] + (c1 * (height * c0) + (s1 * (-px * c0) - py * s0));
}

// 0x800E1C24
void fxSpriteMaterial(Fx* fx, FxEmitter* self, int flags)
{
	FxTexUse uses[3];
	FxMaterialRef ref;
	ref.mMaterial = self->mMaterial;
	ref.mTexGroup = self->mTexGroup;
	ref.mTexUses  = uses;
	int i         = 0;
	while (i < 3) {
		uses[i].mTex   = self->mTex[i];
		uses[i].mFlags = flags;
		i++;
	}
	fxSetMaterial(fx, &ref);
	fxLoadTexMatrices(fx, self->mMaterial, (const f32*)gjIdentityMatrix, (f32*)sFxSpriteTexMtx);
}

// 0x800E1CC4
// Camera-facing quads skipping anything too close
void fxSpriteDraw(Fx* fx, FxEmitter* self)
{
	u8* state = gFxRenderState;
	if (self->mCount == 0) {
		return;
	}
	fxSetCurrentMtx(3);
	fxSetCull(2);
	fxSpriteMaterial(fx, self, 9);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	fxSetVtxLayout(0);
	if (gFxRenderReady != 0) {
		gFxRenderReady = 0;
		int* in        = (int*)(state + 0x260);
		GXSetTevColorIn(GX_TEVSTAGE0, in[0], in[1], in[2], in[3]);
		in = (int*)(state + 0x280);
		GXSetTevAlphaIn(GX_TEVSTAGE0, in[0], in[1], in[2], in[3]);
	}
	fxSetCoordArray(gFxCoordArray, 8);
	for (FxParticle* part = self->mKids; part != NULL; part = part->mNext) {
		f32 view[3];
		PSMTXMultVec((const float*)state, (const NJS_VECTOR*)part->mPos, (NJS_VECTOR*)view);
		f32 z = view[2];
		if (z > -50.0f) {
			continue;
		}
		f32 size = 0.5f * part->mScale;
		f32 x    = view[0];
		f32 y    = view[1];
		f32 ax   = size;
		f32 bx;
		f32 by;
		f32 ay;
		int turn;
		f32 move[3];
		bx = size;
		ay = size;
		by = -size;
		if ((part->mFlags & 0x400) || (part->mFlags & 0x800)) {
			if (part->mFlags & 0x200) {
				fxConePos(part, &move[0], &move[1], &move[2]);
				gjSubVector((Vec3f*)move, (const Vec3f*)part->mPos, (const Vec3f*)move);
				PSMTXMultVecSR((const float*)state, (const NJS_VECTOR*)move, (NJS_VECTOR*)move);
			} else {
				PSMTXMultVecSR((const float*)state, (const NJS_VECTOR*)part->mVel, (NJS_VECTOR*)move);
			}
			turn = (int)(FX_ANGLE * (f32)atan2(move[0], -move[1]));
		} else {
			turn = (int)(FX_ANGLE * part->mRot);
		}
		u8 color[4];
		fxPartColor(part, color);
		u32 tev = *(u32*)color;
		fxSetTevColor((const u8*)&tev);
		u8 uv[4];
		fxCellCoords(part->_05, part->_08, uv);
		if (turn != 0) {
			f32 s, c;
			njSinCos(turn, &s, &c);
			ax = by * s + size * c;
			by = by * c - size * s;
			bx = (size * c) - (size * s);
			ay = (size * s) + (size * c);
		}
		if (part->mFlags & 0x400) {
			f32 mx = -move[0];
			f32 my = -move[1];
			GXBegin(GX_QUADS, GX_VTXFMT7, 4);
			FX_PIPE  = x - bx;
			FX_PIPE  = y - ay;
			FX_PIPE  = z;
			FX_PIPE8 = uv[0];
			FX_PIPE  = mx + (x + by);
			FX_PIPE  = my + (y + ax);
			FX_PIPE  = z;
			FX_PIPE8 = uv[2];
			FX_PIPE  = mx + (x + bx);
			FX_PIPE  = my + (y + ay);
			FX_PIPE  = z;
			FX_PIPE8 = uv[3];
			FX_PIPE  = x - by;
			FX_PIPE  = y - ax;
			FX_PIPE  = z;
			FX_PIPE8 = uv[1];
		} else {
			GXBegin(GX_QUADS, GX_VTXFMT7, 4);
			FX_PIPE  = x - bx;
			FX_PIPE  = y - ay;
			FX_PIPE  = z;
			FX_PIPE8 = uv[0];
			FX_PIPE  = x + by;
			FX_PIPE  = y + ax;
			FX_PIPE  = z;
			FX_PIPE8 = uv[2];
			FX_PIPE  = x + bx;
			FX_PIPE  = y + ay;
			FX_PIPE  = z;
			FX_PIPE8 = uv[3];
			FX_PIPE  = x - by;
			FX_PIPE  = y - ax;
			FX_PIPE  = z;
			FX_PIPE8 = uv[1];
		}
	}
}

// 0x800E20FC
void fxSpriteStub(Fx*, Fx*)
{
}

// 0x800E2100
void fxSpriteDrawKind(Fx* fx)
{
	FxEmitter* self = (FxEmitter*)fx;
	if (fx->mFlags & 0x100) {
		fxSpriteStub(fx, self);
	} else {
		fxSpriteDraw(fx, self);
	}
}

// 0x800E2138
// A kind with nothing behind it
void* fxBankNullKind(FxBank*, int)
{
	return NULL;
}

// 0x800E2140
static inline void fxEmitterReadDesc(FxEmitter* fx, FxEmitterDesc* desc)
{
	fx->mRate       = desc->mRate;
	fx->mAccum      = 0.0f;
	fx->mMax        = desc->mMax;
	fx->mPartLife   = desc->mPartLife;
	fx->mPartFlags  = desc->mPartFlags;
	fx->mTex[0]     = desc->mTex[0];
	fx->mTex[1]     = desc->mTex[1];
	fx->mTexGroup   = desc->mTexGroup;
	fx->mMaterial   = desc->mMaterial;
	fx->mDir[0]     = desc->mDir[0];
	fx->mDir[1]     = desc->mDir[1];
	fx->mDir[2]     = desc->mDir[2];
	fx->mGravity    = desc->mGravity;
	fx->mDamp       = desc->mDamp;
	fx->mScale      = desc->mScale;
	fx->mPartScript = desc->mPartScript;
	fx->mCount      = 0;
	fx->mKids       = NULL;
	fx->_A0         = desc->_14;
	fx->_A4         = desc->_18;
	fxEmitterReadShape(fx, desc);
}

FxEmitter* fxStreakNew(FxBank* bank, int index)
{
	FxEmitter* fx = (FxEmitter*)fxEmitterAlloc();
	if (fx == NULL) {
		return NULL;
	}
	FxEmitterDesc* desc = (FxEmitterDesc*)bank->mDescs[index * 2 + 1];
	Fx* base            = fx;
	fx->mUpdate         = (void (*)(Fx*))fxStreakUpdate;
	fx->mDraw           = (void (*)(Fx*))fxStreakDraw;
	fx->mRemove         = (void (*)(Fx*))fxEmitterFree;
	fx->mFlags |= desc->mFlags;
	fx->mKind = desc->mKind;
	fx->mStop = desc->mLife != 0 ? desc->mLife : 0xFFFF;
	fxEmitterReadDesc((FxEmitter*)base, desc);
	return fx;
}

// 0x800E2290
void fxStreakUpdate(FxEmitter* fx)
{
	if (fx->mRate < 0.0f) {
		fx->mAccum -= fx->mRate;
	} else {
		fx->mAccum += fx->mRate * fxRandom();
	}
	fxSpawnLocal(fx, fx);
	if (fx->mStop != 0 && --fx->mStop == 0) {
		fxEmitterStop(fx, fx);
	}
	fxStepParticles(fx, fxStreakPartStep);
}

// 0x800E2354
// Quads stretched along the particle's movement
void fxStreakDraw(FxEmitter* fx)
{
	u8* state = gFxRenderState;
	if (fx->mCount == 0) {
		return;
	}
	fxSetCurrentMtx(3);
	fxSetCull(2);
	fxSpriteMaterial(fx, fx, 9);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	f32 m[3][4];
	if (fx->mFollow != NULL) {
		PSMTXConcat((const float*)state, (const float*)fx->mFollow, (float*)m);
	} else {
		PSMTXConcat((const float*)state, (const float*)fx->mMatrix, (float*)m);
	}
	fxSetVtxLayout(6);
	if (gFxRenderReady != 1) {
		gFxRenderReady = 1;
		int* in        = (int*)(state + 0x270);
		GXSetTevColorIn(GX_TEVSTAGE0, in[0], in[1], in[2], in[3]);
		in = (int*)(state + 0x290);
		GXSetTevAlphaIn(GX_TEVSTAGE0, in[0], in[1], in[2], in[3]);
	}
	fxSetCoordArray(gFxCoordArray, 8);
	for (FxParticle* part = fx->mKids; part != NULL; part = part->mNext) {
		f32 a[3];
		f32 b[3];
		PSMTXMultVec((const float*)m, (const NJS_VECTOR*)part->mPos, (NJS_VECTOR*)a);
		if (a[2] > -10.0f) {
			continue;
		}
		gjAddVector((Vec3f*)b, (const Vec3f*)part->mPos, (const Vec3f*)part->mVel);
		PSMTXMultVec((const float*)m, (const NJS_VECTOR*)b, (NJS_VECTOR*)b);
		f32 s, c;
		njSinCos((int)(FX_ANGLE * atan2(b[1] - a[1], b[0] - a[0])) + 0x4000, &s, &c);
		f32 ex, ey;
		if (part->mFlags & 2) {
			f32 length = njScalor((Vec3f*)part->mVel);
			c *= part->mScale;
			s *= part->mScale;
			ex = (c * (length + part->mDamp)) / part->mDamp;
			ey = (s * (length + part->mDamp)) / part->mDamp;
		} else {
			c *= part->mScale;
			ex = c;
			ey = s * part->mScale;
			s  = ey;
		}
		u8 uv[4];
		u8 color[4];
		fxCellCoords(part->_05, part->_08, uv);
		fxPartColor(part, color);
		GXBegin(GX_QUADS, GX_VTXFMT7, 4);
		FX_PIPE  = a[0] - c;
		FX_PIPE  = a[1] - s;
		FX_PIPE  = a[2];
		FX_PIPE8 = color[0];
		FX_PIPE8 = color[1];
		FX_PIPE8 = color[2];
		FX_PIPE8 = color[3];
		FX_PIPE8 = uv[2];
		FX_PIPE  = a[0] + c;
		FX_PIPE  = a[1] + s;
		FX_PIPE  = a[2];
		FX_PIPE8 = color[0];
		FX_PIPE8 = color[1];
		FX_PIPE8 = color[2];
		FX_PIPE8 = color[3];
		FX_PIPE8 = uv[3];
		FX_PIPE  = b[0] + ex;
		FX_PIPE  = b[1] + ey;
		FX_PIPE  = b[2];
		FX_PIPE8 = color[0];
		FX_PIPE8 = color[1];
		FX_PIPE8 = color[2];
		FX_PIPE8 = color[3];
		FX_PIPE8 = uv[1];
		FX_PIPE  = b[0] - ex;
		FX_PIPE  = b[1] - ey;
		FX_PIPE  = b[2];
		FX_PIPE8 = color[0];
		FX_PIPE8 = color[1];
		FX_PIPE8 = color[2];
		FX_PIPE8 = color[3];
		FX_PIPE8 = uv[0];
	}
}

// 0x800E26EC
void fxStreakPartStep(FxParticle* part)
{
	if (part->mColorFrames != 0) {
		if (--part->mColorLeft == 0) {
			part->mColorFrames = 0;
			part->mColor[0]    = part->mColorTo[0];
			part->mColor[1]    = part->mColorTo[1];
			part->mColor[2]    = part->mColorTo[2];
		}
	}
	if (part->mAlphaFrames != 0) {
		if (--part->mAlphaLeft == 0) {
			part->mAlphaFrames = 0;
			part->mColor[3]    = part->mColorTo[3];
		}
	}
	if (part->mScaleFrames != 0) {
		part->mScale += (part->mScaleTo - part->mScale) / (f32)part->mScaleFrames;
		part->mScaleFrames--;
	}
	fxPartRunScript(part);
	if (part->mLife != 0 && --part->mLife == 0) {
		part->mFlags |= 0x100;
	}
	if (part->mStep != NULL) {
		part->mStep(part);
	}
}
