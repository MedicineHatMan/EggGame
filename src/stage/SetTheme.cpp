#include "stage/SetManager.h"
#include "io/ResFile.h"
#include "io/Resource.h"
#include "core/dolphin.h"
#include "object/ObjAction.h"
#include "enemy/EnemyGroup.h"
#include "enemy/SpawnGuard.h"
#include "camera/TCamera.h"
#include "object/TObjFogCollision.h"
#include "stage/SetObj16.h"
#include "object/TObjSetLightCollision.h"
#include "object/blue/TObjBlueWaterfall.h"
#include "object/TObjWindColli.h"
#include "object/yellow/TObjYellowSandFloor.h"
#include "enemy/boss/LastBossRes.h"
#include "object/yellow/TObjYellowFogTrapCtrl.h"
#include "object/yellow/TObjYellowFogTrapStatue.h"
#include "object/TObjSceneLightCollision.h"
#include "object/TObjKey00.h"
#include "object/blue/TObjBlueKatana.h"
#include "object/TObjSkull.h"
#include "camera/TCamPath.h"
#include "object/TObjBoss02Fence.h"
#include "object/map/TObjMapDoor01.h"
#include "object/red/TObjRedFireTrap.h"
#include "object/TObjChickenBoss.h"
#include "object/TObjBowlEgg.h"
#include "object/TObjGoldenEgg.h"
#include "object/TObjPilingSwitch.h"
#include "object/red/TObjRedDinoHead.h"
#include "object/red/TObjRedEventMagma.h"
#include "object/TObjEggFeed.h"
#include "object/TObjEggCannon.h"
#include "ui/TAeObjStageTitle.h"
#include "object/TObjHiyoko.h"
#include "object/purple/TObjPurpleVFSnow.h"
#include "object/TObjSnowman.h"
#include "object/purple/TObjPurpleBlizzard.h"
#include "ui/TAeObjStageSelectE3.h"
#include "object/mission/TMsnObjDarkGate.h"
#include "object/mission/TMsnObjGoalPole.h"
#include "object/mission/TObjMissionBomb.h"
#include "object/mission/TObjMissionPrisoner.h"
#include "object/purple/TObjPurpleIceGearL.h"
#include "object/purple/TObjPurplePropeller.h"
#include "object/TObjMGLeader.h"
#include "object/orange/TObjOrangeBossFence.h"
#include "object/orange/TObjOrangeFireworksBall.h"
#include "object/TObjEggRing.h"
#include "object/yellow/TObjYellowSand.h"
#include "object/TObjFractalCloud.h"
#include "enemy/boss/TEneLastBossMagic.h"
#include "object/TObjEggSuit.h"
#include "object/mission/TMsnObjOrangeCannon.h"
#include "object/TObjHelios.h"
#include "object/last/TObjLastAnimus.h"
#include "object/green/GreenObjBank.h"
#include "object/last/TObjLastSealedCrest.h"
#include "ui/TitleObjArc.h"
#include "object/TObjTitleBush.h"
#include "object/TObjTitleFlower.h"
#include "object/TObjTitleFlowers.h"
#include "object/TObjTitleTree.h"
#include "object/TObjTitlePAEffect.h"
#include "object/TObjTitleRainbow.h"
#include "object/TObjClosedEgg.h"
#include "egg/EggCreature.h"
#include "enemy/boss/EneBoss.h"
#include "object/blue/TObjBlueFlag.h"
#include "object/purple/TObjPurpleIceFenceD.h"
#include "object/purple/TObjPurpleIceWall.h"
#include "object/purple/TObjPurpleIceFloor.h"
#include "object/purple/TObjPurpleGear.h"
#include "object/yellow/TObjYellowTimberBrick.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "stage/SetSlot.h"
#include "object/TObjFirePole01.h"
#include "object/TObjBoss02Katana.h"
#include "object/red/TObjRedMagmaScene.h"
#include "object/TObjGameBowling.h"
#include "object/red/TObjRedMagmaShot.h"
#include "object/blue/TObjBlueCaptain.h"
#include "object/red/TObjRedDHeadInvokeArea.h"
#include "object/red/TObjRedFenceCtrl.h"
#include "object/blue/TObjBlueRoundShip.h"
#include "object/mission/TMsnObjCage.h"
#include "object/purple/TObjPurpleIceGearS.h"
#include "object/orange/TObjOrangePanelDeOpen.h"
#include "object/orange/TObjOrangeBattery.h"
#include "object/purple/TObjPurpleIceTrap.h"
#include "object/orange/TObjOrangeMirrorTrap.h"
#include "object/orange/TObjOrangeClockTower.h"
#include "egg/ObjEgg.h"
#include "object/TObjTitleChiken.h"
#include "object/TObjTitleLake.h"
#include "object/TObjTitleBigFall.h"
#include "object/TObjParticleGenerator.h"
#include "object/map/TObjMapCollision.h"
#include "object/TObjRestartColli.h"
#include "object/TObjRestartPoint.h"
#include "object/TObjHook.h"
#include "object/map/TObjMapRock01.h"
#include "object/mission/TMsnObjEmblem.h"
#include "object/mission/TMsnObjRing.h"
#include "object/mission/TMsnObjChicken.h"
#include "object/TObjDivineVoice.h"
#include "object/mission/TMsnObjCoin.h"
#include "object/TObjBrkScaffold.h"
#include "object/TObjStageObj.h"
#include "object/TObjHangBridge.h"
#include "object/green/TObjGreenPondScene.h"
#include "object/green/TObjGreenWaterfall.h"
#include "object/green/TObjGreenWindmill.h"
#include "object/map/TObjMapFoothold01.h"
#include "object/green/TObjGreenRay.h"
#include "object/TObjCannon01.h"
#include "object/TObjAnchor01.h"
#include "object/TObjShark01.h"
#include "object/TObjArrowPoint.h"
#include "object/TObjMirror.h"
#include "object/yellow/TObjYellowSlideDoor.h"
#include "object/yellow/TObjYellowFireTrap.h"
#include "object/yellow/TObjYellowStatue.h"
#include "object/yellow/TObjYellowPillar.h"
#include "object/yellow/TObjYellowSandfall.h"
#include "object/yellow/TObjYellowGateSet.h"
#include "object/last/TObjLastEggLight.h"
#include "object/TObjChickenBomb.h"
#include "object/last/TObjLastCrowLemuresCtrl.h"
#include "object/last/TObjLastRippleField.h"
#include "object/last/TObjLastResonanceSwitch.h"
#include "object/green/TObjGreenBossFence.h"
#include "object/green/TObjGreenBossLake.h"
#include "object/green/TObjGreenBossWaterfall.h"
#include "object/green/TObjGreenBossRay.h"
#include "object/last/TObjLastBossEgg.h"
#include "core/TMainTask.h"
#include "player/TPlyItem.h"
struct TObjSceneLightCollision;
struct TObjLastSealedCrest;
struct TObjLastAnimus;
struct TMsnObjOrangeCannon;
struct TObjEggSuit;
struct TObjFractalCloud;
struct TObjMGLeader;
struct TObjMissionPrisoner;
struct TObjMissionBomb;
struct TMsnObjGoalPole;
struct TMsnObjDarkGate;
struct TObjSnowman;
struct TObjHiyoko;
struct TObjWindColli;
struct TObjEggCannon;
struct TObjSetLightCollision;
struct TObjBowlEgg;
struct TObjChickenBoss;
struct TObjMapDoor01;
struct TObjFogCollision;
struct TObjKey00;
struct TObjTitleBigFall;
struct TObjTitleLake;
struct TObjTitleChiken;
struct TObjYellowTimberBrick;
struct TObjOrangeClockTower;
struct TObjOrangeMirrorTrap;
struct TObjPurpleIceTrap;
struct TObjOrangeBattery;
struct TObjOrangePanelDeOpen;
struct TObjPurpleIceGearS;
struct TObjPurpleGear;
struct TObjPurpleIceFloor;
struct TObjPurpleIceWall;
struct TObjPurpleIceFenceD;
struct TMsnObjCage;
struct TObjBlueFlagScene;
struct TObjBlueFlag;
struct TObjBlueRoundShip;
struct TObjRedFenceCtrl;
struct TObjRedDHeadInvokeArea;
struct TObjBlueCaptain;
struct TObjRedMagmaShot;
struct TObjGameBowling;
struct TObjRedMagmaObj;
struct TObjRedMagmaScene;
struct TObjBoss02Katana;
struct TObjFirePole01;
struct TCameraCtrl;

