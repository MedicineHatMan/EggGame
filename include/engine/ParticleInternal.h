#ifndef _ENGINE_PARTICLEINTERNAL_H_
#define _ENGINE_PARTICLEINTERNAL_H_

#include "enemy/boss/EneBoss.h"
#include "engine/TParticle.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "core/MathUtil.h"
#include "stl/math.h"
#include "camera/TCamEditor.h"
#include "core/TMainTask.h"

struct NJS_VECTOR;

typedef char ParticleDesc_size_check[(sizeof(ParticleDesc) == 0xA0) ? 1 : -1];

enum {
	PARTDESC_StretchX = 0x1,
	PARTDESC_StretchY = 0x2,
	PARTDESC_Fade     = 0x4, // from mColour to mColourEnd
	PARTDESC_SpinAny  = 0x8, // a coin toss picks the direction
	PARTDESC_Local    = 0x10,
	PARTDESC_Fixed    = 0x100, // emit flat speed along z
	PARTDESC_KeepRot  = 0x200,
};

enum {
	PART_Fading   = 0x1,
	PART_Stretch  = 0x2,
	PART_Spin     = 0x4,
	PART_SpinBack = 0x8,
	PART_Colour   = 0x10,
	PART_Bit100   = 0x100, // emit speed runs along mAxis
	PART_Bit200   = 0x200, // vacuum its rise is turned by the matrix
};

#define partRand() (3.0517578125e-05f * (f32)rand())

/**
 * What every particle has. It sits at _20 of its object.
 *
 * @size 0x80
 */
struct Particle {
	void init(const ParticleDesc* desc, f32 scale);
	void setSpriteScale(f32 scale);
	f32 getFade();
	void setAlpha(f32 alpha);
	void updateScale();
	void updateColour();
	void spin();
	void updateFrame();
	int advance();
	void updateStretch();
	void updateStretchAt(const Vec3f* pos);
	void addPos(const Vec3f* move);
	void setLocalPos(int slot, const Vec3f* local);
	void setLastAt(const Vec3f* pos);
	void setLast();

	const ParticleDesc* mDesc; // _00
	u8 mSprite[0x10];          // _04 handed to spriteAdd runs to _3C
	Vec3f mPos;                // _14
	f32 mScaleX;               // _20
	f32 mScaleY;               // _24
	u32 mColourNow;            // _28
	f32 mSpriteScale;          // _2C
	u32 mTex;                  // _30
	s16 mRot;                  // _34
	u16 mFrame;                // _36
	u16 mRotX;                 // _38
	u16 mRotY;                 // _3A
	u32 mFlags;                // _3C
	u32 mColour;               // _40
	Vec3f mLast;               // _44
	f32 mBaseX;                // _50
	f32 mBaseY;                // _54
	f32 mFadeIn;               // _58
	f32 mFadeOut;              // _5C
	f32 mFadeInScale;          // _60
	f32 mFadeOutScale;         // _64
	f32 mAge;                  // _68 0 to 1
	f32 mAgeStep;              // _6C
	s16 mColourStep[3];        // _70
	u16 mColourValue[3];       // _76
	s16 mAnimRate;             // _7C
	s16 mAnimPhase;            // _7E
};

typedef char Particle_size_check[(sizeof(Particle) == 0x80) ? 1 : -1];

/**
 * @size 0xA4
 */
struct ParticleEmit : public Particle {
	void init(const ParticleDesc* desc, int rotX, int rotY, f32 scale);
	int step();

	Vec3f mVel;   // _80
	Vec3f mAccel; // _8C
	Vec3f mAxis;  // _98
};

/**
 * @size 0xA8
 */
struct ParticleSpiral : public Particle {
	void init(const ParticleDesc* desc, f32 scale);
	void getPos(Vec3f* out) const;
	int step();

	Vec3f mCentre;   // _80
	f32 mOrbit;      // _8C
	f32 mOrbitSpeed; // _90
	int mAngle;      // _94
	int mTurn;       // _98
	f32 mUp;         // _9C
	int mWobble;     // _A0
	int mWobbleStep; // _A4
};

/**
 * @size 0xA0
 */
struct ParticleVacuum : public Particle {
	void init(const ParticleDesc* desc, int rotX, int rotY, f32 scale);
	int step(Vec3f* pos);

	f32 mDist;  // _80
	Vec3f mDir; // _84
	f32 mSpeed; // _90
	Vec3f mVel; // _94
};

/**
 * @size 0xC4
 */
class TParticleEmitG : public TObject {
public:
	TParticleEmitG(TObject* parent, const ParticleDesc* desc, const Vec3f* pos, int rotX, int rotY, f32 scale);
	virtual ~TParticleEmitG();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();

	ParticleEmit mPart; // _20
};

/**
 * @size 0xD4
 */
class TParticleEmitL : public TObject {
public:
	TParticleEmitL(TObject* parent, const ParticleDesc* desc, int slot, int rotX, int rotY, f32 scale);
	virtual ~TParticleEmitL();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();

	ParticleEmit mPart; // _20
	int mSlot;          // _C4
	Vec3f mLocal;       // _C8
};

/**
 * @size 0xC8
 */
class TParticleSpiralG : public TObject {
public:
	TParticleSpiralG(TObject* parent, const ParticleDesc* desc, const Vec3f* pos, f32 scale);
	virtual ~TParticleSpiralG() { }

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();

	ParticleSpiral mPart; // _20
};

/**
 * @size 0xD8
 */
class TParticleSpiralL : public TObject {
public:
	TParticleSpiralL(TObject* parent, const ParticleDesc* desc, int slot, f32 scale);
	virtual ~TParticleSpiralL() { }

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();

	ParticleSpiral mPart; // _20
	int mSlot;            // _C8
	Vec3f mLocal;         // _CC
};

/**
 * @size 0xC0
 */
class TParticleVacuumG : public TObject {
public:
	TParticleVacuumG(TObject* parent, const ParticleDesc* desc, const Vec3f* pos, int rotX, int rotY, f32 scale);
	virtual ~TParticleVacuumG();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();

	ParticleVacuum mPart; // _20
};

/**
 * @size 0xD0
 */
class TParticleVacuumL : public TObject {
public:
	TParticleVacuumL(TObject* parent, const ParticleDesc* desc, int slot, int rotX, int rotY, f32 scale);
	virtual ~TParticleVacuumL();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();

	ParticleVacuum mPart; // _20
	int mSlot;            // _C0
	Vec3f mLocal;         // _C4
};

/**
 * @size 0x5C
 */

typedef char TParticleEmitG_size_check[(sizeof(TParticleEmitG) == 0xC4) ? 1 : -1];
typedef char TParticleEmitL_size_check[(sizeof(TParticleEmitL) == 0xD4) ? 1 : -1];
typedef char TParticleSpiralG_size_check[(sizeof(TParticleSpiralG) == 0xC8) ? 1 : -1];
typedef char TParticleSpiralL_size_check[(sizeof(TParticleSpiralL) == 0xD8) ? 1 : -1];
typedef char TParticleVacuumG_size_check[(sizeof(TParticleVacuumG) == 0xC0) ? 1 : -1];
typedef char TParticleVacuumL_size_check[(sizeof(TParticleVacuumL) == 0xD0) ? 1 : -1];
typedef char TParticleGenerator_size_check[(sizeof(TParticleGenerator) == 0x5C) ? 1 : -1];

#endif
