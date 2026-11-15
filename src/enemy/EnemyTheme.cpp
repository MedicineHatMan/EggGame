#include "stage/SetManager.h"
#include "enemy/EnemyCommon.h"
#include "core/dolphin.h"
#include "io/ResBank.h"
#include "egg/EggCreature.h"
#include "enemy/EnemyHit.h"
#include "enemy/TEneAm02.h"
#include "enemy/TEneAm06.h"
#include "enemy/TEneAm07.h"
#include "enemy/TEneAm08.h"
#include "enemy/TEneAntsQueen.h"
#include "enemy/TEnePenguin.h"
#include "enemy/TEneBat.h"
#include "enemy/TEneFrog.h"
#include "enemy/TEneArmadillo.h"
#include "enemy/boss/TEneBoss02.h"
#include "enemy/boss/TEneBoss03Mole.h"
#include "enemy/boss/TEnePurpleBoss.h"
#include "enemy/boss/TEneOrangeBoss.h"
#include "enemy/boss/TEneYellowBoss.h"
#include "enemy/boss/TEneLastBoss.h"
#include "enemy/boss/TEneLastExBoss.h"
#include "enemy/boss/TEneGreenBoss.h"
#include "enemy/TEneGreenDemo.h"
#include "enemy/TEneAm01.h"
#include "enemy/Enemy.h"
#include "core/Mem.h"
#include "enemy/TEneGroupCollision.h"
#include "camera/TCamera.h"
struct ResBank;

// Enemy classes are a second registry beside SetManager's object tables
// The class name is the string a constructor stores at this+0
// This file is the theme tables and the loader 0x8001CE24-0x8001D038

static const f32 kEnemyRange = 90000.0f;

