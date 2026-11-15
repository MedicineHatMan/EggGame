#ifndef _ENEMY_BOSS_TENEYELLOWBOSSPURPLE_H_
#define _ENEMY_BOSS_TENEYELLOWBOSSPURPLE_H_
#include "Ninja/ninja.h"
struct DrawTask;
#include "egg/ObjEgg.h"
#include "enemy/boss/TEneYellowBossCrystal.h"
#include "sound/SeHolder.h"
struct TEneYellowBossPurple;
struct TObjAction;
struct TObject;

#include "core/types.h"

struct BossSuit;
struct CharaColli;
struct EnemyTouchList;
struct Vec3f;

extern "C" {
void crystalBeginDraw();
int crystalBreakFx(f32, const Vec3f*);
void crystalEndDraw();
void crystalSetTex(int);
void crystalSuitBegin(f32, BossSuit*);
void crystalTexScreen();
void crystalTouch(CharaColli*, EnemyTouchList*, f32);
}

/**
 * mPurpleFlags: 1 = skating, so eggs hurt, 8 = dying.
 *
 * @size 0x264
 */
class TEneYellowBossPurple : public TEneYellowBossCrystal {
public:
	TEneYellowBossPurple(TObject* parent, const Vec3f* pos);
	virtual ~TEneYellowBossPurple();
	virtual void draw();                                        // _0C 0x8014EA7C
	virtual void vfn_10();                                      // _10 0x8014F144
	virtual void render();                                      // _1C 0x8014F178
	virtual f32 vfn_40();                                       // _40 0x8014E7EC
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58 0x8014E750
	virtual void runAction(int action, int phase);              // _6C 0x8014EAFC
	virtual u8 isBroken();                                      // _70 0x8014E730

	void pose();                          // 0x8014E7F4
	void calcPoints();                    // 0x8014E840
	void syncShape();                     // 0x8014E8A0
	void bodyMatrix();                    // 0x8014E8E8
	void gravity(Vec3f* vel);             // 0x8014E928
	int move(Vec3f* vel);                 // 0x8014E94C
	int fall(Vec3f* vel);                 // 0x8014E9C4
	void slide(f32 drag);                 // 0x8014EA00
	void actBreak(int phase);             // 0x8014EB7C action 0
	void actTurn(int phase);              // 0x8014ECB8 action 1
	void actSkate(int phase);             // 0x8014ED94 action 2
	void actBrake(int phase);             // 0x8014EF94 action 3
	void actDie(int phase);               // 0x8014F07C action 4

	NJS_OBJECT* mModel;         // _1C4
	void* mMotions;       // _1C8
	CharaColli* mShape;   // _1CC
	GjActor* mAnim;          // _1D0
	DrawTask* mDraw;      // _1D4
	int mHits;            // _1D8 three and it breaks
	HitVec mPoint[6];     // _1DC
	u32 mPurpleFlags;     // _224
	SeHolder mSe;         // _228
	int mStep;            // _25C
	f32 mTime;            // _260
};

extern "C" {
extern u32 gTEneYellowBossPurpleName;
}

#endif
