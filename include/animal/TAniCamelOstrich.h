#ifndef _ANIMAL_TANICAMELOSTRICH_H_
#define _ANIMAL_TANICAMELOSTRICH_H_
#include "Ninja/ninja.h"
#include "animal/AniMount.h"
#include "animal/AniFlagBits.h"
#include "engine/DrawTask.h"
#include "engine/Fx.h"
#include "engine/MotionPlayer.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "player/PlyInput.h"
#include "sound/SoundHandle.h"
#include "stage/Stage.h"
struct TAniCamelOstrich;

#include "core/types.h"

class TObject;

extern "C" {
void* createCamelOstrich(TObject*);
}

/**
 * @size 0x4C0
 */
class TAniCamelOstrich : public TAniMount {
public:
	TAniCamelOstrich(TObject* parent);
	virtual ~TAniCamelOstrich();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void vfn_10();
	virtual Vec3f* getPos() // 0x801C08F8
	{
		return &mPos;
	}
	virtual int getRotY() // 0x801C0900
	{
		return mRotY;
	}
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

	static void drawTask(TAniCamelOstrich* self);

	void updateMatrix();
	void updateShadow();
	void playWalk(int motion);
	void feed(const PlyInput* input, AniMove* move);
	void sound(int id);
	void bump(const Vec3f* pos);
	void dust(const Vec3f* pos);
	void land();
	void useMove(AniMove* move);

	void runBirth(int phase);
	void runIdle(int phase);
	void runStand(int phase);
	void runWalk(int phase);
	void runDash(int phase);
	void runJump(int phase);
	void runFall(int phase);
	void runCharge(int phase);
	void runSpit(int phase);
	void runBreath(int phase);
	void runBreathEnd(int phase);
	void runHurt(int phase);
	void runDead(int phase);

	s8 mStep;              // _9C
	FlagByte mFlags2;      // _9D a FlagByte 1 = its death is on the way
	s16 mTimer;            // _9E
	s16 mBreathTime;       // _A0
	s16 mFxBank;           // _A2
	Vec3f mPos;            // _A4
	int mRotY;             // _B0
	f32 mMatrix[12];       // _B4
	MotionPlayer mMotion;  // _E4
	DrawTask* mDrawTask;   // _120
	AniHits mHits;         // _124 0x18
	AniBodyHit mBodyHit;   // _13C
	AniReachHit mReachHit; // _174
	AniBody mBody;         // _1B0
	AniModel mModel;       // _350
	AniMotions mMotions;   // _36C
	AniLife mLife;         // _37C
	AniBlink mBlink;       // _388
	ObjShadow mShadow;     // _3A0
	AniMove* mMove;        // _3D8
	AniWalk mWalk;         // _3DC
	AniJump mJump;         // _410
	AniDash mDash;         // _43C
	AniBlast mBlast;       // _460
	Fx* mBreathFx;         // _488
	SoundHandle mSound[1]; // _48C
	TAniBirth* mBirth;     // _4B8
	int mHitValue;         // _4BC
};

extern "C" {
extern const char* gTAniCamelOstrichName;
extern AniBodyOffset sCamelBodyOffset;
extern const AniNode sCamelOstrichModelNodes[3];
extern const MotionEntry sCamelOstrichMotions[];
}

#endif
