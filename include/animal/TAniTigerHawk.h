#ifndef _ANIMAL_TANITIGERHAWK_H
#define _ANIMAL_TANITIGERHAWK_H
#include "egg/EggCreature.h"
#include "engine/Fx.h"
#include "object/ObjAction.h"
#include "stage/Stage.h"
struct TAniTigerHawk;

#include "core/types.h"

/**
 * @size 0x3CC
 */
class TAniTigerHawk : public TEggCreatureTypeE {
public:
	TAniTigerHawk(TObject* parent, int player, int kind, const Vec3f* pos);
	virtual ~TAniTigerHawk();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual int onHit(const TObjAction* other, int arg, int* result);
	virtual void resetBones();
	virtual void onPhase(int phase);

	void runFly();
	void runStep27();
	void runEnd();

	f32 mFall;          // _384
	f32 mTravelled;     // _388
	f32 mSpeed;         // _38C
	f32 mAccel;         // _390
	int _394;           // _394
	Fx* mFx2;           // _398
	f32 mFx2Matrix[12]; // _39C
};

extern "C" {
extern const char* gTAniTigerHawkName;
extern void* const sTigerHawkCallbacks[];
}

#endif
