#ifndef _ANIMAL_TANIKIJISAI_H
#define _ANIMAL_TANIKIJISAI_H
#include "Ninja/ninja.h"
struct SpiralVtx;
#include "egg/EggCreature.h"
#include "engine/Fx.h"
#include "object/ObjAction.h"
#include "stage/Stage.h"
struct TAniKijiSai;

#include "core/types.h"

/**
 * @size 0x3D4
 */
class TAniKijiSai : public TEggCreatureTypeE {
public:
	TAniKijiSai(TObject* parent, int player, int kind, const Vec3f* pos);
	virtual ~TAniKijiSai();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual int onHit(const TObjAction* other, int arg, int* result);
	virtual void render();
	virtual void resetBones();
	virtual void onPhase(int phase);
	virtual void vfn_A0();

	void runFly();
	void runEnd();

	inline void killFx2();

	Vec3f mFlyDir;       // _384
	f32 mSpeed;          // _390
	f32 mSpread;         // _394 how far each copy sits from the middle
	int mSpin;           // _398
	f32 mTravelled;      // _39C
	Fx* mFx2;          // _3A0
	f32 mFx2Matrix[12];  // _3A4
};

struct SpiralVtx {
	f32 x, y, z; // _00
	f32 u, v;    // _0C
	u32 color;   // _14
};

#include "engine/CharaColli.h"
#include "engine/MotionPlayer.h"
#include "object/ObjClassName.h"
extern "C" {
extern ObjClassName gTAniKijiSaiName;
extern ObjClassName gTAniSpiralFireName;
extern f32 sKijiSaiRate0;
extern f32 sKijiSaiRate1;
extern f32 sKijiSaiRate2;
extern f32 sKijiSaiRate3;
extern void* const sKijiSaiCallbacks[];
extern const ColliShape sSpiralFireShape;
extern const SpiralVtx sSpiralFireRing[8];
extern MotionEntry gKijiMotions[];
}

#endif
