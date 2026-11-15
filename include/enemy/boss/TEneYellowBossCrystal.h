#ifndef _TENEYELLOWBOSSCRYSTAL_H
#define _TENEYELLOWBOSSCRYSTAL_H

#include "object/ObjAction.h"
#include "stage/SetManager.h"
#include "engine/MotionPlayer.h"
#include "enemy/EnemyAnim.h"
#include "enemy/ObjEnemy.h"
#include "enemy/EnemyLight.h"
#include "enemy/EnemySight.h"
#include "enemy/boss/BossParts.h"
#include "engine/DrawTask.h"
#include "object/ObjCommon.h"

struct CharaColli;

/**
 * mCrystalFlags: 1 = sinking (drawn with the suit), 2 = hidden, 4 = kill asked for, 8 = can't act.
 * Vtable 0x802608E8.
 *
 * @size 0x1C4
 */
class TEneYellowBossCrystal : public TObjAction {
public:
	TEneYellowBossCrystal(TObject* parent); // 0x8014F238
	virtual ~TEneYellowBossCrystal();       // 0x8014F33C
	virtual Vec3f* getPos();                // _20 0x8014F21C
	virtual int getKind();                  // _2C 0x8014F214
	virtual int canAct();                   // _44 0x8014F224
	virtual void runAction(int action, int phase) = 0; // _6C
	virtual u8 isBroken() = 0;              // _70
	virtual void kill();                    // _74 0x8014C8F4

	void look();                            // 0x8014F3F8
	void startAction(int action);           // 0x8014F464
	void setNext(int action);               // 0x8014F4A4
	void clearNext();                       // 0x8014F4AC
	void checkAction();                     // 0x8014F4B8
	void switchAction();                    // 0x8014F4EC
	void stepAction();                      // 0x8014F55C
	void hide();                            // 0x8014F768
	void waitBreak(int* timer, f32* time);  // 0x8014F79C
	void show();                            // 0x8014F82C
	f32 startSink(f32 from, f32 to, f32 extra); // 0x8014F864
	void endSink();                         // 0x8014F910
	int isSinking();                        // 0x8014F920
	void cull(DrawTask* task, f32 far, f32 radius, Vec3f* pos, int arg); // 0x8014F92C

	SetTransform mXform;     // _30
	MotionPlayer mMotion;    // _74
	EnemyAnim mClip;         // _B0
	ObjShadow mShadow;       // _BC
	EnemyLight mLight;       // _F4
	BossSuit mSuit;          // _134
	EnemyTouchList mTouch;   // _16C
	EnemySight mSight;       // _178
	Vec3f mLookPos;          // _1A8 where the player was last seen
	int mLookAngle;          // _1B4
	u32 mCrystalFlags;       // _1B8
	int mAction;             // _1BC
	int mNextAction;         // _1C0 -1 = none
};

extern "C" {
void crystalTouch(CharaColli* colli, EnemyTouchList* list, f32 radius); // 0x8014F590
int crystalBreakFx(f32 radius, const Vec3f* pos);                       // 0x8014F6E4 frames to wait
void crystalTexScreen();                                                // 0x8014FA6C
void crystalBeginDraw();                                                // 0x8014FAC4
void crystalSetTex(int angle);                                          // 0x8014FB2C
void crystalEndDraw();                                                  // 0x8014FB4C
void crystalSuitBegin(f32 rise, BossSuit* suit);                        // 0x8014FB84
}

#endif
