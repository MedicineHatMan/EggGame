#ifndef _TAEOBJSTAGESELECT_H
#define _TAEOBJSTAGESELECT_H
struct TArc;

#include "ui/TAeLayer.h"

struct DrawTask;

/**
 * Stage select. A draw task on list 0x3C, a row of backdrop layers, eight pairs of
 * icon layers, and five more. draw() steps all of them. render() draws the backdrops.
 *
 * @size 0xB8
 */
class TAeObjStageSelect : public TAeObject {
public:
	TAeObjStageSelect(TObject* parent, void* data, void* assets);
	virtual ~TAeObjStageSelect();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void render(); // _1C

	void attachWorld();   // 0x8017D5C0
	void attachGba();     // 0x8017D6C8
	void attachOptions(); // 0x8017D7AC

	DrawTask* mTask;        // _34
	TAeLayer* mBack;        // _38 the backdrop mDone is set once it has played
	TAeLayer* mSide;        // _3C
	TAeLayer* mStatus;      // _40 the bar with the lives the emblems and the stage kind
	TAeLayer* mBase;        // _44
	TAeLayer* mPanel;       // _48 the world GBA or options panel with the others bound into it
	TAeLayer* mTint;        // _4C the panel's colour
	TAeLayer* mStageInfo;   // _50
	TAeLayer* mStageIcon[8]; // _54
	TAeLayer* mStageRing[8]; // _74
	TAeLayer* mRank;        // _94
	TAeLayer* mGbaRow[5];   // _98 one for each GBA game
	TAeLayer* mGbaCursor;   // _AC also the options panel's back button
	s8 mWorld;              // _B0
	s8 mStage;              // _B1
	s8 mGba;                // _B2 5 for the back button
	u8 _B3;
	BOOL mDone;             // _B4
};

extern "C" {
void aeStageSelectOpen(); // 0x8017C514
void aeStageSelectClose(); // 0x8017C588
BOOL aeStageSelectIsDone(); // 0x8017C5FC
void aeStageSelectShowWorld(s8 world); // 0x8017C618
void aeStageSelectHideWorld(); // 0x8017C69C
void aeStageSelectShowGba(); // 0x8017C6E8
void aeStageSelectHideGba(); // 0x8017C7A4
void aeStageSelectShowOptions(); // 0x8017C7F0
void aeStageSelectCloseAll(); // 0x8017C888
void aeStageSelectSetWorld(s8 world); // 0x8017C9C8
void aeStageSelectSetStage(s8 stage); // 0x8017CA90
void aeStageSelectSetGba(s8 gba); // 0x8017CBBC
void aeStageSelectClearStages(); // 0x8017CD48
void aeStageSelectSetStageIcon(s8 stage, BOOL cleared); // 0x8017CDE8
void aeStageSelectSetRank(s8 rank); // 0x8017CEB4
void aeStageSelectSetScore(int score); // 0x8017CF40
void aeStageSelectSetEggs(s8 eggs); // 0x8017D0A4
void aeStageSelectSetEggTotal(s8 total); // 0x8017D148
void aeStageSelectSetMinutes(s8 minutes); // 0x8017D1EC
void aeStageSelectSetSeconds(s8 seconds); // 0x8017D290
void aeStageSelectSetMsecs(s16 msecs); // 0x8017D334
void aeStageSelectLockGbaRows(); // 0x8017D3D8
void aeStageSelectOpenGbaRow(s8 gba); // 0x8017D464
void aeStageSelectSetStageKind(s8 kind); // 0x8017D4D4
void aeStageSelectSetLives(s8 lives); // 0x8017D528
void aeStageSelectSetEmblems(s8 emblems); // 0x8017D574
}

class TStage;

extern "C" {
TStage* createYellowStage();
void freeAeStageSelect();
void loadAeStageSelect();
}

extern "C" {
TAeObjStageSelect* createAeStageSelect();
}

#include "Ninja/njtypes.h"
extern "C" {
extern const char* gTAeObjStageSelectName;
extern TAeObjStageSelect* gAeStageSelect;
extern TArc* sAeStageSelectArc;
extern void* sAeStageSelectAe;
extern NJS_TEXLIST* sAeStageSelectTexList;
}

#endif
