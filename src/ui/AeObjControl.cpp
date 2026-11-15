#include "ui/AeObjControl.h"
#include "ui/TAeObjConsole1P.h"
#include "ui/TAeObjPlayRest.h"
#include "ui/TAeObjCoinNum.h"
#include "ui/TAeObjCombo.h"
#include "ui/TAeObjBattle.h"
#include "ui/TAeObjMissionIcon.h"
#include "core/BlockPool.h"
#include "io/Resource.h"
#include "Dolphin/os.h"
#include "ui/TAeObjMsgWindow.h"
#include "ui/TAeObjStageTitle.h"
#include "ui/TAeObjPause.h"
#include "enemy/boss/TEneYellowBossBlue.h"
#include "object/TObjMGLeader.h"
#include "ui/TBattleResult.h"
#include "ui/TResult.h"
#include "io/ResManager.h"
#include "core/TMainTask.h"
#include "ui/TAeObjTimeUp.h"
#include "ui/TAeObjGoal.h"
struct TAeObjGo;
struct TAeObjGameOver;
struct TAeObjTimeUp;
struct TAeObjGoal;

struct BattleTimeMsg;
struct BattlePlayerMsg;

extern "C" {
s8 sAeSlots[4];               // 0x802C5468 each player's console number -1 when out
s8 sAePlayerCount;            // 0x802C546C
BlockPool* sAeArcPool;        // 0x802C5470
void* sAeArcBlock;            // 0x802C5474
TAeConsoleSet* sConsoleSet;   // 0x802C5478
TAeObjPlayRest* sPlayRest;    // 0x802C547C
TAeObjCoinNum* sCoinNum;      // 0x802C5480
TAeObjMissionIcon* sMissionIcon; // 0x802C5484
TAeObjCombo* sCombo;          // 0x802C5488
TAeObjBattle* sBattle;        // 0x802C548C
int sAePlayers[4];            // 0x80282550 nonzero for each player in the battle
}

// 0x800FF0A4
extern "C" s8 aeGetPlayerSlot(int player)
{
	return sAeSlots[(s8)player];
}

// 0x800FF0B4
extern "C" s8 aeGetPlayerCount()
{
	return sAePlayerCount;
}

// 0x800FF0BC
extern "C" void aeLoadArc(const char* name, TArc** arc)
{
	sAeArcBlock = sAeArcPool->alloc();
	if (sAeArcBlock == 0) {
		OSReport("aeobj_control: cannot alloc block\n");
		*arc = 0;
	} else {
		readFromResPack(name, sAeArcBlock);
		*arc = new TArc(sAeArcBlock);
	}
}

// 0x800FF140
extern "C" void aeFreeArc(TArc** arc)
{
	releaseResource(*arc);
	sAeArcPool->release(sAeArcBlock);
	sAeArcBlock = 0;
}

// 0x800FF178
extern "C" void aeLoadStage()
{
	sAeArcPool  = new BlockPool(0x44000, 1);
	sAeArcBlock = 0;
	loadMsgWindow();
	stageTitleReset();
	loadConsole1P();
	loadMissionIcon();
	loadPause();
	initAeGo();
	initAeGameOver();
	loadResultText();
	loadCombo();
}

// 0x800FF1E8
extern "C" void aeLoadBattle()
{
	loadMsgWindow();
	loadConsole1P();
	loadBattleResult();
	loadPause();
	aeClearPlayers();
}

// 0x800FF218
extern "C" void aeFreeStage()
{
	freeMsgWindow();
	stageTitleRelease();
	freeConsole1P();
	freeMissionIcon();
	freePause();
	termAeGo();
	termAeGameOver();
	freeResultText();
	freeCombo();
	delete sAeArcPool;
	sAeArcPool = 0;
}

// 0x800FF280
extern "C" void aeFreeBattle()
{
	freeMsgWindow();
	freeConsole1P();
	freeBattleResult();
	freePause();
}

// 0x800FF2AC
extern "C" void aeCreateGo()
{
	if (loadAeGo()) {
		createAeGo();
	}
}

// 0x800FF2D8
extern "C" void aeCreateGameOver()
{
	if (loadAeGameOver()) {
		createAeGameOver();
	}
}

// 0x800FF304
extern "C" void aeCreateTimeUp()
{
	if (loadTimeUp()) {
		createTimeUp();
	}
}

// 0x800FF330
extern "C" void aeCreateGoal()
{
	if (loadGoal()) {
		createGoal();
	}
}

