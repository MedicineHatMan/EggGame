#ifndef _TRESULT_H
#define _TRESULT_H
#include "core/types.h"
#include "stage/Stage.h"

#include "ui/TAeObject.h"

class TAeLayer;
class TSimpleText;
struct DrawTask;

/**
 * The results screen at the end of a stage: the time, the four scores counting up, the total, a new best, the rank,
 * then any emblem and the messages that go with what was won. The stage (Stage.cpp) fills in the record and flags,
 * and waits for mDone. The name is the DOL's.
 *
 * The screen runs as a list of states. Each state function gets phase 0 on entering, 1 to decide what comes next,
 * 2 to animate and 3 on leaving. Pressing any button while the numbers count skips to state 11, which shows them all.
 *
 * @size 0xE8
 */
class TResult : public TAeObject {
public:
	TResult(TObject* parent, void* data, void* assets);
	virtual ~TResult();

	virtual void draw();   // _0C steps the states
	virtual void render(); // _1C

	int pressedA();
	int pressedAny();
	void closeMessage();
	void showMessage(void* text);
	int messageDone();
	void setState(int state);
	void runState(int state, int phase);

	void runOpen(int phase);     // 0
	void runTitle(int phase);    // 1
	void runShow(int phase);     // 2
	void runScore0(int phase);   // 3
	void runScore1(int phase);   // 4
	void runScore2(int phase);   // 5
	void runScore3(int phase);   // 6
	void runTotal(int phase);    // 7
	void runBest(int phase);     // 8 a new best
	void runRank(int phase);     // 9
	void runSkip(int phase);     // 11 shows everything at once
	void runWait(int phase);     // 10 waits for A
	void runClose(int phase);    // 12
	void runNext(int phase);     // 13 picks what to show next from mFlags
	void runEmblem(int phase);   // 14
	void runMessage(int phase);  // 15 one of the messages for bits 1-8
	void runMessage2(int phase); // 16 one of the messages for bits 9-12
	void runEnd(int phase);      // 17 fades out and sets mDone

	void showText(int index);
	void showTexts();
	void styleSmallText(int index);
	void hideTexts();
	void showCounts();
	void showNumber(u32 slot, int value);
	void showTime();
	void showEmblems();

	// Not in the DOL as functions of their own
	inline void runScore(int phase, int index);
	inline void updateRank();

	StageTime mTime;          // _34
	u8 _3A[2];
	StageRecord mRecord;      // _3C
	int mShown;               // _5C the number counting up
	int mRank;                // _60 0 to 4
	int mCleared;             // _64 emblems won so far
	int mNewBest;             // _68
	u32 mFlags;               // _6C what to tell the player bit 0 an emblem 1-8 and 9-12 messages
	TAeLayer* mBase;          // _70
	TAeLayer* mTitle;         // _74
	TAeLayer* mBest;          // _78
	TAeLayer* mRankLayer;     // _7C
	TAeLayer* mPrompt;        // _80
	TAeLayer* mDigits[2];     // _84 the emblem count ones then tens
	TAeLayer* mEmblem;        // _8C
	DrawTask* mDrawTask;      // _90
	int mState;               // _94
	int mNextState;           // _98
	int mWait;                // _9C
	int mDone;                // _A0
	TSimpleText* mTexts[13];  // _A4
	s8 mMsgOpen;              // _D8
	int mMsgWait;             // _DC
	TSimpleText* mMsg;        // _E0
	int mTensRoll;            // _E4 the emblem count's tens digit turns over too
};

void loadResultText();                  // 0x801C78F4 mes_result.bin
void freeResultText();                  // 0x801C798C
void loadResult();                      // 0x801C79C0 ae_result.arc
void freeResult();                      // 0x801C7AD8
TResult* createResult(TObject* parent); // 0x801C7B24 NULL if ae_result.arc is not loaded

extern "C" {
extern const char* gTResultName;
}

#endif
