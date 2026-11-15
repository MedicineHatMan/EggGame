#include "engine/ParticleInternal.h"

extern "C" {
int sParticlePosNext;
int sParticlePosCount;
int sParticleDescNext;
int sParticleDescCount;
ParticleDesc sParticleDescSlots[0x40];
u8 sParticleDescUsed[0x40];
Vec3f sParticlePosSlots[0x40];
u8 sParticlePosUsed[0x40];
}

// 0x8002684C
extern "C" int particleSlotAlloc(int* next, int* count, u8* used, int max, s16* out)
{
	if (*count >= max) {
		return 0;
	}
	int start = *next;
	int found = -1;
	for (int i = start; i < max; i++) {
		if (used[i] == 0) {
			found = i;
			break;
		}
	}
	if (found == -1) {
		for (int i = 0; i < start; i++) {
			if (used[i] == 0) {
				found = i;
				break;
			}
		}
	}
	if (found == -1) {
		return 0;
	}
	(*count)++;
	used[found] = 1;
	*out        = found;
	*next       = found + 1;
	if (*next >= max) {
		*next = 0;
	}
	return 1;
}

// 0x80026928
extern "C" void particleSlotFree(int* count, u8* used, int max, s16 slot)
{
	if (slot < 0 || slot >= max) {
		return;
	}
	(*count)--;
	used[slot] = 0;
}

// 0x80026958
extern "C" int particlePosSlotAlloc(s16* out)
{
	return particleSlotAlloc(&sParticlePosNext, &sParticlePosCount, sParticlePosUsed, 0x40, out);
}

// 0x80026990
extern "C" void particlePosSlotFree(s16 slot)
{
	particleSlotFree(&sParticlePosCount, sParticlePosUsed, 0x40, slot);
}

// 0x800269C4
extern "C" void particlePosSlotSet(s16 slot, const Vec3f* pos)
{
	if (slot < 0 || slot >= 0x40) {
		return;
	}
	sParticlePosSlots[slot] = *pos;
}

// 0x80026A08
extern "C" void particlePosSlotAdd(s16 slot, Vec3f* pos)
{
	if (slot < 0 || slot >= 0x40) {
		return;
	}
	Vec3f at;
	at = sParticlePosSlots[slot];
	njAddVector(pos, &at);
}

// Not in the DOL the linker stripped it
extern "C" void particlePosSlotClear(s16 slot)
{
	if (slot < 0 || slot >= 0x40) {
		return;
	}
	sParticlePosSlots[slot].x = 0.0f;
	sParticlePosSlots[slot].y = 0.0f;
	sParticlePosSlots[slot].z = 0.0f;
}

// 0x80026A70
extern "C" ParticleDesc* particleDescSlotAlloc(s16* out)
{
	if (particleSlotAlloc(&sParticleDescNext, &sParticleDescCount, sParticleDescUsed, 0x40, out)) {
		return &sParticleDescSlots[*out];
	}
	return 0;
}

// 0x80026AD8
extern "C" void particleDescSlotFree(s16 slot)
{
	particleSlotFree(&sParticleDescCount, sParticleDescUsed, 0x40, slot);
}

// 0x80026B0C
extern "C" TParticleGenerator* particleGenNew(TObject* parent, const Vec3f* pos, const ParticleDesc* desc)
{
	return new TParticleGenerator(parent, pos, desc);
}

// 0x80026B68
// with no record and so no particles
extern "C" TParticleGenerator* particleGenEmpty(TObject* parent, const Vec3f* pos, int, int)
{
	return particleGenNew(parent, pos, 0);
}

// 0x80026B8C
extern "C" TParticleGenerator* particleGenEmptyDefault(const Vec3f* pos, int a, int b)
{
	return particleGenEmpty(gTL_07, pos, a, b);
}

// 0x80026BC4
extern "C" TParticleGenerator* particleGenById(TObject* parent, const Vec3f* pos, int bank, int id)
{
	const ParticleDesc* desc = getParticleDesc(bank, id);
	return particleGenNew(parent, pos, desc);
}

// 0x80026C10
extern "C" TParticleGenerator* particleGenByIdRot(TObject* parent, const Vec3f* pos, int bank, int id, int rotX, int rotY, f32 scale)
{
	TParticleGenerator* gen = particleGenById(parent, pos, bank, id);
	if (gen != 0) {
		gen->setRot(rotX, rotY);
		gen->setScale(scale);
	}
	return gen;
}

// 0x80026C70
extern "C" TParticleGenerator* particleGenByIdDefault(const Vec3f* pos, int bank, int id)
{
	return particleGenById(gTL_07, pos, bank, id);
}

// 0x80026CA8
extern "C" TParticleGenerator* particleGenByIdRotDefault(const Vec3f* pos, int bank, int id, int rotX, int rotY, f32 scale)
{
	return particleGenByIdRot(gTL_07, pos, bank, id, rotX, rotY, scale);
}