// 0x800FF35C
extern "C" void aeCreateStageHud()
{
	sConsoleSet = aeCreateConsoleSet(1);
	aeCreatePlayRest();
	aeCreateCoinNum();
	aeCreateCombo();
}

// 0x800FF390
extern "C" void aeSetupBattle(int p1, int p2, int p3, int p4)
{
	const int* slot;
	s8 count;
	int i;

	sAePlayers[0] = p1;
	sAePlayers[1] = p2;
	sAePlayers[2] = p3;
	sAePlayers[3] = p4;
	count = 0;
	for (i = 0; i < 4; i++) {
		if (sAePlayers[i] != 0) {
			count++;
		}
	}
	// The console numbers handed out in order for two three or four players
	sAePlayerCount = count;
	int two[2]   = { 0, 1 };
	int three[3] = { 0, 4, 5 };
	int four[4]  = { 2, 3, 4, 5 };
	if (count == 2) {
		slot = two;
	} else if (count == 3) {
		slot = three;
	} else {
		slot = four;
	}
	for (i = 0; i < 4; i++) {
		if (sAePlayers[i] != 0) {
			sAeSlots[i] = *slot++;
		} else {
			sAeSlots[i] = -1;
		}
	}
	sConsoleSet = aeCreateConsoleSet(count);
	aeCreateBattle();
}

// 0x800FF4CC
extern "C" void aeForgetConsoleSet(TAeConsoleSet* set)
{
	if (sConsoleSet == set) {
		sConsoleSet = 0;
	}
}

// 0x800FF4E4
extern "C" void aeCreatePlayRest()
{
	sPlayRest = createPlayRest(gTL_08);
}

// 0x800FF50C
extern "C" void aeForgetPlayRest(TAeObjPlayRest* rest)
{
	if (sPlayRest == rest) {
		sPlayRest = 0;
	}
}

// 0x800FF524
extern "C" void aeShowPlayRest()
{
	if (sPlayRest != 0) {
		sPlayRest->show();
	}
}

// 0x800FF550
extern "C" void aeHidePlayRest()
{
	if (sPlayRest != 0) {
		sPlayRest->hide();
	}
}

// 0x800FF57C
extern "C" void aeCreateCoinNum()
{
	sCoinNum = createCoinNum(gTL_08);
}

// 0x800FF5A4
extern "C" void aeForgetCoinNum(TAeObjCoinNum* coins)
{
	if (sCoinNum == coins) {
		sCoinNum = 0;
	}
}

// 0x800FF5BC
extern "C" void aeSetCoinNum(u32 count)
{
	if (sCoinNum != 0) {
		sCoinNum->show(count);
	}
}

// 0x800FF5F0
extern "C" void aeHideCoinNum()
{
	if (sCoinNum != 0) {
		sCoinNum->hide();
	}
}

// 0x800FF61C
extern "C" void aeSetMissionIcon(TAeObjMissionIcon* icon)
{
	sMissionIcon = icon;
}

// 0x800FF624
extern "C" void aeCreateMissionIcon0()
{
	sMissionIcon = createMissionIcon(0);
}

// 0x800FF64C
extern "C" void aeCreateMissionIcon1()
{
	sMissionIcon = createMissionIcon(1);
}

// 0x800FF674
extern "C" void aeCreateMissionIcon2()
{
	sMissionIcon = createMissionIcon(2);
}

// 0x800FF69C
extern "C" void aeCreateMissionIcon3()
{
	sMissionIcon = createMissionIcon(3);
}

// 0x800FF6C4
extern "C" void aeCreateMissionIcon4()
{
	sMissionIcon = createMissionIcon(4);
}

// 0x800FF6EC
extern "C" void aeCreateMissionIcon6()
{
	sMissionIcon = createMissionIcon(6);
}

// 0x800FF714
extern "C" void aeCreateMissionIcon7()
{
	sMissionIcon = createMissionIcon(7);
}

// 0x800FF73C
extern "C" void aeCreateMissionIcon8()
{
	sMissionIcon = createMissionIcon(8);
}

// 0x800FF764
extern "C" void aeForgetMissionIcon(TAeObjMissionIcon* icon)
{
	if (sMissionIcon == icon) {
		sMissionIcon = 0;
	}
}

// 0x800FF77C
extern "C" int aeSendMissionIcon(s16* code)
{
	if (sMissionIcon != 0) {
		return sMissionIcon->vfn_20(code);
	}
	return 0;
}

