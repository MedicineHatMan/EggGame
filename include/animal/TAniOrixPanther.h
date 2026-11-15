#ifndef _ANIMAL_TANIORIXPANTHER_H
#define _ANIMAL_TANIORIXPANTHER_H
#include "egg/EggCreature.h"
#include "engine/Fx.h"
#include "object/ObjAction.h"
#include "stage/Stage.h"
struct TAniOrixPanther;

#include "core/types.h"

/**
 * @size 0x3F4
 */
class TAniOrixPanther : public TEggCreatureTypeE {
public:
	TAniOrixPanther(TObject* parent, int player, int kind, const Vec3f* pos);
	virtual ~TAniOrixPanther();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual int onHit(const TObjAction* other, int arg, int* result);
	virtual void resetBones();
	virtual void onPhase(int phase);

	void runStart();
	void runDash();
	void runEnd();
	void endDash();
	void pickTarget();
	void pickPoint();
	inline void killAllFx();

	int mDashes;        // _384
	s8 mMotionCount;    // _388 how many motions it can pick between after the first ten
	u8 _389[3];
	Fx* mFx2;         // _38C
	Fx* mFx3;         // _390
	f32 mFx2Matrix[12]; // _394
	f32 mFx3Matrix[12]; // _3C4
};

#include "engine/MotionPlayer.h"
extern "C" {
extern MotionEntry gOrixMotions[13];
extern const char* gTAniOrixPantherName;
extern void* const sOrixPantherCallbacks[];
}

#endif
