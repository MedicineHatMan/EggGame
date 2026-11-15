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

// How stage objects get created
// 1 TStage::load gives the manager the three placement files and runs the setup function of every class in the common
// list and in the stage theme's list
// 2 TSetGenerator::draw calls scan() each frame which checks 30 records of each kind and wraps around at the end
// 3 A free slot makes SetDesc look up the record's class ID
// Not identified yet dtk's default fn_/lbl_ names
extern "C" {

// Wrappers around the single manager

extern const int sSetListSizes[2] = {
	1024, 512,
};
}

// The single manager
SetManager* sSetManagerObj;

inline SetLink::SetLink()
{
	mKind  = -1;
	mIndex = 0;
	mMode  = 0;
	mTimer = 0;
}

#include "stage/SetManagerBody.h"

SetList::SetList(int kind, int capacity)
{
	mKind     = kind;
	mSlots    = (SetSlot*)__construct_new_array(operator new[](capacity * sizeof(SetSlot) + 0x10), 0, 0, sizeof(SetSlot), capacity);
	mCount    = 0;
	mCapacity = capacity;
	mFile     = 0;
	for (int i = 0; i < mCapacity; i++) {
		mSlots[i].init();
	}
}

// 0x800169FC
SetList::~SetList()
{
	if (mFile != 0) {
		releaseResource(mFile);
		mFile = 0;
	}
	if (mSlots != 0) {
		if (mSlots != 0) {
			operator delete[]((u8*)mSlots - 0x10);
		}
		mSlots = 0;
	}
}

int SetList::isValid(int index)
{
	int valid = 0;
	if (index >= 0 && index < mCapacity) {
		valid = 1;
	}
	return (u8)valid;
}

SetSlot* SetList::getSlot(int index)
{
	if (isValid(index)) {
		return &mSlots[index];
	}
	return 0;
}

// 0x80016AF4
void SetList::attachLink(SetLink* link)
{
	s16 index = link->mIndex;
	if (isValid(index)) {
		((SetLink**)&mSlots[index])[5] = link;
	}
}
