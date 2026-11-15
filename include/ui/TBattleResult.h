#ifndef _TBATTLERESULT_H
#define _TBATTLERESULT_H
struct TArc;
struct DrawTask;

#include "ui/TAeObject.h"

class TAeLayer;

/** What the results screen gets for one slot copied by setRow */
struct BattleResultRow {
	u8 mPlace;    // _00
	u16 mWins;    // _02
	u16 mPoints;  // _04
	u16 mLost;    // _06
	u16 _08;      // _08
};

typedef char BattleResultRow_size_check[(sizeof(BattleResultRow) == 10) ? 1 : -1];

/**
 * Battle results screen. mState walks 0 open, 1 wait, 2 read the pick, 3 hold.
 * Each of the four ports has a place layer, a rank layer, a 10-byte row, four
 * text pairs, and four 2-byte number buffers.
 *
 * @size 0x170
 */
class TBattleResult : public TAeObject {
public:
	TBattleResult(TObject* parent, void* data, void* assets);
	virtual ~TBattleResult();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void render(); // _1C

	void setup();                 // 0x801A5B60
	void waitOpen();              // 0x801A5F00
	void waitEnd();               // 0x801A6054
	void readPick();              // 0x801A60F0
	void reveal();                // 0x801A637C
	void open();                  // 0x801A6918
	void hideRow(s8 index);       // 0x801A6BBC
	void releaseRow(s8 index);    // 0x801A6CA8
	void setRow(u8 slot, const BattleResultRow* row); // 0x801A59AC

	DrawTask* mDraw;          // _34
	TAeLayer* mLayer;     // _38
	TAeLayer* mPlace[4];  // _3C
	TAeLayer* mRank[4];   // _4C
	u8 mState;            // _5C
	u8 _5D[3];            // _5D
	int mTimer;           // _60
	int mLive[4];         // _64 nonzero while that port is still in the round
	int mPicked[4];       // _74
	int mDone;            // _84
	BattleResultRow mRow[4]; // _88 mPlace is the result kind 0xFF means unused
	void* mText[4][4][2]; // _B0 four groups of four pairs
	char* mNum[4][4];     // _130 four groups of four buffers
};

class TObject;

extern "C" {
TBattleResult* createTBattleResult(TObject*);
void freeBattleResult();
void loadBattleResult();
}

extern "C" {
extern TArc* gBattleResultArc;
extern void* gBattleResultAe0;
extern void* gBattleResultAe1;
extern const char* gTBattleResultName;
extern const u32 gAeBattlePalettes[];
extern const u32 gAeBattleScoreSlots[6];
extern const u32 sBattleResultSlotsA[6];
extern const u32 sBattleResultSlotsB[6];
extern const u32 sBattleResultSlotsC[6];
extern const s16 sBattleResultPosX[6];
extern const s16 sBattleResultPosY0[6];
extern const s16 sBattleResultPosY1[6];
extern const s16 sBattleResultPosY2[6];
extern const s16 sBattleResultPosY3[6];
}

#endif
