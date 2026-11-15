#ifndef _AEOBJCONTROL_H
#define _AEOBJCONTROL_H
struct TAeObjCoinNum;
struct TAeObjPlayRest;
struct TArc;

#include "stage/Stage.h"

class TAeObjConsole1P;

/**
 * The console messages 0xAD-0xB8. Most name a player and carry one value. Names are ours.
 *
 * @size 0xC
 */
struct ConsoleMsg {
	s16 mCode;    // _00
	u8 _02[2];    // _02
	u16 mPlayer;  // _04 0-3
	u16 mValueLo; // _06 0xB1 only
	int mValue;   // _08
};

/**
 * Console message 0xAF. Names are ours.
 *
 * @size 0x10
 */
struct ConsoleRateMsg {
	s16 mCode;   // _00
	u8 _02[2];   // _02
	u16 mPlayer; // _04
	f32 mRate;   // _08
	s8 mFlag;    // _0C the egg kind
};

/**
 * Console message 0xB4, sent to the first console only. Names are ours.
 *
 * @size 0x10
 */
struct ConsoleTripleMsg {
	s16 mCode; // _00
	int mA;    // _04
	int mB;    // _08
	int mC;    // _0C
};

/**
 * Holds one TAeObjConsole1P per player. The mode picks how they are built:
 * 1 a single console, 2 one per joined player, 3 the same but numbered past the first,
 * 4 all numbered. vfn_20 passes console messages to the player they name. Name is ours.
 * Vtable 0x8025CC08.
 *
 * @size 0x30
 */
class TAeConsoleSet : public TObject {
public:
	TAeConsoleSet(TObject* parent, s8 mode);
	virtual ~TAeConsoleSet();

	virtual int vfn_20(s16* code); // _20

	void setupSolo();     // 0x800FFCBC
	void setupJoined();   // 0x800FFCFC
	void setupMixed();    // 0x800FFD80
	void setupNumbered(); // 0x800FFE20

	int onAD(s16* code); // 0x800FFEA4
	int onAE(s16* code); // 0x800FFEF4
	int onAF(s16* code); // 0x800FFEFC
	int onB0(s16* code); // 0x800FFF50
	int onB1(s16* code); // 0x800FFF58
	int onB2(s16* code); // 0x800FFFB0
	int onB3(s16* code); // 0x80100000
	int onB4(s16* code); // 0x8010004C
	int onB5(s16* code); // 0x8010008C
	int onB6(s16* code); // 0x801000DC
	int onB7(s16* code); // 0x80100128
	int onB8(s16* code); // 0x80100130

	TAeObjConsole1P* mConsoles[4]; // _20
};

class TAeObjBattle;
class TAeObjCombo;
class TAeObjMissionIcon;

extern "C" {
void aeBattleCount1();
void aeBattleCount2();
void aeBattleCount3();
void aeBattleGo();
void aeBattleNothing();
void aeBattleReady();
void aeBattleRuleHatch();
void aeBattleRuleSurvival();
void aeBattleRuleTime();
void aeBattleSuddenDeath();
void aeBattleTimeUp();
void aeClearPlayers();
void aeCreateBattle();
void aeCreateCoinNum();
void aeCreateCombo();
TAeConsoleSet* aeCreateConsoleSet(s8);
void aeCreateGameOver();
void aeCreateGo();
void aeCreateGoal();
void aeCreateMissionIcon0();
void aeCreateMissionIcon1();
void aeCreateMissionIcon2();
void aeCreateMissionIcon3();
void aeCreateMissionIcon4();
void aeCreateMissionIcon6();
void aeCreateMissionIcon7();
void aeCreateMissionIcon8();
void aeCreatePlayRest();
void aeCreateStageHud();
void aeCreateTimeUp();
void aeFadeComboCounts();
void aeForgetBattle(TAeObjBattle*);
void aeForgetCombo(TAeObjCombo*);
void aeForgetMissionIcon(TAeObjMissionIcon*);
void aeFreeBattle();
void aeFreeStage();
s8 aeGetPlayerCount();
s8 aeGetPlayerSlot(int);
void aeHideCoinNum();
void aeHidePlayRest();
void aeLoadBattle();
void aeLoadStage();
void aeSetCoinNum(u32);
void aeSetGaugeLevel(int);
void aeSetMissionIcon(TAeObjMissionIcon*);
void aeSetupBattle(int, int, int, int);
void aeShowComboCount(int);
void aeShowComboRank(int, int);
void aeShowPlayRest();
}

extern "C" {
int evSetRescue(int*);
}

extern "C" {
void aeLoadArc(const char*, TArc**);
void aeFreeArc(TArc**);
}

extern "C" {
void aeForgetCoinNum(TAeObjCoinNum*);
void aeForgetPlayRest(TAeObjPlayRest*);
}

#endif
