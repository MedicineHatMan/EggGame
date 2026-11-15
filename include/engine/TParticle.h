#ifndef _TPARTICLE_H
#define _TPARTICLE_H

#include "stage/Stage.h"
#include "engine/ParticleBank.h"

/**
 * @size 0x5C
 */
class TParticleGenerator : public TObject {
public:
	TParticleGenerator(TObject* parent, const Vec3f* pos, const ParticleDesc* desc);
	virtual ~TParticleGenerator();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	void setDelay(int delay);
	void setLife(int life);
	void setScale(f32 scale);
	void setRot(int rotX, int rotY);
	int hasOwnDesc();
	void makeOwnDesc();
	ParticleDesc* getOwnDesc();

	u32 mGenFlags;            // _20 1 timed 2 ended 4 stay when ended 8 own record
	Vec3f mPos;               // _24
	int mRotX;                // _30
	int mRotY;                // _34
	const ParticleDesc* mDesc; // _38
	f32 mRate;                // _3C
	f32 mCount;               // _40 particles owed
	int mPhase;               // _44
	int mTime;                // _48 negative while waiting
	int mLife;                // _4C
	f32 mScale;               // _50
	s16 mPosSlot;             // _54
	s16 mDescSlot;            // _56
	const f32* mAttach;       // _58 a matrix to follow
};

extern "C" {
TParticleGenerator* particleGenByIdRot(TObject* parent, const Vec3f* pos, int bank, int id, int rotX, int rotY, f32 scale);
}

class TObject;
struct ParticleDesc;
struct Vec3f;

extern "C" {
ParticleDesc* particleDescSlotAlloc(s16*);
void particleDescSlotFree(s16);
TParticleGenerator* particleGenEmptyDefault(const Vec3f*, int, int);
void particlePosSlotAdd(s16, Vec3f*);
int particlePosSlotAlloc(s16*);
void particlePosSlotFree(s16);
void particlePosSlotSet(s16, const Vec3f*);
int particleSlotAlloc(int*, int*, u8*, int, s16*);
void particleSlotFree(int*, u8*, int, s16);
void* particleEmitGNew(TObject*, const ParticleDesc*, const Vec3f*, f32, int, int);
void* particleEmitLNew(TObject*, const ParticleDesc*, int, f32, int, int);
void* particleSpiralGNew(TObject*, const ParticleDesc*, const Vec3f*, f32);
void* particleSpiralLNew(TObject*, const ParticleDesc*, int, f32);
void* particleVacuumGNew(TObject*, const ParticleDesc*, const Vec3f*, f32, int, int);
void* particleVacuumLNew(TObject*, const ParticleDesc*, int, f32, int, int);
}

extern "C" {
TParticleGenerator* particleOnceById(TObject*, const Vec3f*, int, int);
}

extern "C" {
TParticleGenerator* particleOnceByIdDefault(const Vec3f*, int, int);
TParticleGenerator* particleGenById(TObject*, const Vec3f*, int, int);
TParticleGenerator* particleGenByIdDefault(const Vec3f*, int, int);
TParticleGenerator* particleOnceByIdRotDefault(const Vec3f*, int, int, int, int, f32);
TParticleGenerator* particleGenTimedByIdDefault(const Vec3f*, int, int, int, int);
TParticleGenerator* particleGenTimedById(TObject*, const Vec3f*, int, int, int, int);
}

extern "C" {
TParticleGenerator* particleGenTimedByIdRotDefault(const Vec3f*, int, int, int, int, int, int, f32);
TParticleGenerator* particleOnceByIdRot(TObject*, const Vec3f*, int, int, int, int, f32);
}

extern "C" {
TParticleGenerator* particleGenByIdRotDefault(const Vec3f*, int, int, int, int, f32);
}

extern "C" {
extern const char* gTParticleEmitGName;
extern const char* gTParticleEmitLName;
extern const char* gTParticleGeneratorName;
extern const char* gTParticleSpiralGName;
extern const char* gTParticleSpiralLName;
extern const char* gTParticleVacuumGName;
extern const char* gTParticleVacuumLName;
extern int sParticlePosNext;
extern int sParticlePosCount;
extern int sParticleDescNext;
extern int sParticleDescCount;
}

extern "C" {
extern u8 sParticlePosUsed[0x40];    // position slot in use
extern Vec3f sParticlePosSlots[0x40]; // position slots
extern u8 sParticleDescUsed[0x40];    // record slot in use
extern ParticleDesc sParticleDescSlots[0x40]; // record slots
}

#endif
