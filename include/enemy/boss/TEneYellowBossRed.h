#ifndef _ENEMY_BOSS_TENEYELLOWBOSSRED_H
#define _ENEMY_BOSS_TENEYELLOWBOSSRED_H
#include "Ninja/ninja.h"
struct DrawTask;
#include "egg/ObjEgg.h"
#include "enemy/boss/TEneYellowBossCrystal.h"
#include "stage/Stage.h"
struct CharaColli;
struct TEneYellowBossRed;
struct TObjAction;
struct TObject;

#include "core/types.h"

/**
 * mRedFlags: 1 = done, 8 = dying.
 *
 * @size 0x208
 */
class TEneYellowBossRed : public TEneYellowBossCrystal {
public:
	TEneYellowBossRed(TObject* parent, const Vec3f* pos);
	virtual ~TEneYellowBossRed();
	virtual void draw();                                              // _0C 0x8016DE0C
	virtual void vfn_10();                                            // _10 0x8016E43C
	virtual void render();                                            // _1C 0x8016E470
	virtual f32 vfn_40();                                             // _40 0x8016DC70
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58 0x8016DBE4
	virtual void runAction(int action, int phase);                    // _6C 0x8016DE8C
	virtual u8 isBroken();                                            // _70 0x8016DBD8

	void pose();              // 0x8016DC78
	void calcPoint();         // 0x8016DCC4
	void syncShape();         // 0x8016DD08
	void bodyMatrix();        // 0x8016DD7C
	void dropShadow();        // 0x8016DDBC
	void actBreak(int phase); // 0x8016DF0C action 0
	void actMove(int phase);  // 0x8016E054 action 1
	void actShoot(int phase); // 0x8016E1FC action 2
	void actEnd(int phase);   // 0x8016E2F8 action 3
	void actDie(int phase);   // 0x8016E38C action 4

	NJS_OBJECT* mModel; // _1C4
	void* mMotions;     // _1C8
	CharaColli* mShape; // _1CC
	GjActor* mAnim;     // _1D0
	DrawTask* mDraw;    // _1D4
	int mShots;         // _1D8
	int mHeading;       // _1DC where on the ring it goes next
	Vec3f mGoal;        // _1E0
	HitVec mPoint[1];   // _1EC the beam's start
	u32 mRedFlags;      // _1F8
	int mStep;          // _1FC
	int mTimer;         // _200
	f32 mTime;          // _204
};

extern "C" {
extern const char* gTEneYellowBossRedName;
extern const int gYellowBossRedNodes[2];
}

#endif