// The stage themes for each the setup functions TStage::load runs and the classes a SetDesc can create
// The sSet* pointers hold the loaded theme
const SetInitList* sSetThemeInits;
const SetClassList* sSetThemeClasses;
const SetInitList* sSetCommonInits;
const SetClassList* sSetCommonClasses;

// clang-format off

static SetInitPair sInits_80253220Pairs[] = {
	{ initTObjKey00, termTObjKey00 },
	{ initDoors, termDoors },
	{ initTObjEggFeed, termTObjEggFeed },
	{ doorUnused1, doorUnused2 },
	{ eggRingInit, eggRingExit },
	{ bowlEggInit, bowlEggTerm },
	{ initGoldenEgg, termGoldenEgg },
	{ initChickenBoss, termChickenBoss },
	{ pilingSwitchSetInit, pilingSwitchSetExit },
	{ initTObjEggCannon, termTObjEggCannon },
	{ initMissionBomb, termMissionBomb },
	{ initMissionPrisoner, termMissionPrisoner },
	{ initCage, termCage },
	{ resetDarkGateAssets, releaseDarkGateAssets },
	{ initGoalArc, releaseGoalArc },
	{ loadEmblemArc, releaseEmblemArc },
	{ initRingArc, releaseRingArc },
	{ clearChickenArc, termChickenArc },
	{ initMGLeader, releaseMGLeader },
	{ resetOrangeCannonAssets, releaseOrangeCannonAssets },
	{ initTObjHelios, termTObjHelios },
	{ lightAreaInit, lightAreaExit },
	{ initClosedEgg, termClosedEgg },
	{ initTObjFractalCloud, termTObjFractalCloud },
};
static SetInitList sInits_80253220 = { sInits_80253220Pairs, 24 };
static SetInitPair sInits_802532E0Pairs[] = {
	{ initTObjHiyoko, termTObjHiyoko },
	{ initTObjEggSuit, termTObjEggSuit },
};
static SetInitList sInits_802532E0 = { sInits_802532E0Pairs, 2 };
static SetInitPair sInits_802532F0Pairs[] = {
	{ initBlueKatana, termBlueKatana },
	{ initSkulls, termSkulls },
	{ initBlueWaterfallSet, termBlueWaterfallSet },
	{ initStageTitleSet, termStageTitleSet },
	{ initBlueFlagSet, termBlueFlagSet },
};
static SetInitList sInits_802532F0 = { sInits_802532F0Pairs, 5 };
static SetInitPair sInits_80253318Pairs[] = {
	{ loadRedMagmaStage, releaseRedMagma },
	{ lightCollisionInit, lightCollisionExit },
	{ fireTrapSetInit, fireTrapSetExit },
	{ dinoHeadSetInit, dinoHeadSetExit },
	{ initRedDinoHeadSet, termRedDinoHeadSet },
	{ loadMagmaArc, releaseMagmaArc },
};
static SetInitList sInits_80253318 = { sInits_80253318Pairs, 6 };
static SetInitPair sInits_80253348Pairs[] = {
	{ initSnowman, termSnowman },
	{ initSnowField, termSnowField },
	{ initStageSelectE3Set, termStageSelectE3Set },
	{ initPurpleIceFenceDSet, termPurpleIceFenceDSet },
	{ initPurpleIceWallSet, termPurpleIceWallSet },
	{ initPurpleIceFloorSet, termPurpleIceFloorSet },
	{ bombUnused1, bombUnused2 },
	{ iceGearSetInit, iceGearSetExit },
	{ initPurpleIceGearLSet, termPurpleIceGearLSet },
	{ blizzardInit, blizzardExit },
};
static SetInitList sInits_80253348 = { sInits_80253348Pairs, 10 };
static SetInitPair sInits_802C39B8Pairs[] = {
	{ loadFireworksBallArc, releaseFireworksBallArc },
};
static SetInitList sInits_802C39B8 = { sInits_802C39B8Pairs, 1 };
static SetInitPair sInits_80253398Pairs[] = {
	{ initYellowSand, termYellowSand },
	{ timberBrickSetInit, timberBrickSetExit },
};
static SetInitList sInits_80253398 = { sInits_80253398Pairs, 2 };
static SetInitPair sInits_802533A8Pairs[] = {
	{ initLastEggLightSet, termLastEggLightSet },
	{ initTObjLastSealedCrest, shutdownTObjLastSealedCrest },
};
static SetInitList sInits_802533A8 = { sInits_802533A8Pairs, 2 };
static SetInitPair sInits_802533B8Pairs[] = {
	{ greenObjLoad, greenObjFree },
	{ greenObjInit, greenObjExit },
	{ initObjIds, shutdownObjIds },
	{ initObjIds, shutdownObjIds },
	{ initObjIds, shutdownObjIds },
};
static SetInitList sInits_802533B8 = { sInits_802533B8Pairs, 5 };
static SetInitPair sInits_802533E0Pairs[] = {
	{ initBlueKatana, termBlueKatana },
	{ initBlueBoss, termBlueBoss },
};
static SetInitList sInits_802533E0 = { sInits_802533E0Pairs, 2 };
static SetInitPair sInits_802C39E8Pairs[] = {
	{ loadRedMagmaBoss, releaseRedMagmaBoss },
};
static SetInitList sInits_802C39E8 = { sInits_802C39E8Pairs, 1 };
static SetInitPair sInits_802C39F8Pairs[] = {
	{ loadOrangeBossObj, releaseOrangeBossObj },
};
static SetInitList sInits_802C39F8 = { sInits_802C39F8Pairs, 1 };
static SetInitPair sInits_802C3A08Pairs[] = {
	{ lastBossLoad, lastBossFree },
};
static SetInitList sInits_802C3A08 = { sInits_802C3A08Pairs, 1 };
static SetInitPair sInits_802C3A18Pairs[] = {
	{ lastExBossLoad, lastExBossFree },
};
static SetInitList sInits_802C3A18 = { sInits_802C3A18Pairs, 1 };
static SetInitPair sInits_802533F0Pairs[] = {
	{ initTitleObjArc, shutdownTitleObjArc },
	{ initTObjTitleBush, termTObjTitleBush },
	{ initTObjTitleFlower, termTObjTitleFlower },
	{ initTObjTitleFlowers, termTObjTitleFlowers },
	{ initTObjTitleTree, termTObjTitleTree },
	{ initTitleObjNone, shutdownTitleObjNone },
	{ initTObjTitlePAEffect, termTObjTitlePAEffect },
	{ initObjIds, shutdownObjIds },
	{ initObjIds, shutdownObjIds },
	{ initTObjTitleRainbow, termTObjTitleRainbow },
};
static SetInitList sInits_802533F0 = { sInits_802533F0Pairs, 10 };
// The set hooks these two classes give with C++ linkage

