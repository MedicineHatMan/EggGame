#ifndef _TAEOBJBATTLESELECT_H
#define _TAEOBJBATTLESELECT_H
struct TArc;

#include "ui/TAeLayer.h"
struct DrawTask;

/**
 * Battle select. Two draw tasks, on lists 0x3C and 0x3E, and a grid of layers.
 * draw() steps every layer. The destructor clears a global and leaves the tasks.
 *
 * @size 0xB4
 */
class TAeObjBattleSelect : public TAeObject {
public:
	TAeObjBattleSelect(TObject* parent, void* data, void* assets);
	virtual ~TAeObjBattleSelect();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw(); // _0C

	DrawTask* mTask;  // _34
	void* mTask2;     // _38
	TAeLayer* _3C;    // _3C
	TAeLayer* _40;    // _40
	TAeLayer* _44;    // _44
	TAeLayer* _48;    // _48
	TAeLayer* _4C[4]; // _4C
	TAeLayer* _5C[4]; // _5C
	TAeLayer* _6C[4]; // _6C
	TAeLayer* _7C[4]; // _7C
	TAeLayer* _8C[4]; // _8C
	TAeLayer* _9C;    // _9C
	TAeLayer* _A0;    // _A0
	TAeLayer* _A4;    // _A4
	TAeLayer* _A8;    // _A8
	u8 _AC[4];        // _AC
	u8 _B0;           // _B0
	u8 _B1;           // _B1
	u8 _B2[2];        // _B2
};

// The battle select screen's free functions
void loadBattleSelect();                  // 0x80196620 loads ae_battle_menu.arc
void freeBattleSelect();                  // 0x801967F8
TAeObjBattleSelect* createBattleSelect(); // 0x80196844
void aeBattleSelectSetDigits(s8 index, u8 value);
void aeBattleSelectShowIcon(s8 row, s8 which);
void aeBattleSelectClearIcons();
void aeBattleSelectJoin(s8 slot, s8 other);
void aeBattleSelectLeave(s8 slot, s8 other);
void aeBattleSelectBindIcon(s8 row, s8 which);
void aeBattleSelectClearRow(s8 row);
void aeBattleSelectBindBanner();
void aeBattleSelectResetBanner();
void aeBattleSelectClearBanner();
void aeBattleSelectEnterChara();
void aeBattleSelectLeaveChara();
void aeBattleSelectEnterRules();
void aeBattleSelectLeaveRules();
void aeBattleSelectPickRule(s8 choice);
void aeBattleSelectBindPairB(s8 choice);
void aeBattleSelectBindPairC(s8 choice);
void aeBattleSelectShowStage(s8 choice);
void aeBattleSelectOpenStage(s8 choice);

extern "C" {
extern const char* gTAeObjBattleSelectName;
extern TAeObjBattleSelect* sAeBattleSelectScreen;
extern TArc* sAeBattleSelectArc;
extern u32 sAeBattleSelectData;
extern u32 sAeBattleSelectTex;
}

#endif