// 0x80026CF0
extern "C" TParticleGenerator* particleOnce(TObject* parent, const Vec3f* pos, const ParticleDesc* desc)
{
	TParticleGenerator* gen = new TParticleGenerator(parent, pos, desc);
	if (gen != 0) {
		gen->setLife(1);
	}
	return gen;
}

// 0x80026D60
extern "C" TParticleGenerator* particleOnceById(TObject* parent, const Vec3f* pos, int bank, int id)
{
	const ParticleDesc* desc = getParticleDesc(bank, id);
	return particleOnce(parent, pos, desc);
}

// 0x80026DAC
extern "C" TParticleGenerator* particleOnceByIdRot(TObject* parent, const Vec3f* pos, int bank, int id, int rotX, int rotY, f32 scale)
{
	TParticleGenerator* gen = particleGenById(parent, pos, bank, id);
	if (gen != 0) {
		gen->setLife(1);
		gen->setRot(rotX, rotY);
		gen->setScale(scale);
	}
	return gen;
}

// 0x80026E18
extern "C" TParticleGenerator* particleOnceByIdDefault(const Vec3f* pos, int bank, int id)
{
	return particleOnceById(gTL_07, pos, bank, id);
}

// 0x80026E50
extern "C" TParticleGenerator* particleOnceByIdRotDefault(const Vec3f* pos, int bank, int id, int rotX, int rotY, f32 scale)
{
	return particleOnceByIdRot(gTL_07, pos, bank, id, rotX, rotY, scale);
}

// 0x80026E98
extern "C" TParticleGenerator* particleGenTimed(TObject* parent, const Vec3f* pos, const ParticleDesc* desc, int delay, int life)
{
	TParticleGenerator* gen = new TParticleGenerator(parent, pos, desc);
	if (gen != 0) {
		gen->setDelay(delay);
		gen->setLife(life);
	}
	return gen;
}

// 0x80026F1C
extern "C" TParticleGenerator* particleGenTimedById(TObject* parent, const Vec3f* pos, int bank, int id, int delay, int life)
{
	const ParticleDesc* desc = getParticleDesc(bank, id);
	return particleGenTimed(parent, pos, desc, delay, life);
}

// 0x80026F78
extern "C" TParticleGenerator* particleGenTimedByIdRot(TObject* parent, const Vec3f* pos, int bank, int id, int rotX, int rotY, int delay,
                                                       int life, f32 scale)
{
	TParticleGenerator* gen = particleGenById(parent, pos, bank, id);
	if (gen != 0) {
		gen->setDelay(delay);
		gen->setLife(life);
		gen->setRot(rotX, rotY);
		gen->setScale(scale);
	}
	return gen;
}

// 0x80026FF8
extern "C" TParticleGenerator* particleGenTimedByIdDefault(const Vec3f* pos, int bank, int id, int delay, int life)
{
	return particleGenTimedById(gTL_07, pos, bank, id, delay, life);
}

// 0x80027040
extern "C" TParticleGenerator* particleGenTimedByIdRotDefault(const Vec3f* pos, int bank, int id, int rotX, int rotY, int delay, int life,
                                                              f32 scale)
{
	return particleGenTimedByIdRot(gTL_07, pos, bank, id, rotX, rotY, delay, life, scale);
}

const char* gTParticleGeneratorName = "TParticleGenerator";

// 0x800270A0
TParticleGenerator::TParticleGenerator(TObject* parent, const Vec3f* pos, const ParticleDesc* desc)
    : TObject(parent)
{
	_00       = (u32)gTParticleGeneratorName;
	mSize     = 0x5C;
	mGenFlags = 0;
	mPos      = *pos;
	mRotX     = 0;
	mRotY     = 0;
	mDesc     = desc;
	mPosSlot  = -1;
	if (mDesc != 0) {
		mRate = (f32)mDesc->mRate / 100.0f;
		if (mDesc->mFlags & PARTDESC_Local) {
			particlePosSlotAlloc(&mPosSlot);
		}
	} else {
		mRate = 0.0f;
	}
	mCount    = 0.0f;
	mPhase    = 0;
	mTime     = 0;
	mLife     = 0;
	mScale    = 0.0f;
	mDescSlot = -1;
	mAttach   = 0;
}

// 0x800271AC
TParticleGenerator::~TParticleGenerator()
{
	particlePosSlotFree(mPosSlot);
	particleDescSlotFree(mDescSlot);
}

// 0x8002721C
void TParticleGenerator::setDelay(int delay)
{
	mTime = -delay;
}

// 0x80027228
void TParticleGenerator::setLife(int life)
{
	mGenFlags |= 1;
	mLife = life;
}