// 0x800FF7C4
extern "C" void aeSetGaugeLevel(int level)
{
	if (sMissionIcon != 0) {
		missionIconSetGaugeLevel(sMissionIcon, level);
	}
}

// 0x800FF7F8
extern "C" void aeCreateCombo()
{
	sCombo = createCombo(gTL_08);
}

// 0x800FF820
extern "C" void aeForgetCombo(TAeObjCombo* combo)
{
	if (sCombo == combo) {
		sCombo = 0;
	}
}

// 0x800FF838
extern "C" void aeShowComboRank(int rank, int word)
{
	if (sCombo != 0) {
		sCombo->showRank(rank, word);
	}
}

// 0x800FF874
extern "C" void aeShowComboCount(int count)
{
	if (sCombo != 0) {
		sCombo->showCount(count);
	}
}

// 0x800FF8A8
extern "C" void aeFadeComboCounts()
{
	if (sCombo != 0) {
		sCombo->fadeCounts();
	}
}

// 0x800FF8D4
extern "C" void aeClearPlayers()
{
	int i;

	for (i = 0; i < 4; i++) {
		sAePlayers[i] = 0;
		sAeSlots[i]   = -1;
	}
	sAePlayerCount = 0;
}

// 0x800FF914
extern "C" void aeCreateBattle()
{
	sBattle = createTAeObjBattle();
}

// 0x800FF938
extern "C" void aeBattleRuleSurvival()
{
	if (sBattle != 0) {
		battleRuleSurvival(sBattle);
	}
}

// 0x800FF964
extern "C" void aeBattleRuleTime()
{
	if (sBattle != 0) {
		battleRuleTime(sBattle);
	}
}

// 0x800FF990
extern "C" void aeBattleRuleHatch()
{
	if (sBattle != 0) {
		battleRuleHatch(sBattle);
	}
}

// 0x800FF9BC
extern "C" void aeForgetBattle(TAeObjBattle* battle)
{
	if (sBattle == battle) {
		sBattle = 0;
	}
}

// 0x800FF9D4
extern "C" void aeBattleReady()
{
	if (sBattle != 0) {
		battleShowReady(sBattle);
	}
}

// 0x800FFA00
extern "C" void aeBattleGo()
{
	if (sBattle != 0) {
		battleShowGo(sBattle);
	}
}

// 0x800FFA2C
extern "C" void aeBattleNothing()
{
	if (sBattle != 0) {
		battleShowNothing(sBattle);
	}
}

// 0x800FFA58
extern "C" void aeBattleCount3()
{
	if (sBattle != 0) {
		battleShowCount3(sBattle);
	}
}

// 0x800FFA84
extern "C" void aeBattleCount2()
{
	if (sBattle != 0) {
		battleShowCount2(sBattle);
	}
}

// 0x800FFAB0
extern "C" void aeBattleCount1()
{
	if (sBattle != 0) {
		battleShowCount1(sBattle);
	}
}

// 0x800FFADC
extern "C" void aeBattleTimeUp()
{
	if (sBattle != 0) {
		battleShowTimeUp(sBattle);
	}
}

// 0x800FFB08
extern "C" void aeBattleSuddenDeath()
{
	if (sBattle != 0) {
		battleShowSuddenDeath(sBattle);
	}
}

// 0x800FFB34
// the "set_rescue" event command
extern "C" int evSetRescue(int* args)
{
	aeSetGaugeLevel(args[0]);
	return 1;
}

// 0x800FFB5C
TAeConsoleSet::TAeConsoleSet(TObject* parent, s8 mode)
    : TObject(parent)
{
	switch (mode) {
	case 1:
		setupSolo();
		break;
	case 2:
		setupJoined();
		break;
	case 3:
		setupMixed();
		break;
	case 4:
		setupNumbered();
		break;
	}
}

// 0x800FFBF4
TAeConsoleSet::~TAeConsoleSet()
{
	aeForgetConsoleSet(this);
	mConsoles[0] = mConsoles[1] = mConsoles[2] = mConsoles[3] = 0;
}

// 0x800FFC6C
extern "C" TAeConsoleSet* aeCreateConsoleSet(s8 mode)
{
	return new TAeConsoleSet(gTL_05, mode);
}

// 0x800FFCBC
void TAeConsoleSet::setupSolo()
{
	mConsoles[0] = createConsole1P(this);
	mConsoles[1] = mConsoles[2] = mConsoles[3] = 0;
}

