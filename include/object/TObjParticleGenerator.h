#ifndef _OBJECT_TOBJPARTICLEGENERATOR_H_
#define _OBJECT_TOBJPARTICLEGENERATOR_H_
#include "Ninja/ninja.h"
#include "engine/TParticle.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjParticleGenerator;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjParticleGenerator(TObject*, SetDesc*);
TObject* createTObjParticleGenerator1F(TObject*, SetDesc*);
}

class TObjParticleGenerator : public TSetObj {
public:
	TObjParticleGenerator(TObject* parent, SetDesc* desc, f32 range);
	virtual ~TObjParticleGenerator();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void initFromDesc(SetDesc* desc);
	virtual void vfn_78(SetDesc* desc);

	void start();
	int isActive();

	inline void stop()
	{
		if (mGen != 0) {
			mGen->mFlags |= 1;
			mGen = 0;
		}
	}

	TParticleGenerator* mGen; // _90
	s16 mBank;                // _94 the slot in ParticleBank.cpp
	s16 mId;                  // _96 the record in that slot
	f32 mScale;               // _98
	f32 mRange;               // _9C how far from the player it keeps running
	int mWhen;                // _A0 PTCLGEN_*
};

#endif
