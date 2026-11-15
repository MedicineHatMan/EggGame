#ifndef _STAGE_TSTAGESCORE_H_
#define _STAGE_TSTAGESCORE_H_
struct ScorePoints;
#include "Ninja/ninja.h"
#include "stage/Stage.h"
struct ScoreChain;
struct TStageScore;

#include "core/types.h"

struct StageRecord;

extern "C" {
void scoreAddBonus();
void scoreAddExtra(int);
void scoreAddTime(int);
void scoreCreate();
void scoreEgg(int);
void scoreGetRecord(StageRecord*);
int scoreRank(int, int, int, int, int);
void scoreSetRecord(StageRecord*);
int scoreWithTime(int, int, int);
void stageRecordClear(StageRecord*);
int stageRecordTotal(StageRecord*);
}

extern "C" {
int scoreNewChain(int);
}

extern "C" {
void scoreHit(int);
}

/** @size 0x14 */
struct ScoreChain {
	int mId;       // _00 0 when free
	int mKind;     // _04
	int mLastTime; // _08
	int mHits;     // _0C
	int mRank;     // _10 its place in the combo once it joined else 0
};

/**
 * Vtable 0x802679E8.
 * @size 0x1AC
 */
class TStageScore : public TObject {
public:
	TStageScore(TObject* parent); // 0x801B3388
	virtual ~TStageScore();       // 0x801B3458
	virtual void draw();          // 0x801B3AE0

	int hitsNeeded(int kind);     // 0x801B34CC
	void addKind(int kind);       // 0x801B34DC
	int comboPattern();           // 0x801B3578
	void addPatternBonus();       // 0x801B360C
	void touchCombo();            // 0x801B366C
	int newChain(int kind);       // 0x801B3678
	void hit(int id);             // 0x801B374C
	void egg(int fresh);          // 0x801B390C
	void addExtra(int points);    // 0x801B39B4
	void addBonus();              // 0x801B39C4
	void addTime(int clock);      // 0x801B39F0
	void getRecord(StageRecord* out);      // 0x801B3A28
	void setRecord(StageRecord* record);   // 0x801B3A84

	int mFrames;             // _20
	int mTime;               // _24 milliseconds
	ScoreChain mChains[16];  // _28
	int mCombo;              // _168
	int mComboTime;          // _16C
	int mComboWindow;        // _170 7000
	int mComboCount;         // _174
	int mKinds[3];           // _178 the kinds of the first three chains of the combo
	int mNextId;             // _184
	StageRecord mRecord;     // _188
	u32 mDirty;              // _1A8 1 = count changed 2 = combo changed
};

/** The points, a const table at 0x8024CED0. @size 0x30 */
struct ScorePoints {
	int mHit;         // _00 100
	int mHitCombo;    // _04 200 for each step of the combo
	int mEgg;         // _08 100
	int mEggCombo;    // _0C 200
	int _10;          // _10 200
	int mBonus;       // _14 1500 scoreAddBonus
	int mPerSecond;   // _18 300 scoreAddTime
	int mPattern[4];  // _1C by comboPattern 500 1000 1500 500
	int _2C;
};

extern "C" {
extern const ScorePoints sScorePoints;
extern TStageScore* sStageScore;
}

#endif
