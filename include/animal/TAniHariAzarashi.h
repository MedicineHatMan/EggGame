#ifndef _ANIMAL_TANIHARIAZARASHI_H
#define _ANIMAL_TANIHARIAZARASHI_H
#include "animal/AniAttacks.h"
#include "egg/EggCreature.h"
#include "object/ObjAction.h"
#include "stage/Stage.h"
struct TAniHariAzarashi;

#include "core/types.h"

/**
 * @size 0x3A0
 */
class TAniHariAzarashi : public TEggCreatureTypeE {
public:
	TAniHariAzarashi(TObject* parent, int player, int kind, const Vec3f* pos);
	virtual ~TAniHariAzarashi();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual int onHit(const TObjAction* other, int arg, int* result);
	virtual void render();
	virtual void resetBones();
	virtual void onPhase(int phase);

	void runSpread();
	void runSlide();
	void runRocks();
	void killAll();

	TAniIceSlider* mSliders[3]; // _384
	TAniIceRock* mRocks[3];     // _390
	f32 mSpread;                // _39C
};

#include "engine/MotionPlayer.h"
extern "C" {
extern const char* gTAniHariAzarashiName;
extern f32 sHariAzarashiRate0;
extern f32 sHariAzarashiRate1;
extern f32 sHariAzarashiRate2;
extern f32 sHariAzarashiRate3;
extern f32 sHariAzarashiRate4;
extern void* const sHariAzarashiCallbacks[];
extern MotionEntry gHariMotions[];
}

#endif