// 0x800FFCFC
void TAeConsoleSet::setupJoined()
{
	int i;

	for (i = 0; i < 4; i++) {
		if (sAePlayers[i] != 0) {
			mConsoles[i] = createConsole1PB(this, sAeSlots[i]);
		} else {
			mConsoles[i] = 0;
		}
	}
}

// 0x800FFD80
void TAeConsoleSet::setupMixed()
{
	int i;

	for (i = 0; i < 4; i++) {
		if (sAePlayers[i] != 0) {
			if (sAeSlots[i] == 0) {
				mConsoles[i] = createConsole1PB(this, sAeSlots[i]);
			} else {
				mConsoles[i] = createConsole1PC(this, sAeSlots[i]);
			}
		} else {
			mConsoles[i] = 0;
		}
	}
}

// 0x800FFE20
void TAeConsoleSet::setupNumbered()
{
	int i;

	for (i = 0; i < 4; i++) {
		if (sAePlayers[i] != 0) {
			mConsoles[i] = createConsole1PC(this, sAeSlots[i]);
		} else {
			mConsoles[i] = 0;
		}
	}
}

// 0x800FFEA4
int TAeConsoleSet::onAD(s16* code)
{
	ConsoleMsg* msg = (ConsoleMsg*)code;
	if (msg->mPlayer >= 4) {
		return 0;
	}
	if (mConsoles[msg->mPlayer] != 0) {
		mConsoles[msg->mPlayer]->setValue(msg->mValue);
	}
	return 1;
}

// 0x800FFEF4
int TAeConsoleSet::onAE(s16* code)
{
	return 1;
}

// 0x800FFEFC
int TAeConsoleSet::onAF(s16* code)
{
	ConsoleRateMsg* msg = (ConsoleRateMsg*)code;
	if (msg->mPlayer >= 4) {
		return 0;
	}
	if (mConsoles[msg->mPlayer] != 0) {
		mConsoles[msg->mPlayer]->setHatch(msg->mRate, msg->mFlag);
	}
	return 1;
}

// 0x800FFF50
int TAeConsoleSet::onB0(s16* code)
{
	return 1;
}

// 0x800FFF58
int TAeConsoleSet::onB1(s16* code)
{
	ConsoleMsg* msg = (ConsoleMsg*)code;
	if (msg->mPlayer >= 4) {
		return 0;
	}
	if (mConsoles[msg->mPlayer] != 0) {
		mConsoles[msg->mPlayer]->setStars(msg->mValueLo, *(u16*)&msg->mValue);
	}
	return 1;
}

// 0x800FFFB0
int TAeConsoleSet::onB2(s16* code)
{
	ConsoleMsg* msg = (ConsoleMsg*)code;
	if (msg->mPlayer >= 4) {
		return 0;
	}
	if (mConsoles[msg->mPlayer] != 0) {
		mConsoles[msg->mPlayer]->hitValue(msg->mValue);
	}
	return 1;
}

// 0x80100000
int TAeConsoleSet::onB3(s16* code)
{
	ConsoleMsg* msg = (ConsoleMsg*)code;
	if (msg->mPlayer >= 4) {
		return 0;
	}
	if (mConsoles[msg->mPlayer] != 0) {
		mConsoles[msg->mPlayer]->raiseLevel();
	}
	return 1;
}

// 0x8010004C
int TAeConsoleSet::onB4(s16* code)
{
	ConsoleTripleMsg* msg = (ConsoleTripleMsg*)code;
	if (mConsoles[0] != 0) {
		mConsoles[0]->setScore(msg->mA, msg->mB, msg->mC);
	}
	return 1;
}

// 0x8010008C
int TAeConsoleSet::onB5(s16* code)
{
	ConsoleMsg* msg = (ConsoleMsg*)code;
	if (msg->mPlayer >= 4) {
		return 0;
	}
	if (mConsoles[msg->mPlayer] != 0) {
		mConsoles[msg->mPlayer]->showBanner(msg->mValue);
	}
	return 1;
}

// 0x801000DC
int TAeConsoleSet::onB6(s16* code)
{
	ConsoleMsg* msg = (ConsoleMsg*)code;
	if (msg->mPlayer >= 4) {
		return 0;
	}
	if (mConsoles[msg->mPlayer] != 0) {
		mConsoles[msg->mPlayer]->hideBanner();
	}
	return 1;
}

