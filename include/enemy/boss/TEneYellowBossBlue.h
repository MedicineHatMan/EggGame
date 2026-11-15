#ifndef _ENEMY_BOSS_TENEYELLOWBOSSBLUE_H_
#define _ENEMY_BOSS_TENEYELLOWBOSSBLUE_H_
#include "Ninja/ninja.h"
struct DrawTask;
#include "egg/ObjEgg.h"
#include "enemy/boss/TEneYellowBossCrystal.h"
#include "stage/Stage.h"
struct CharaColli;
struct TEneYellowBossBlue;
struct TObjAction;
struct TObject;
struct TAeObjGo;

#include "core/types.h"

extern "C" {
void initAeGo();
int loadAeGo();
void releaseAeGo();
void termAeGo();
}

extern "C" {
TAeObjGo* createAeGo();
}

/**
 * mBlueFlags: 1 = done, 8 = dying.
 *
 * @size 0x254
 */
class TEneYellowBossBlue : public TEneYellowBossCrystal {
public:
	TEneYellowBossBlue(TObject* parent, const Vec3f* pos);
	virtual ~TEneYellowBossBlue();
	virtual void draw();                                        // _0C 0x80152534
	virtual void vfn_10();                                      // _10 0x80152B10
	virtual void render();                                      // _1C 0x80152B44
	virtual f32 vfn_40();                                       // _40 0x801523C8
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58 0x8015233C
	virtual void runAction(int action, int phase);              // _6C 0x801525BC
	virtual u8 isBroken();                                      // _70 0x80152330

	void pose();              // 0x801523D0
	void calcPoints();        // 0x80152424
	void syncShape();         // 0x80152484
	void bodyMatrix();        // 0x801524C8
	void dropShadow();        // 0x80152500
	void aimAt(Vec3f from, int angle, Vec3f* target, int* heading); // 0x80152728
	void actBreak(int phase); // 0x8015261C action 0
	void actShoot(int phase); // 0x801527C0 action 1
	void actDie(int phase);   // 0x80152A60 action 2

	NJS_OBJECT* mModel;       // _1C4
	void* mMotions;     // _1C8
	CharaColli* mShape;       // _1CC
	GjActor* mAnim;        // _1D0
	DrawTask* mDraw;    // _1D4
	HitVec mPoint[6];   // _1D8 [3] is the muzzle
	u32 mBlueFlags;     // _220
	int mHome;          // _224 its angle round the middle
	int mTargetAngle;   // _228 the heading to the target
	Vec3f mTarget;      // _22C
	int mAimAngle;      // _238 the target's angle round the ring
	int mRounds;        // _23C
	int mStep;          // _240
	int mShots;         // _244
	int mTurnStep;      // _248
	int mAimStep;       // _24C how far round the ring each shot moves
	f32 mTime;          // _250
};

extern "C" {
extern u32 gTEneYellowBossBlueName;
}

#endif
