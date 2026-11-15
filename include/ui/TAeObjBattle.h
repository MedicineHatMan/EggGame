#ifndef _TAEOBJBATTLE_H
#define _TAEOBJBATTLE_H
#include "core/types.h"
struct DrawTask;
struct BattlePlayerMsg;
struct BattleTimeMsg;

#include "ui/TAeLayer.h"

/**
 * Battle HUD. One draw task on list 0x3C. Layers are built by 0x801A7098 after
 * 0x801A73B8 clears the slots. draw() steps them and records a finish at _78.
 * vfn_20 handles message codes 0xC4, 0xC5 and 0xC6. Its bytes are at 0x80100444.
 *
 * @size 0x9C
 */
class TAeObjBattle : public TAeObject {
public:
	TAeObjBattle(TObject* parent, void* data, void* assets);
	virtual ~TAeObjBattle();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();           // _0C
	virtual void render();         // _1C
	virtual int vfn_20(s16* code); // _20 0x80100444

	DrawTask* mTask;  // _34
	TAeLayer* _38;    // _38
	TAeLayer* _3C;    // _3C
	TAeLayer* _40;    // _40
	TAeLayer* _44;    // _44
	TAeLayer* _48[4]; // _48
	TAeLayer* _58[4]; // _58
	u8 mRule;         // _68 1 survival 2 time 3 hatch scores shown
	u8 _69[3];        // _69
	int _6C;          // _6C
	u32 _70;          // _70
	u32 _74;          // _74
	u32 _78;          // _78
	int _7C[4];       // _7C scores clamped to 0..99
	s32 _8C[4];       // _8C
};

#include "stage/Stage.h"
#include "ui/TextVec.h"
class TAeLayer;

extern "C" {
void battlePlaceLayer(TAeObjBattle*, TAeLayer*, int);
void battleRuleHatch(TAeObjBattle*);
void battleRuleSurvival(TAeObjBattle*);
void battleRuleTime(TAeObjBattle*);
void battleSetPlayers(TAeObjBattle*, int);
void battleShowCount1(TAeObjBattle*);
void battleShowCount2(TAeObjBattle*);
void battleShowCount3(TAeObjBattle*);
void battleShowGo(TAeObjBattle*);
void battleShowNothing(TAeObjBattle*);
void battleShowReady(TAeObjBattle*);
void battleShowScore(TAeObjBattle*, int);
void battleShowSuddenDeath(TAeObjBattle*);
void battleShowTimeUp(TAeObjBattle*);
TAeObjBattle* createTAeObjBattle();
}

extern "C" {
int battleOnTimeLeft(TAeObjBattle*, BattleTimeMsg*);
int battleOnLeader(TAeObjBattle*, BattlePlayerMsg*);
int battleOnScore(TAeObjBattle*, BattlePlayerMsg*);
}

/**
 * The 0xC4 message: time left. @size 0xC
 */
struct BattleTimeMsg {
	u16 mKind;    // _00
	int mMinutes; // _04
	int mSeconds; // _08
};

/**
 * The 0xC5 (new leader) and 0xC6 (score change) messages. @size 0xC
 */
struct BattlePlayerMsg {
	u16 mKind;   // _00
	u16 _02;     // _02
	u16 mPlayer; // _04
	u16 _06;     // _06
	int mDelta;  // _08 0xC6 only
};

extern "C" {
extern const char* gTAeObjBattleName;
extern const u32 sAeBattleScoreLayers[];
extern const u32 sAeBattleScoreChangeLayers[];
extern const u32 sAeBattleIconCells[];
}

#endif