// 0x80100128
int TAeConsoleSet::onB7(s16* code)
{
	return 1;
}

// 0x80100130
int TAeConsoleSet::onB8(s16* code)
{
	return 1;
}

// 0x80100138
extern "C" int aeSendMessage(s16* code)
{
	switch (*(u16*)code) {
	case 0xAD:
		if (sConsoleSet != 0) {
			return sConsoleSet->vfn_20(code);
		}
		break;
	case 0xAE:
		if (sConsoleSet != 0) {
			return sConsoleSet->vfn_20(code);
		}
		break;
	case 0xAF:
		if (sConsoleSet != 0) {
			return sConsoleSet->vfn_20(code);
		}
		break;
	case 0xB0:
		if (sConsoleSet != 0) {
			return sConsoleSet->vfn_20(code);
		}
		break;
	case 0xB1:
		if (sPlayRest != 0) {
			return sPlayRest->vfn_20(code);
		}
		if (sConsoleSet != 0) {
			return sConsoleSet->vfn_20(code);
		}
		break;
	case 0xB2:
		if (sConsoleSet != 0) {
			return sConsoleSet->vfn_20(code);
		}
		break;
	case 0xB3:
		if (sConsoleSet != 0) {
			return sConsoleSet->vfn_20(code);
		}
		break;
	case 0xB4:
		if (sConsoleSet != 0) {
			return sConsoleSet->vfn_20(code);
		}
		break;
	case 0xB5:
		if (sConsoleSet != 0) {
			return sConsoleSet->vfn_20(code);
		}
		break;
	case 0xB6:
		if (sConsoleSet != 0) {
			return sConsoleSet->vfn_20(code);
		}
		break;
	case 0xB7:
		if (sConsoleSet != 0) {
			return sConsoleSet->vfn_20(code);
		}
		break;
	case 0xB8:
		if (sConsoleSet != 0) {
			return sConsoleSet->vfn_20(code);
		}
		break;
	case 0xBE:
		if (sMissionIcon != 0) {
			return sMissionIcon->vfn_20(code);
		}
		break;
	case 0xBF:
		if (sMissionIcon != 0) {
			return sMissionIcon->vfn_20(code);
		}
		break;
	case 0xC0:
		if (sMissionIcon != 0) {
			return sMissionIcon->vfn_20(code);
		}
		break;
	case 0xC1:
		if (sMissionIcon != 0) {
			return sMissionIcon->vfn_20(code);
		}
		break;
	case 0xC2:
		if (sMissionIcon != 0) {
			return sMissionIcon->vfn_20(code);
		}
		break;
	case 0xC3:
		if (sMissionIcon != 0) {
			return sMissionIcon->vfn_20(code);
		}
		break;
	case 0xC4:
		if (sBattle != 0) {
			return sBattle->vfn_20(code);
		}
		break;
	case 0xC5:
		if (sBattle != 0) {
			return sBattle->vfn_20(code);
		}
		break;
	case 0xC6:
		if (sBattle != 0) {
			return sBattle->vfn_20(code);
		}
		break;
	}
	return 0;
}

// 0x80100444
int TAeObjBattle::vfn_20(s16* code)
{
	switch (*(u16*)code) {
	case 0xC4:
		return battleOnTimeLeft(this, (BattleTimeMsg*)code);
	case 0xC5:
		return battleOnLeader(this, (BattlePlayerMsg*)code);
	case 0xC6:
		return battleOnScore(this, (BattlePlayerMsg*)code);
	}
	return 0;
}

// 0x801004A4
int TAeObjPlayRest::vfn_20(s16* code)
{
	switch (*(u16*)code) {
	case 0xB1:
		return onRest(code);
	}
	return 1;
}

// 0x801004DC
int TAeConsoleSet::vfn_20(s16* code)
{
	switch (*(u16*)code) {
	case 0xAD:
		return onAD(code);
	case 0xAE:
		return onAE(code);
	case 0xAF:
		return onAF(code);
	case 0xB0:
		return onB0(code);
	case 0xB1:
		return onB1(code);
	case 0xB2:
		return onB2(code);
	case 0xB3:
		return onB3(code);
	case 0xB4:
		return onB4(code);
	case 0xB5:
		return onB5(code);
	case 0xB6:
		return onB6(code);
	case 0xB7:
		return onB7(code);
	case 0xB8:
		return onB8(code);
	default:
		return 1;
	}
}
