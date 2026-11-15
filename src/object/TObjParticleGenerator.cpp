#include "stage/SetManager.h"
#include "engine/TParticle.h"
#include "egg/EggCreature.h"
#include "camera/TCamera.h"
#include "engine/FogBlend.h"
#include "object/TObjParticleGenerator.h"
#include "core/TMainTask.h"
#include "Ninja/ninja.h"

extern "C" {
const char* gTObjParticleGeneratorName = "TObjParticleGenerator";
}

enum {
	PTCLGEN_Always = 0,
	PTCLGEN_Day    = 1,
	PTCLGEN_Night  = 2,
};

typedef char TObjParticleGenerator_size_check[(sizeof(TObjParticleGenerator) == 0xA4) ? 1 : -1];

// 0x8004D45C
// class 0x3
extern "C" TObject* createTObjParticleGenerator(TObject* parent, SetDesc* desc)
{
	return new TObjParticleGenerator(parent, desc, 500.0f);
}

// 0x8004D4B4
// class 0x1F
extern "C" TObject* createTObjParticleGenerator1F(TObject* parent, SetDesc* desc)
{
	return new TObjParticleGenerator(parent, desc, 100050.0f);
}

// 0x8004D50C
TObjParticleGenerator::TObjParticleGenerator(TObject* parent, SetDesc* desc, f32 range)
    : TSetObj(parent, desc)
{
	_00    = (u32)gTObjParticleGeneratorName;
	mSize  = 0xA4;
	mGen   = 0;
	mBank  = 0;
	mId    = -1;
	mScale = 0.0f;
	mRange = range;
	mWhen  = PTCLGEN_Always;
	initFromDesc(desc);
}

// 0x8004D5AC
TObjParticleGenerator::~TObjParticleGenerator()
{
	stop();
}

static inline int clampIndex(int i, int count)
{
	return i < 0 ? 0 : (i > count - 1 ? count - 1 : i);
}

// 0x8004D62C
void TObjParticleGenerator::initFromDesc(SetDesc* desc)
{
	readDesc(desc);

	int banks = getParticleSlotCount();
	int bank  = clampIndex(desc->getInt(1), banks);
	int count = getParticleDescCount(bank);
	int id    = clampIndex(desc->getInt(0), count);

	mWhen  = desc->getInt(2);
	mScale = desc->getFloat(0);

	if (isActive()) {
		if (mGen == 0) {
			mBank = bank;
			mId   = id;
			start();
		} else if (mId != id || mBank != bank) {
			stop();
			mBank = bank;
			mId   = id;
			start();
		}
	} else {
		mGen = 0;
	}

	if (mGen != 0) {
		int rotX   = mTrans.mRot.x;
		int rotY   = mTrans.mRot.y;
		mGen->mPos = mTrans.mPos;
		mGen->setRot(rotX, rotY);
		mGen->setScale(mScale);
	}
}

// 0x8004D7EC
void TObjParticleGenerator::start()
{
	mGen = particleGenByIdRot(gTL_07, &mTrans.mPos, mBank, mId, mTrans.mRot.x, mTrans.mRot.y, mScale);
}

// 0x8004D838
int TObjParticleGenerator::isActive()
{
	if (mWhen == PTCLGEN_Day) {
		return fogBlendIsDay();
	} else if (mWhen == PTCLGEN_Night) {
		return (u8)(fogBlendIsDay() == 0);
	}
	return true;
}

// 0x8004D884
// Deletes itself out of range
void TObjParticleGenerator::draw()
{
	if (!isInRange((void*)this, (const Vec3f*)&mTrans.mPos, mRange)) {
		mFlags |= 1;
	} else if (isActive()) {
		if (mGen == 0) {
			start();
		}
	} else {
		stop();
	}
}

// 0x8004D918
// The set editor's labels
void TObjParticleGenerator::vfn_78(SetDesc* desc)
{
	njPrintC(0xA0013, ":PTCL NO");
	njPrintC(0xA0014, ":BANK NO");
	switch (mWhen) {
	case PTCLGEN_Always:
	default:
		njPrintC(0xA0015, ":ALL TIME");
		break;
	case PTCLGEN_Day:
		njPrintC(0xA0015, ":DAY");
		break;
	case PTCLGEN_Night:
		njPrintC(0xA0015, ":NIGHT");
		break;
	}
	njPrintC(0xA0017, ":OFFS Z");
}
