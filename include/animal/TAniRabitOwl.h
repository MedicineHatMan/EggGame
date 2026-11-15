#ifndef _ANIMAL_TANIRABITOWL_H
#define _ANIMAL_TANIRABITOWL_H
#include "animal/AniAttacks.h"
#include "egg/EggCreature.h"
#include "object/ObjAction.h"
#include "stage/Stage.h"
struct TAniRabitOwl;

#include "core/types.h"

/**
 * @size 0x3B8
 */
class TAniRabitOwl : public TEggCreatureTypeE {
public:
	TAniRabitOwl(TObject* parent, int player, int kind, const Vec3f* pos);
	virtual ~TAniRabitOwl();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual int onHit(const TObjAction* other, int arg, int* result);
	virtual void resetBones();
	virtual void onPhase(int phase);

	void runFly();
	void runWait();
	void runBurst();
	void throwBombs();

	f32 mTravelled;                 // _384
	f32 mSpeed;                     // _388
	f32 mAccel;                     // _38C
	f32 mGrowth;                    // _390
	int mWait;                      // _394
	TAniFlushFlowerBomb* mBombs[8]; // _398
};

#include "engine/MotionPlayer.h"
extern "C" {
extern MotionEntry gRacerMotions[];
extern const char* gTAniRabitOwlName;
extern void* const sRabitOwlCallbacks[];
}

#endif