static SetInitPair sCommonInitPairs[] = {
	{ enemyParamLoad, enemyParamRelease },
	{ enemyCommonLoad, enemyCommonRelease },
};
static SetInitList sCommonInits = { sCommonInitPairs, 2 };
static SetInitPair sGreenInitPairs[] = {
	{ initZakoMotions, shutdownZakoMotions },
	{ loadZakoCat, 0 },
	{ loadZakoAnt, 0 },
	{ initTEneAm02, shutdownTEneAm02 },
	{ initTEneAm07, shutdownTEneAm07 },
	{ initTEneAntsQueen, shutdownTEneAntsQueen },
	{ greenDemoBankInit, greenDemoBankExit },
};
static SetInitList sGreenInits = { sGreenInitPairs, 7 };
static SetInitPair sBlueInitPairs[] = {
	{ initZakoMotions, shutdownZakoMotions },
	{ loadZakoSnake, 0 },
	{ loadZakoFugu, 0 },
	{ initTEneAm06, shutdownTEneAm06 },
	{ initTEnePenguin, shutdownTEnePenguin },
	{ initTEneFrog, shutdownTEneFrog },
};
static SetInitList sBlueInits = { sBlueInitPairs, 6 };
static SetInitPair sRedInitPairs[] = {
	{ initZakoMotions, shutdownZakoMotions },
	{ loadZakoSnake, 0 },
	{ loadZakoBat, 0 },
	{ initTEneAm08, shutdownTEneAm08 },
	{ initTEneBat, shutdownTEneBat },
	{ initTEneArmadillo, shutdownTEneArmadillo },
};
static SetInitList sRedInits = { sRedInitPairs, 6 };
static SetInitPair sPurpleInitPairs[] = {
	{ initZakoMotions, shutdownZakoMotions },
	{ loadZakoCat, 0 },
	{ loadZakoFugu, 0 },
	{ initTEneAm06, shutdownTEneAm06 },
	{ initTEnePenguin, shutdownTEnePenguin },
	{ initTEneAm02, shutdownTEneAm02 },
};
static SetInitList sPurpleInits = { sPurpleInitPairs, 6 };
static SetInitPair sOrangeInitPairs[] = {
	{ initZakoMotions, shutdownZakoMotions },
	{ loadZakoSnake, 0 },
	{ loadZakoAnt, 0 },
	{ initTEneAm02, shutdownTEneAm02 },
	{ initTEneAntsQueen, shutdownTEneAntsQueen },
	{ initTEnePenguin, shutdownTEnePenguin },
};
static SetInitList sOrangeInits = { sOrangeInitPairs, 6 };
static SetInitPair sYellowInitPairs[] = {
	{ initZakoMotions, shutdownZakoMotions },
	{ loadZakoCat, 0 },
	{ loadZakoAnt, 0 },
	{ loadZakoBat, 0 },
	{ initTEneAm07, shutdownTEneAm07 },
	{ initTEneAm08, shutdownTEneAm08 },
	{ initTEneArmadillo, shutdownTEneArmadillo },
};
static SetInitList sYellowInits = { sYellowInitPairs, 7 };
static SetInitPair sLastInitPairs[] = {
	{ initZakoMotions, shutdownZakoMotions },
	{ loadZakoCat, 0 },
	{ loadZakoAnt, 0 },
	{ loadZakoBat, 0 },
	{ initTEneAm02, shutdownTEneAm02 },
	{ initTEneAm08, shutdownTEneAm08 },
	{ initTEneBat, shutdownTEneBat },
};
static SetInitList sLastInits = { sLastInitPairs, 7 };
static SetInitPair sGreenBossInitPairs[] = {
	{ initTEneGreenBoss, shutdownTEneGreenBoss },
};
static SetInitList sGreenBossInits = { sGreenBossInitPairs, 1 };
static SetInitPair sBlueBossInitPairs[] = {
	{ initTEneBoss02, shutdownTEneBoss02 },
};
static SetInitList sBlueBossInits = { sBlueBossInitPairs, 1 };
static SetInitPair sRedBossInitPairs[] = {
	{ initTEneBoss03Mole, shutdownTEneBoss03Mole },
};
static SetInitList sRedBossInits = { sRedBossInitPairs, 1 };
static SetInitPair sPurpleBossInitPairs[] = {
	{ initTEnePurpleBoss, shutdownTEnePurpleBoss },
};
static SetInitList sPurpleBossInits = { sPurpleBossInitPairs, 1 };
static SetInitPair sOrangeBossInitPairs[] = {
	{ initTEneOrangeBoss, shutdownTEneOrangeBoss },
};
static SetInitList sOrangeBossInits = { sOrangeBossInitPairs, 1 };
static SetInitPair sYellowBossInitPairs[] = {
	{ initTEneYellowBoss, shutdownTEneYellowBoss },
};
static SetInitList sYellowBossInits = { sYellowBossInitPairs, 1 };
static SetInitPair sLastBossInitPairs[] = {
	{ initTEneLastBoss, shutdownTEneLastBoss },
};
static SetInitList sLastBossInits = { sLastBossInitPairs, 1 };
static SetInitPair sLastBoss2InitPairs[] = {
	{ initTEneLastExBoss, shutdownTEneLastExBoss },
};
static SetInitList sLastBoss2Inits = { sLastBoss2InitPairs, 1 };

