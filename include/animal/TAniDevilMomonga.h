#ifndef _ANIMAL_TANIDEVILMOMONGA_H
#define _ANIMAL_TANIDEVILMOMONGA_H
#include "egg/EggCreature.h"
#include "engine/Fx.h"
#include "object/ObjAction.h"
#include "stage/Stage.h"
struct TAniDevilMomonga;

#include "core/types.h"

/**
 * @size 0x408
 */
class TAniDevilMomonga : public TEggCreatureTypeE {
public:
	TAniDevilMomonga(TObject* parent, int player, int kind, const Vec3f* pos);
	virtual ~TAniDevilMomonga();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual int onHit(const TObjAction* other, int arg, int* result);
	virtual void render();
	virtual void resetBones();
	virtual void onPhase(int phase);

	void runOut();
	void runBack();
	void runEnd();
	void turnBack(int angle);
	inline void killAllFx();

	int mImageWait;      // _384
	f32 mTravelled;      // _388
	f32 mSpeed;          // _38C
	f32 mTopSpeed;       // _390
	int mTurnRate;       // _394
	f32 mBob;            // _398
	int mBobAngle;       // _39C
	f32 mSpinMatrix[12]; // _3A0
	int mSpinAngle;      // _3D0
	Fx* mFx2;            // _3D4
	f32 mFx2Matrix[12];  // _3D8
};

#include "engine/MotionPlayer.h"
extern "C" {
extern const char* gTAniDevilMomongaName;
extern void* const sDevilMomongaCallbacks[];
extern MotionEntry gMomongaMotions[11];
}

#endif
