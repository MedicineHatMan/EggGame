#ifndef _ANIMAL_TANILIONKANGAROO_H_
#define _ANIMAL_TANILIONKANGAROO_H_
#include "Ninja/ninja.h"
#include "animal/AniMount.h"
#include "engine/DrawTask.h"
#include "engine/Fx.h"
#include "engine/MotionPlayer.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "player/PlyInput.h"
#include "sound/SoundHandle.h"
#include "stage/Stage.h"
struct TAniLionKangaroo;

#include "core/types.h"

class TObject;

extern "C" {
void* createLionKangaroo(TObject*);
}

/**
 * @size 0x474
 */
class TAniLionKangaroo : public TAniMount {
public:
	TAniLionKangaroo(TObject* parent);
	virtual ~TAniLionKangaroo();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void vfn_10();
	virtual Vec3f* getPos();
	virtual int getRotY();
	virtual int onHit(const TObjAction* other, int arg, int* result);
	virtual void vfn_94(f32* out);
	virtual int canMount() const;
	virtual int canDismount();
	virtual void onMount();
	virtual void onDismount();
	virtual void onLoaded();
	virtual void runState(int state, int phase);
	virtual void preUpdate();
	virtual void postUpdate();

	static void drawTask(TAniLionKangaroo* self);

	void updateMatrix();
	void updateShadow();
	void playWalk(int motion);
	void feed(const PlyInput* input, AniMove* move);
	void sound(int id);
	void bump(const Vec3f* pos);
	void land();
	void updateCharge();
	void clearCharge();
	int chargeFull();
	void useMove(AniMove* move);

	void runBirth(int phase);
	void runIdle(int phase);
	void runStand(int phase);
	void runWalk(int phase);
	void runJump(int phase);
	void runFall(int phase);
	void runChargeUp(int phase);
	void runShot(int phase);
	void runBreath(int phase);
	void runHurt(int phase);
	void runDead(int phase);

	s16 mCharge; // _9C frames B was held up to 60
	u8 _9E[2];
	s8 mStep;                    // _A0
	FlagByte mFlags2;            // _A1 a FlagByte 1 = its death is on the way
	s16 mFxBank;                 // _A2
	int mRotY;                   // _A4
	Vec3f mPos;                  // _A8
	f32 mMatrix[12];             // _B4
	MotionPlayer mMotion;        // _E4
	DrawTask* mDrawTask;         // _120
	AniHits mHits;               // _124
	AniBodyHit mBodyHit;         // _13C
	AniBody mBody;               // _174
	AniModel mModel;             // _314
	AniMotions mMotions;         // _330
	AniLife mLife;               // _340
	AniBlink mBlink;             // _34C
	ObjShadow mShadow;           // _364
	AniWalk mWalk;               // _39C
	AniJump mJump;               // _3D0
	AniMove* mMove;              // _3FC
	int mSweep;                  // _400 the breath's angle added to mRotY
	SoundHandle mSounds[2];      // _404
	TObject* mShotParent;        // _45C the fireballs hang from this
	Fx* mFx;                     // _460
	Fx* mChargeFx;               // _464
	TAniBirth* mBirth;           // _468
	int mHitValue;               // _46C
	TAniLionFireBreath* mBreath; // _470
};

extern "C" {
extern const char* gTAniLionKangarooName;
extern AniBodyOffset sLionBodyOffset;
extern const AniNode sLionKangarooModelNodes[3];
extern const MotionEntry sLionKangarooMotions[];
}

#endif