static SetClass sClasses_80253440Table[] = {
	{ 0x3, createTObjParticleGenerator, 0, 202500.0f },
	{ 0x1F, createTObjParticleGenerator1F, 0, 10000000000.0f },
	{ 0x1, createTObjFogCollision, prepareTObjFogCollision, 1000000.0f },
	{ 0x2C, createTObjFogCollision2C, 0, 10000000000.0f },
	{ 0x2, createTObjSetLightCollision, prepareTObjSetLightCollision, 90000.0f },
	{ 0x30, createTObjSceneLightCollision, prepareTObjSceneLightCollision, 90000.0f },
	{ 0x19, createEgg, 0, 90000.0f },
	{ 0x16, createSetObj16, 0, 1000000.0f },
	{ 0x2D, createTObjEggRing, 0, 1000000.0f },
	{ 0x4, createTObjKey00, 0, gKey00SetRange * gKey00SetRange },
	{ 0x1A, createTObjPilingSwitch, 0, gPilingSwitchSetRange * gPilingSwitchSetRange },
	{ 0xD, createTObjFirePole01, 0, 250000.0f },
	{ 0x10, createTObjMapCollision, 0, 90000.0f },
	{ 0xB, createTObjEggFeed, 0, gEggFeedSetRange * gEggFeedSetRange },
	{ 0xE, createTObjRestartColli, 0, 90000.0f },
	{ 0xF, createTObjRestartPoint, 0, 90000.0f },
	{ 0x17, createTObjBowlEgg, 0, 90000.0f },
	{ 0x18, createTObjGameBowling, 0, 640000.0f },
	{ 0x12, createTObjHiyoko, 0, gHiyokoSetRange * gHiyokoSetRange },
	{ 0x11, createTObjHook, 0, 640000.0f },
	{ 0x1B, createTObjMapRock01, 0, 640000.0f },
	{ 0x1C, createTObjGoldenEgg, 0, 10000000300000000.0f },
	{ 0x1D, createTObjEggCannon, 0, gEggCannonSetRange * gEggCannonSetRange },
	{ 0x20, createTObjWindColli, prepareTObjWindColli, 90000.0f },
	{ 0x21, createTMsnObjCage, 0, 10000000300000000.0f },
	{ 0x23, createTMsnObjDarkGate, 0, 10000000300000000.0f },
	{ 0x24, createTMsnObjGoalPole, 0, 10000000300000000.0f },
	{ 0x25, createTMsnObjEmblem, 0, 10000000300000000.0f },
	{ 0x26, createTMsnObjRing, 0, 10000000300000000.0f },
	{ 0x27, createTObjMissionBomb, 0, 10000000300000000.0f },
	{ 0x28, createTObjMissionPrisoner, 0, 10000000300000000.0f },
	{ 0x29, createTMsnObjChicken, 0, 10000000300000000.0f },
	{ 0x2A, createTObjMGLeader, 0, 640000.0f },
	{ 0x2B, createTObjDivineVoice, 0, 640000.0f },
	{ 0x2F, createTMsnObjCoin, 0, gCoinSetRange * gCoinSetRange },
	{ 0x15, createTObjChickenBoss, 0, gChickenBossSetRange * gChickenBossSetRange },
	{ 0x31, createTObjBrkScaffold, 0, gBrkScaffoldSetRange * gBrkScaffoldSetRange },
	{ 0x32, createTObjClosedEgg, 0, 90000.0f },
};
static SetClassList sClasses_80253440 = { sClasses_80253440Table, 38 };
static SetClass sClasses_802536A0Table[] = {
	{ 0xA, createTObjStageObj, 0, gStageObjSetRange * gStageObjSetRange },
	{ 0x1E, createTObjHangBridge, 0, 640000.0f },
	{ 0x5, createDoorA, 0, gDoorASetRange * gDoorASetRange },
	{ 0x2E, createTObjEggSuit, 0, 8099999740.0f },
	{ 0x1000, createTObjGreenPondScene, 0, gGreenPondSceneSetRange * gGreenPondSceneSetRange },
	{ 0x1002, createTObjGreenWaterfall, 0, gGreenWaterfallSetRange * gGreenWaterfallSetRange },
	{ 0x1001, createTObjGreenWindmill, 0, gGreenWindmillSetRange * gGreenWindmillSetRange },
	{ 0xC, createTObjMapFoothold01, 0, gMapFoothold01SetRange * gMapFoothold01SetRange },
	{ 0x1003, createTObjGreenRay, 0, gGreenRaySetRange * gGreenRaySetRange },
};
static SetClassList sClasses_802536A0 = { sClasses_802536A0Table, 9 };
static SetClass sClasses_80253730Table[] = {
	{ 0x100, createTObjCannon01, 0, gCannon01SetRange * gCannon01SetRange },
	{ 0x101, createTObjAnchor01, 0, gAnchor01SetRange * gAnchor01SetRange },
	{ 0x102, createTObjShark01, 0, 1000000.0f },
	{ 0x103, createTObjBlueKatana, 0, 90000.0f },
	{ 0x104, createTObjBlueWaterfall, 0, 1000000.0f },
	{ 0x105, createTObjArrowPoint, 0, 250000.0f },
	{ 0x106, createTObjSkull, 0, 1000000.0f },
	{ 0x5, createDoorB, 0, gDoorASetRange * gDoorASetRange },
	{ 0x107, createTObjBlueCaptain, 0, 1000000.0f },
	{ 0x108, createTObjBlueRoundShip, 0, 25000000.0f },
	{ 0x109, createTObjBlueFlag, 0, 1000000.0f },
	{ 0x10A, createTObjBlueFlagScene, 0, 1000000.0f },
	{ 0xA, createTObjStageObj, 0, gStageObjSetRange * gStageObjSetRange },
	{ 0xC, createTObjMapFoothold01, 0, gMapFoothold01SetRange * gMapFoothold01SetRange },
	{ 0x1E, createTObjHangBridge, 0, 640000.0f },
};
static SetClassList sClasses_80253730 = { sClasses_80253730Table, 15 };
static SetClass sClasses_80253820Table[] = {
	{ 0x11, createTObjHook, 0, 640000.0f },
	{ 0x300, createTObjRedMagmaScene, 0, 999999996000.0f },
	{ 0x301, createTObjRedMagmaObj, 0, 810000.0f },
	{ 0x5, createDoorC, 0, gDoorASetRange * gDoorASetRange },
	{ 0x7004, createTObjMirror, 0, 640000.0f },
	{ 0x304, createTObjRedMagmaShot, 0, 1000000.0f },
	{ 0x306, createTObjRedFireTrap, 0, 640000.0f },
	{ 0x307, createTObjRedDinoHead, 0, 4000000.0f },
	{ 0x308, createTObjRedDHeadInvokeArea, 0, 640000.0f },
	{ 0x309, createTObjRedEventMagma, 0, 999999996000.0f },
	{ 0x30A, createTObjRedFenceCtrl, 0, 250000.0f },
	{ 0xA, createTObjStageObj, 0, gStageObjSetRange * gStageObjSetRange },
	{ 0xC, createTObjMapFoothold01, 0, gMapFoothold01SetRange * gMapFoothold01SetRange },
	{ 0x1E, createTObjHangBridge, 0, 640000.0f },
};
static SetClassList sClasses_80253820 = { sClasses_80253820Table, 14 };
static SetClass sClasses_80253900Table[] = {
	{ 0x401, createTObjSnowman, 0, 250000.0f },
	{ 0x402, createTObjSnowmanHead, 0, 250000.0f },
	{ 0x403, createTObjPurpleVFSnow, 0, 4000000.0f },
	{ 0x404, createTObjPurpleIceFenceD, 0, 640000.0f },
	{ 0x405, createTObjPurpleIceWall, 0, gPurpleIceWallSetRange * gPurpleIceWallSetRange },
	{ 0x406, createTObjPurpleIceFloor, 0, gPurpleIceFloorSetRange * gPurpleIceFloorSetRange },
	{ 0x407, createTObjPurpleGear, 0, 640000.0f },
	{ 0x408, createTObjPurpleIceGearS, 0, 640000.0f },
	{ 0x409, createTObjPurpleIceGearL, 0, 10000000300000000.0f },
	{ 0x40A, createTObjPurplePropeller, 0, 1000000.0f },
	{ 0xA, createTObjStageObj, 0, gStageObjSetRange * gStageObjSetRange },
	{ 0x5, createDoorD, 0, gDoorASetRange * gDoorASetRange },
	{ 0x40B, createTObjPurpleIceTrap, 0, 640000.0f },
	{ 0xC, createTObjMapFoothold01, 0, gMapFoothold01SetRange * gMapFoothold01SetRange },
};
static SetClassList sClasses_80253900 = { sClasses_80253900Table, 14 };
static SetClass sClasses_802539E0Table[] = {
	{ 0xA, createTObjStageObj, 0, gStageObjSetRange * gStageObjSetRange },
	{ 0x5, createDoorE, 0, gDoorASetRange * gDoorASetRange },
	{ 0x500, createTObjOrangePanelDeOpen, 0, gOrangePanelDeOpenSetRange * gOrangePanelDeOpenSetRange },
	{ 0x501, createTObjOrangeBattery, 0, 640000.0f },
	{ 0x502, createTObjOrangeMirrorTrap, 0, 1000000.0f },
	{ 0x503, createTObjOrangeClockTower, 0, 10000000300000000.0f },
	{ 0x504, createTObjOrangeFireworksBall, 0, 10000000300000000.0f },
	{ 0x505, createDoor02, 0, gDoorASetRange * gDoorASetRange },
	{ 0x506, createTObjFractalCloud, 0, 10000000300000000.0f },
	{ 0x507, createTMsnObjOrangeCannon, 0, gOrangeCannonSetRange * gOrangeCannonSetRange },
};
static SetClassList sClasses_802539E0 = { sClasses_802539E0Table, 10 };
static SetClass sClasses_80253A80Table[] = {
	{ 0xA, createTObjStageObj, 0, gStageObjSetRange * gStageObjSetRange },
	{ 0x700, createTObjYellowSlideDoor, 0, gYellowSlideDoorSetRange * gYellowSlideDoorSetRange },
	{ 0x701, createTObjYellowFireTrap, 0, 640000.0f },
	{ 0x702, createTObjYellowSandGen, 0, 999999996000.0f },
	{ 0x703, createTObjYellowStatue, 0, 16000000.0f },
	{ 0x704, createTObjYellowSandMdl, 0, 999999996000.0f },
	{ 0x705, createTObjYellowPillar, 0, gYellowPillarSetRange * gYellowPillarSetRange },
	{ 0x706, createTObjYellowTimberBrick, 0, gYellowTimberBrickSetRange * gYellowTimberBrickSetRange },
	{ 0x70A, createTObjYellowSandFloor, 0, gYellowSandFloorSetRange * gYellowSandFloorSetRange },
	{ 0x707, createTObjYellowSandfall, 0, gYellowSandfallSetRange * gYellowSandfallSetRange },
	{ 0x708, createTObjYellowFogTrapCtrl, 0, gYellowFogTrapCtrlSetRange * gYellowFogTrapCtrlSetRange },
	{ 0x709, createTObjYellowFogTrapStatue, 0, gYellowFogTrapStatueSetRange * gYellowFogTrapStatueSetRange },
	{ 0x5, createDoorF, 0, gDoorASetRange * gDoorASetRange },
	{ 0x70B, createTObjYellowGateSet, 0, gYellowGateSetSetRange * gYellowGateSetSetRange },
};
static SetClassList sClasses_80253A80 = { sClasses_80253A80Table, 14 };
static SetClass sClasses_80253B60Table[] = {
	{ 0x800, createTObjLastEggLight, 0, 999999996000.0f },
	{ 0xA, createTObjStageObj, 0, gStageObjSetRange * gStageObjSetRange },
	{ 0x5, createDoorG, 0, gDoorASetRange * gDoorASetRange },
	{ 0x801, createTObjLastTorch, 0, gLastTorchSetRange * gLastTorchSetRange },
	{ 0x802, createTObjLastAnimus, 0, gLastAnimusSetRange * gLastAnimusSetRange },
	{ 0x803, createTObjLastCrowLemuresCtrl, 0, gLastCrowLemuresCtrlSetRange * gLastCrowLemuresCtrlSetRange },
	{ 0x804, createTObjLastSealedCrest, 0, gLastSealedCrestSetRange * gLastSealedCrestSetRange },
	{ 0x805, createTObjLastRippleField, 0, gLastRippleFieldSetRange * gLastRippleFieldSetRange },
	{ 0x902, createTObjLastResonanceSwitch, 0, gPilingSwitchSetRange * gPilingSwitchSetRange },
};
static SetClassList sClasses_80253B60 = { sClasses_80253B60Table, 9 };
static SetClass sClasses_80253BF0Table[] = {
	{ 0x1100, createTObjGreenBossFence, 0, 4000000.0f },
	{ 0x1102, createTObjGreenBossLake, 0, 9000000.0f },
	{ 0x1101, createTObjGreenBossWaterfall, 0, 9000000.0f },
	{ 0x1103, createTObjGreenBossRay, 0, 9000000.0f },
};
static SetClassList sClasses_80253BF0 = { sClasses_80253BF0Table, 4 };
static SetClass sClasses_80253C30Table[] = {
	{ 0x103, createTObjBlueKatana, 0, 90000.0f },
	{ 0x200, createTObjBoss02Katana, 0, 4000000.0f },
	{ 0x201, createTObjBoss02Fence, 0, 4000000.0f },
};
static SetClassList sClasses_80253C30 = { sClasses_80253C30Table, 3 };
static SetClass sClasses_80253C60Table[] = {
	{ 0x300, createTObjRedMagmaScene, 0, 4000000.0f },
	{ 0x301, createTObjRedMagmaObj, 0, 3240000.0f },
};
static SetClassList sClasses_80253C60 = { sClasses_80253C60Table, 2 };
static SetClass sClasses_80253C80Table[] = {
	{ 0x600, createTObjOrangeBossFence, 0, 1000000.0f },
};
static SetClassList sClasses_80253C80 = { sClasses_80253C80Table, 1 };
static SetClass sClasses_80253C90Table[] = {
	{ 0x900, createTObjLastBossInd, 0, 999999996000.0f },
	{ 0x901, createTObjLastBossEgg, 0, 999999996000.0f },
};
static SetClassList sClasses_80253C90 = { sClasses_80253C90Table, 2 };
static SetClass sClasses_80253CB0Table[] = {
	{ 0x900, createTObjLastBossInd, 0, 999999996000.0f },
};
static SetClassList sClasses_80253CB0 = { sClasses_80253CB0Table, 1 };
static SetClass sClasses_80253CC0Table[] = {
	{ 0x1200, createTObjTitleBush, 0, 10000000000.0f },
	{ 0x1201, createTObjTitleFlower, 0, 10000000000.0f },
	{ 0x1202, createTObjTitleFlowers, 0, 10000000000.0f },
	{ 0x1203, createTObjTitleTree, 0, 10000000000.0f },
	{ 0x1204, createTObjTitleChiken, 0, 4000000.0f },
	{ 0x1206, createTObjTitlePAEffect, 0, 4000000.0f },
	{ 0x1207, createTObjTitleLake, 0, 100000000.0f },
	{ 0x1208, createTObjTitleBigFall, 0, 100000000.0f },
	{ 0x1209, createTObjTitleRainbow, 0, 10000000000.0f },
};
static SetClassList sClasses_80253CC0 = { sClasses_80253CC0Table, 9 };

