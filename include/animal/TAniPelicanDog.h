#ifndef _ANIMAL_TANIPELICANDOG_H
#define _ANIMAL_TANIPELICANDOG_H
#include "egg/EggCreature.h"
#include "object/ObjAction.h"
#include "stage/Stage.h"
struct TAniPelicanDog;

#include "core/types.h"

/**
 * @size 0x3A0
 */
class TAniPelicanDog : public TEggCreatureTypeE {
public:
	TAniPelicanDog(TObject* parent, int player, int kind, const Vec3f* pos);
	virtual ~TAniPelicanDog();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual int onHit(const TObjAction* other, int arg, int* result);
	virtual void draw();
	virtual void vfn_10();
	virtual void resetBones();
	virtual void onPhase(int phase);

	void renderExtra();
	void runWait();
	void runFly();
	void runShrink();

	f32 mTravelled;  // _384
	f32 mSpeed;      // _388
	int mSpin;       // _38C
	s16 mNodeKind;   // _390
	u8 _392[2];
	void* mExtraTask; // _394
	int mShowExtra;  // _398
	f32 mExtraFrame; // _39C
};

#include "engine/MotionPlayer.h"
extern "C" {
extern MotionEntry gPelicanMotions[];
extern const char* gTAniPelicanDogName;
extern f32 sPelicanDogRate0;
extern f32 sPelicanDogRate1;
extern f32 sPelicanDogRate2;
extern f32 sPelicanDogRate3;
extern void* const sPelicanDogCallbacks[];
extern f32 sPelicanBoneMtx[12];
}

#endif
