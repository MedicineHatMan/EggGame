#ifndef _ANIMAL_TANIKUMAKAME_H
#define _ANIMAL_TANIKUMAKAME_H
#include "egg/EggCreature.h"
#include "object/ObjAction.h"
#include "stage/Stage.h"
struct TAniKumaKame;

#include "core/types.h"

/**
 * @size 0x38C
 */
class TAniKumaKame : public TEggCreatureTypeE {
public:
	TAniKumaKame(TObject* parent, int player, int kind, const Vec3f* pos);
	virtual ~TAniKumaKame();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual int onHit(const TObjAction* other, int arg, int* result);
	virtual void resetBones();
	virtual void onPhase(int phase);

	void runWait();
	void runRise();
	void runFall();
	void runShrink();

	s16 mHops;     // _384
	u8 _386[2];
	f32 mDistance; // _388 to the target
};

#include "engine/MotionPlayer.h"
extern "C" {
extern const char* gTAniKumaKameName;
extern f32 sKumaKameRate0;
extern f32 sKumaKameRate1;
extern f32 sKumaKameRate2;
extern f32 sKumaKameRate3;
extern f32 sKumaKameRate4;
extern void* const sKumaKameCallbacks[];
extern MotionEntry gKumaMotions[];
}

#endif