struct SetTheme {
	const char* mName;
	const SetInitList* mInits;
	const SetClassList* mClasses;
};

static SetTheme sCommonTheme[2] = {
	{ "common", &sInits_80253220, &sClasses_80253440 },
	{ 0, 0, 0 },
};

static SetTheme sThemes[] = {
	{ "green", &sInits_802532E0, &sClasses_802536A0 },
	{ "blue", &sInits_802532F0, &sClasses_80253730 },
	{ "red", &sInits_80253318, &sClasses_80253820 },
	{ "purple", &sInits_80253348, &sClasses_80253900 },
	{ "orange", &sInits_802C39B8, &sClasses_802539E0 },
	{ "yellow", &sInits_80253398, &sClasses_80253A80 },
	{ "last", &sInits_802533A8, &sClasses_80253B60 },
	{ "greenboss", &sInits_802533B8, &sClasses_80253BF0 },
	{ "blueboss", &sInits_802533E0, &sClasses_80253C30 },
	{ "redboss", &sInits_802C39E8, &sClasses_80253C60 },
	{ "orangeboss", &sInits_802C39F8, &sClasses_80253C80 },
	{ "lastboss", &sInits_802C3A08, &sClasses_80253C90 },
	{ "lastboss2", &sInits_802C3A18, &sClasses_80253CB0 },
	{ "title", &sInits_802533F0, &sClasses_80253CC0 },
	{ 0, 0, 0 },
};
// clang-format on