static SetClass sCommonClassesTable[] = {
	{ 0x0002, createTEneGroupCollision, 0, kEnemyRange },
};
static SetClassList sCommonClasses = { sCommonClassesTable, 1 };
static SetClass sGreenClassesTable[] = {
	{ 0x0101, createTEneAm01, 0, kEnemyRange },
	{ 0x0102, createTEneAm02, 0, kEnemyRange },
	{ 0x0104, createTEneAm07, 0, kEnemyRange },
	{ 0x0106, createTEneAntsQueen, 0, kEnemyRange },
};
static SetClassList sGreenClasses = { sGreenClassesTable, 4 };
static SetClass sBlueClassesTable[] = {
	{ 0x0101, createTEneAm01, 0, kEnemyRange },
	{ 0x0103, createTEneAm06, 0, kEnemyRange },
	{ 0x0107, createTEnePenguin, 0, kEnemyRange },
	{ 0x0109, createTEneFrog, 0, kEnemyRange },
};
static SetClassList sBlueClasses = { sBlueClassesTable, 4 };
static SetClass sRedClassesTable[] = {
	{ 0x0101, createTEneAm01, 0, kEnemyRange },
	{ 0x0105, createTEneAm08, 0, kEnemyRange },
	{ 0x0108, createTEneBat, 0, kEnemyRange },
	{ 0x010A, createTEneArmadillo, 0, kEnemyRange },
};
static SetClassList sRedClasses = { sRedClassesTable, 4 };
static SetClass sPurpleClassesTable[] = {
	{ 0x0101, createTEneAm01, 0, kEnemyRange },
	{ 0x0103, createTEneAm06, 0, kEnemyRange },
	{ 0x0107, createTEnePenguin, 0, kEnemyRange },
	{ 0x0102, createTEneAm02, 0, kEnemyRange },
};
static SetClassList sPurpleClasses = { sPurpleClassesTable, 4 };
static SetClass sOrangeClassesTable[] = {
	{ 0x0101, createTEneAm01, 0, kEnemyRange },
	{ 0x0102, createTEneAm02, 0, kEnemyRange },
	{ 0x0106, createTEneAntsQueen, 0, kEnemyRange },
	{ 0x0107, createTEnePenguin, 0, kEnemyRange },
};
static SetClassList sOrangeClasses = { sOrangeClassesTable, 4 };
static SetClass sYellowClassesTable[] = {
	{ 0x0101, createTEneAm01, 0, kEnemyRange },
	{ 0x0104, createTEneAm07, 0, kEnemyRange },
	{ 0x0105, createTEneAm08, 0, kEnemyRange },
	{ 0x010A, createTEneArmadillo, 0, kEnemyRange },
};
static SetClassList sYellowClasses = { sYellowClassesTable, 4 };
static SetClass sLastClassesTable[] = {
	{ 0x0101, createTEneAm01, 0, kEnemyRange },
	{ 0x0102, createTEneAm02, 0, kEnemyRange },
	{ 0x0105, createTEneAm08, 0, kEnemyRange },
	{ 0x0108, createTEneBat, 0, kEnemyRange },
};
static SetClassList sLastClasses = { sLastClassesTable, 4 };
static SetClass sGreenBossClassesTable[] = {
	{ 0x0900, createTEneGreenBoss, 0, kEnemyRange },
};
static SetClassList sGreenBossClasses = { sGreenBossClassesTable, 1 };
static SetClass sBlueBossClassesTable[] = {
	{ 0x0200, createTEneBoss02, 0, kEnemyRange },
};
static SetClassList sBlueBossClasses = { sBlueBossClassesTable, 1 };
static SetClass sRedBossClassesTable[] = {
	{ 0x0300, createTEneBoss03Mole, 0, kEnemyRange },
};
static SetClassList sRedBossClasses = { sRedBossClassesTable, 1 };
static SetClass sPurpleBossClassesTable[] = {
	{ 0x0400, createTEnePurpleBoss, 0, kEnemyRange },
};
static SetClassList sPurpleBossClasses = { sPurpleBossClassesTable, 1 };
static SetClass sOrangeBossClassesTable[] = {
	{ 0x0500, createTEneOrangeBoss, 0, kEnemyRange },
};
static SetClassList sOrangeBossClasses = { sOrangeBossClassesTable, 1 };
static SetClass sYellowBossClassesTable[] = {
	{ 0x0600, createTEneYellowBoss, 0, kEnemyRange },
};
static SetClassList sYellowBossClasses = { sYellowBossClassesTable, 1 };
static SetClass sLastBossClassesTable[] = {
	{ 0x0700, createTEneLastBoss, 0, kEnemyRange },
};
static SetClassList sLastBossClasses = { sLastBossClassesTable, 1 };
static SetClass sLastBoss2ClassesTable[] = {
	{ 0x0800, createTEneLastExBoss, 0, kEnemyRange },
};
static SetClassList sLastBoss2Classes = { sLastBoss2ClassesTable, 1 };

