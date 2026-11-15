#ifndef _ANIMAL_TANIBULLBOAR_H
#define _ANIMAL_TANIBULLBOAR_H
#include "egg/EggCreature.h"
#include "engine/Fx.h"
#include "object/ObjAction.h"
#include "stage/Stage.h"
struct TAniBullBoar;

#include "core/types.h"

/**
 * @size 0x3F4
 */
class TAniBullBoar : public TEggCreatureTypeE {
public:
	TAniBullBoar(TObject* parent, int player, int kind, const Vec3f* pos);
	virtual ~TAniBullBoar();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual int onHit(const TObjAction* other, int arg, int* result);
	virtual void resetBones();
	virtual void onPhase(int phase);

	void runPaw();
	void runCharge();
	void runShrink();
	inline void killAllFx();

	f32 mSpeed;         // _384
	f32 mTopSpeed;      // _388
	Fx* mFx2;           // _38C
	Fx* mFx3;           // _390
	f32 mFx2Matrix[12]; // _394
	f32 mFx3Matrix[12]; // _3C4
};

#include "engine/MotionPlayer.h"
extern "C" {
extern const char* gTAniBullBoarName;
extern void* const sBullBoarCallbacks[8];
extern MotionEntry gBullBoarMotions[9];
}

#endif