extern "C" void loadSetTheme(const char* theme)
{
	sSetCommonInits   = sCommonTheme[0].mInits;
	sSetCommonClasses = sCommonTheme[0].mClasses;
	if (sSetCommonInits != 0) {
		runInits((const ObjInitPair*)sSetCommonInits->mPairs, sSetCommonInits->mCount);
	}

	sSetThemeInits   = 0;
	sSetThemeClasses = 0;
	if (theme != 0) {
		for (const SetTheme* entry = sThemes; entry->mName != 0; entry++) {
			if (strncmp(theme, entry->mName, 0x80) == 0) {
				sSetThemeInits   = entry->mInits;
				sSetThemeClasses = entry->mClasses;
				runInits((const ObjInitPair*)sSetThemeInits->mPairs, sSetThemeInits->mCount);
				return;
			}
		}
	}
}

extern "C" void freeSetInits(const SetInitList** list)
{
	const SetInitList* inits = *list;
	if (inits != 0) {
		runTerms((const ObjInitPair*)inits->mPairs, inits->mCount);
		*list = 0;
	}
}

extern "C" void freeSetThemes()
{
	freeSetInits(&sSetThemeInits);
	freeSetInits(&sSetCommonInits);
}

// 0x80018A9C
extern "C" const SetClass* findSetClassIn(const SetClassList* list, int id, int* outIndex)
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

extern "C" const SetClass* findSetClass(int id, int* outIndex)
{
	*outIndex             = 0;
	const SetClass* found = findSetClassIn(sSetCommonClasses, id, outIndex);
	if (found == 0) {
		found = findSetClassIn(sSetThemeClasses, id, outIndex);
	}
	return found;
}

const SetClass* findObjClass(int id)
{
	int index;
	return findSetClass(id, &index);
}