struct EnemyTheme {
	const char* mName;
	const SetInitList* mInits;
	const SetClassList* mClasses;
};

static EnemyTheme sCommonTheme[] = {
	{ "common", &sCommonInits, &sCommonClasses },
	{ 0, 0, 0 },
};

static EnemyTheme sEnemyThemes[] = {
	{ "green", &sGreenInits, &sGreenClasses },
	{ "blue", &sBlueInits, &sBlueClasses },
	{ "red", &sRedInits, &sRedClasses },
	{ "purple", &sPurpleInits, &sPurpleClasses },
	{ "orange", &sOrangeInits, &sOrangeClasses },
	{ "yellow", &sYellowInits, &sYellowClasses },
	{ "last", &sLastInits, &sLastClasses },
	{ "greenboss", &sGreenBossInits, &sGreenBossClasses },
	{ "blueboss", &sBlueBossInits, &sBlueBossClasses },
	{ "redboss", &sRedBossInits, &sRedBossClasses },
	{ "purpleboss", &sPurpleBossInits, &sPurpleBossClasses },
	{ "orangeboss", &sOrangeBossInits, &sOrangeBossClasses },
	{ "yellowboss", &sYellowBossInits, &sYellowBossClasses },
	{ "lastboss", &sLastBossInits, &sLastBossClasses },
	{ "lastboss2", &sLastBoss2Inits, &sLastBoss2Classes },
	{ 0, 0, 0 },
};

// 0x8001CE24
extern "C" void loadEnemyTheme(const char* theme)
{
	sEnemyCommonInits = sCommonTheme[0].mInits;
	sEnemyCommonClasses = sCommonTheme[0].mClasses;
	if (sEnemyCommonInits != 0) {
		runInits((const ObjInitPair*)sEnemyCommonInits->mPairs, sEnemyCommonInits->mCount);
	}

	sEnemyThemeInits = 0;
	sEnemyThemeClasses = 0;
	if (theme != 0) {
		for (const EnemyTheme* entry = sEnemyThemes; entry->mName != 0; entry++) {
			if (strncmp(theme, entry->mName, 0x80) == 0) {
				sEnemyThemeInits = entry->mInits;
				sEnemyThemeClasses = entry->mClasses;
				runInits((const ObjInitPair*)sEnemyThemeInits->mPairs, sEnemyThemeInits->mCount);
				return;
			}
		}
	}
}

// 0x8001CEE0
extern "C" void freeEnemyInits(const SetInitList** list)
{
	const SetInitList* inits = *list;
	if (inits != 0) {
		runTerms((const ObjInitPair*)inits->mPairs, inits->mCount);
		*list = 0;
	}
}

// 0x8001CF28
extern "C" void freeEnemyThemes()
{
	freeEnemyInits(&sEnemyThemeInits);
	freeEnemyInits(&sEnemyCommonInits);
}

// 0x8001CF54
// On a hit *outIndex grows by the position
extern "C" const SetClass* findEnemyClassIn(const SetClassList* list, int id, int* outIndex)
{
	const SetClass* classes;
	int count;
	if (list != 0) {
		count   = list->mCount;
		classes = list->mClasses;
		for (int i = 0; i < count; i++) {
			if (id == classes[i].mId) {
				*outIndex += i;
				return &classes[i];
			}
		}
	} else {
		count = 0;
	}
	*outIndex = count;
	return 0;
}

// 0x8001CFB8
// Common classes first then the theme that loadEnemyTheme selected
extern "C" const SetClass* findEnemyClass(int id, int* outIndex)
{
	*outIndex             = 0;
	const SetClass* found = findEnemyClassIn(sEnemyCommonClasses, id, outIndex);
	if (found == 0) {
		found = findEnemyClassIn(sEnemyThemeClasses, id, outIndex);
	}
	return found;
}

// 0x8001D014
extern "C" const SetClass* lookupEnemyClass(int id)
{
	int index;
	return findEnemyClass(id, &index);
}

