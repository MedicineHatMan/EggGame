#ifndef _ENEMY_BOSS_TENEYELLOWBOSSORANGE_H_
#define _ENEMY_BOSS_TENEYELLOWBOSSORANGE_H_
#include "Ninja/ninja.h"
struct DrawTask;
#include "egg/ObjEgg.h"
#include "enemy/boss/TEneYellowBossCrystal.h"
struct CharaColli;
struct TEneYellowBossOrange;
struct TEneYellowBossTama_Orange;
struct TObjAction;
struct TObject;

#include "core/types.h"

struct ObjShadow;
struct Vec3f;

extern "C" {
int crystalGroundAt(const Vec3f*, f32*, ObjShadow*);
}

/**
 * mOrangeFlags: 1 = done, 8 = dying.
 *
 * @size 0x2B4
 */
class TEneYellowBossOrange : public TEneYellowBossCrystal {
public:
	TEneYellowBossOrange(TObject* parent, const Vec3f* pos);
	virtual ~TEneYellowBossOrange();
	virtual void draw();                                              // _0C 0x80150200
	virtual void vfn_10();                                            // _10 0x80150748
	virtual void render();                                            // _1C 0x8015077C
	virtual f32 vfn_40();                                             // _40 0x8014FED8
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58 0x8014FE4C
	virtual void runAction(int action, int phase);                    // _6C 0x80150280
	virtual u8 isBroken();                                            // _70 0x8014FE40

	void pose();                                    // 0x8014FEE0
	void calcPoints();                              // 0x8014FF30
	void syncShape();                               // 0x8014FF90
	void bodyMatrix();                              // 0x8014FFBC
	void walkFoot(int foot, f32 scale, Vec3f* vel); // 0x8014FFFC moves by what the planted foot slid
	void keepOnRing();                              // 0x801500A4
	void dropShadow();                              // 0x801501BC
	void actBreak(int phase);                       // 0x801502E0 action 0
	void actThrow(int phase);                       // 0x80150408 action 1
	void actDie(int phase);                         // 0x80150698 action 2

	NJS_OBJECT* mModel;               // _1C4
	void* mMotions;                   // _1C8
	CharaColli* mShape;               // _1CC
	GjActor* mAnim;                   // _1D0
	DrawTask* mDraw;                  // _1D4
	TEneYellowBossTama_Orange* mTama; // _1D8 the shot in its hand
	f32 mSpeed;                       // _1DC of the walk
	int mThrows;                      // _1E0 every third goes to a random point
	HitVec mPoint[16];                // _1E4
	u32 mOrangeFlags;                 // _2A4
	int mStep;                        // _2A8 the break's wait then the walk's time left
	int mHandStep;                    // _2AC
	f32 mTime;                        // _2B0
};

extern "C" {
extern u32 gTEneYellowBossOrangeName;
}

#endif