// 0x8002723C
void TParticleGenerator::setScale(f32 scale)
{
	mScale = scale;
}

// 0x80027244
void TParticleGenerator::setRot(int rotX, int rotY)
{
	mRotX = rotX;
	mRotY = rotY;
}

// 0x80027250
void TParticleGenerator::draw()
{
	if (mGenFlags & 2) {
		if (!(mGenFlags & 4)) {
			mFlags |= 1;
		}
		return;
	}
	mTime++;
	if (mTime < 0) {
		return;
	}
	if ((mGenFlags & 1) && mTime >= mLife) {
		mGenFlags |= 2;
	}
	if (mDesc == 0) {
		return;
	}
	if (mAttach != 0) {
		mtxGetPos(mAttach, &mPos);
	}
	if (mDesc->mFlags & PARTDESC_Local) {
		particlePosSlotSet(mPosSlot, &mPos);
	}
	if (mDesc->mRateRate == 0) {
		mCount += mRate;
	} else {
		mPhase += mDesc->mRateRate;
		mPhase &= 0x7FFF;
		mCount += mRate * njSin(mPhase);
	}

	bool t                   = false;
	const ParticleDesc* desc = mDesc;
	int kind                 = desc->mKind;
	if (mRotX != 0 || mRotY != 0) {
		t = true;
	}
	int turned = t;
	if (desc->mFlags & PARTDESC_Local) {
		if (turned) {
			gjPushUnitMatrix();
			gjRotateY(mRotY);
			gjRotateX(mRotX);
		}
		switch (kind) {
		case 0:
			while (mCount >= 1.0f) {
				mCount -= 1.0f;
				particleEmitLNew(gTL_PARTICLE, mDesc, mPosSlot, mScale, mRotX, mRotY);
			}
			break;
		case 1:
			while (mCount >= 1.0f) {
				mCount -= 1.0f;
				particleSpiralLNew(gTL_PARTICLE, mDesc, mPosSlot, mScale);
			}
			break;
		case 2:
			while (mCount >= 1.0f) {
				mCount -= 1.0f;
				particleVacuumLNew(gTL_PARTICLE, mDesc, mPosSlot, mScale, mRotX, mRotY);
			}
			break;
		}
		if (turned) {
			njPopMatrixEx();
		}
	} else {
		if (turned) {
			gjPushUnitMatrix();
			gjRotateY(mRotY);
			gjRotateX(mRotX);
		}
		switch (kind) {
		case 0:
			while (mCount >= 1.0f) {
				mCount -= 1.0f;
				particleEmitGNew(gTL_PARTICLE, mDesc, &mPos, mScale, mRotX, mRotY);
			}
			break;
		case 1:
			while (mCount >= 1.0f) {
				mCount -= 1.0f;
				particleSpiralGNew(gTL_PARTICLE, mDesc, &mPos, mScale);
			}
			break;
		case 2:
			while (mCount >= 1.0f) {
				mCount -= 1.0f;
				particleVacuumGNew(gTL_PARTICLE, mDesc, &mPos, mScale, mRotX, mRotY);
			}
			break;
		}
		if (turned) {
			njPopMatrixEx();
		}
	}
}

// 0x800275C8
int TParticleGenerator::hasOwnDesc()
{
	return mGenFlags & 8;
}

// 0x800275D4
// ParticleDesc as the original seems to have declared it value/range pairs and the start box are nested structs so a
// copy moves them as word pairs makeOwnDesc
struct DescRange {
	f32 v, r;
};
struct DescRangeI {
	s32 v;
	u32 r;
};
struct DescBox {
	f32 x, y, z;
};
struct DescBoxW {
	DescBox b;
};
struct DescMirror {
	u16 kind, flags;
	ParticleTex tex;
	u32 texId;
	DescBoxW radius;
	DescRange scaleX, scaleY;
	f32 scaleDamp;
	DescRange speed, up;
	f32 speedDamp, gravity;
	u8 colour[4];
	u8 colourEnd[4];
	u16 spin, rot;
	s16 rateRate, rate, life, _56;
	f32 fadeIn, fadeOut;
	DescRange orbit, wobble;
	DescRangeI turn;
	u8 _78[8];
	f32 animRate;
	u8 _84[0x1C];
};

void TParticleGenerator::makeOwnDesc()
{
	if (hasOwnDesc()) {
		return;
	}
	ParticleDesc* own = particleDescSlotAlloc(&mDescSlot);
	if (own != 0) {
		mGenFlags |= 8;
		*(DescMirror*)own = *(const DescMirror*)mDesc;
		mDesc             = own;
	}
}

// 0x80027780
ParticleDesc* TParticleGenerator::getOwnDesc()
{
	if (hasOwnDesc()) {
		return (ParticleDesc*)mDesc;
	}
	return 0;
}
