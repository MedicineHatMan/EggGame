#ifndef _ENEMY_BOSS_TENEYELLOWBOSSGREEN_H_
#define _ENEMY_BOSS_TENEYELLOWBOSSGREEN_H_
struct TArc;
struct GreenMotion;
struct StoryData;
struct StoryProgram;
struct StorySeq;
#include "Ninja/ninja.h"
#include "egg/ObjEgg.h"
#include "enemy/boss/TEneYellowBossCrystal.h"
#include "engine/FxHolder.h"
#include "engine/dlist.h"
#include "sound/SeHolder.h"
struct CharaColli;
struct DrawTask;
struct TEneYellowBossGreen;
struct TObjAction;
struct TObject;
struct Vec3f;

#include "core/types.h"

extern "C" {
void loadYellowBossGreen();
void releaseYellowBossGreen();
void storySeqInit();
}

extern "C" {
int storySeqEval(int);
}

/**
 * mGreenFlags: 8 = dying.
 *
 * @size 0x2FC
 */
class TEneYellowBossGreen : public TEneYellowBossCrystal {
public:
	TEneYellowBossGreen(TObject* parent, const Vec3f* pos);
	virtual ~TEneYellowBossGreen();
	virtual void draw();                                              // _0C 0x801A9B14
	virtual void vfn_10();                                            // _10 0x801AA3F4
	virtual void render();                                            // _1C 0x801AA428
	virtual f32 vfn_40();                                             // _40 0x801A96A4
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58 0x801A959C
	virtual void runAction(int action, int phase);                    // _6C 0x801A9BA0
	virtual u8 isBroken();                                            // _70 0x801A9568

	void pose();                                             // 0x801A96AC
	void calcPoints();                                       // 0x801A96F8
	void syncShape();                                        // 0x801A9758
	void syncSwing();                                        // 0x801A97B0
	void bodyMatrix();                                       // 0x801A9810
	void groundShadow();                                     // 0x801A9850
	int probeGround(Vec3f* from, Vec3f* pos, Vec3f* normal); // 0x801A98D8
	void stepNode(int node, Vec3f* vel);                     // 0x801A9980
	void drag(f32 rate);                                     // 0x801A9A14
	int move(Vec3f* vel);                                    // 0x801A9A30
	void slide(f32 rate);                                    // 0x801A9AA4
	f32 motionRate(s16 entry);                               // 0x801A9AF0
	int turnTo(int* angle, f32* step, int rate, int target); // 0x801A9D5C
	int isNear(Vec3f* at);                                   // 0x801A9F6C
	int isAhead(int angle);                                  // 0x801A9FA4
	void actBreak(int phase);                                // 0x801A9C20 action 0
	void actTurn(int phase);                                 // 0x801A9E74 action 1
	void actWalk(int phase);                                 // 0x801A9FC4 action 2
	void actSwing(int phase);                                // 0x801AA12C action 3
	void actDie(int phase);                                  // 0x801AA310 action 4

	u8 _1C4[8];
	CharaColli* mShape; // _1CC
	CharaColli* mSwing; // _1D0 the swing's sphere on only in the swing
	GjActor* mAnim;     // _1D4
	DrawTask* mDraw;    // _1D8
	int mHits;          // _1DC three swings and it breaks
	int mFrames;        // _1E0 600 and it breaks too
	f32 mSpeed;         // _1E4
	HitVec mPoint[13];  // _1E8
	u32 mGreenFlags;    // _284
	SeHolder mSe;       // _288
	int mStep;          // _2BC
	f32 mTime;          // _2C0
	int mSwingAngle;    // _2C4
	FxAttach mTrail;    // _2C8
};

// One row of the green crystal's motion table
struct GreenMotion { // size 0x1C a MotionEntry
	s8 mType;        // _00
	s16 mMotion;     // _02
	s16 mShape;      // _04
	s16 mNext;       // _06
	f32 mStart;      // _08
	f32 mEnd;        // _0C
	f32 mRate;       // _10
	f32 mBlendRate;  // _14
	int mUser;       // _18
};

// One program its code and how many words of the stack it takes and keeps
struct StoryProgram { // size 0xC
	u8* mCode;        // _00
	int mArgs;        // _04 words below the stack top that are its own
	int mLocals;      // _08 words it needs cleared
};

struct StoryData {
	u8 _00[8];
	u32 mCount;              // _08
	StoryProgram* mPrograms; // _0C
};

struct StorySeq { // size 0xC
	StorySeq();   // 0x801AAA6C

	int eval(int id); // 0x801AAAAC

	StoryData* mData; // _00
	u32 _04;
	int* mStack; // _08
};

#include "io/ResBank.h"
extern "C" {
extern const char* gTEneYellowBossGreenName;
extern ResBank gYellowGreenBank;
extern GreenMotion sYellowBossGreenMotions[];
extern TArc* gStorySeqArc;
extern StorySeq* gStorySeq;
}

#endif
