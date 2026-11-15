#include "core/dolphin.h"
#include "stl/string.h"
#include "stage/Stage.h"
#include "io/LangFile.h"
#include "io/ResFile.h"
#include "io/Resource.h"
#include "camera/EventCamArc.h"
#include "stage/EventScript.h"
#include "item/TItemManage.h"
#include "engine/Landscape.h"
#include "player/Player.h"
#include "ui/TResult.h"
#include "engine/Rail.h"
#include "engine/PortalVis.h"
#include "ui/Font.h"
#include "engine/ShadowTex.h"
#include "engine/GroundQuery.h"
#include "object/TObjPlayer.h"
#include "stage/SetManager.h"
#include "enemy/Enemy.h"
#include "camera/TCameraCtrl.h"
#include "engine/ParticleBank.h"
#include "object/ObjCommon.h"
#include "core/TSceneManager.h"
#include "stage/TEventCtrl.h"
#include "sound/SoundCtrl.h"
#include "sound/SoundBank.h"
#include "ui/AeObjControl.h"
#include "core/Vibration.h"
#include "ui/TAeObjGo.h"
#include "animal/TAniRacePath.h"
#include "animal/AniModelManager.h"
#include "stage/TStageScore.h"
#include "ui/NowLoading.h"
#include "engine/Colli.h"
#include "object/ObjAction.h"
#include "engine/Viewport.h"
#include "camera/TCamEditor.h"
#include "io/NinjaFile.h"
#include "object/TObjKey00.h"
#include "stage/SetSpawner.h"
#include "engine/TFrameBlur.h"
#include "stage/TGameState.h"
#include "object/TObjLauncher.h"
#include "engine/FxSystem.h"
#include "object/ObjShadow.h"
#include "ui/BossConsole.h"
#include "object/TObjWindColli.h"
#include "ui/TFade.h"
#include "io/SaveData.h"
#include "engine/LightSet.h"
#include "egg/TEggTextureManager.h"
#include "ui/TBlindCtrl.h"
#include "stage/TStageBlue.h"
#include "stage/TStageRed.h"
#include "object/purple/TObjPurpleBlizzard.h"
#include "stage/TStageOrange.h"
#include "ui/TAeObjStageSelect.h"
#include "stage/TStageLast.h"
#include "stage/TStageGreenBoss.h"
#include "enemy/boss/TEneBoss02_PoolPow.h"
#include "stage/TStageRedBoss.h"
#include "stage/TStageOrangeBoss.h"
#include "stage/TStageYellowBoss.h"
#include "stage/TStageLastBoss.h"
#include "stage/TStageLastBoss2.h"
#include "core/Mem.h"
#include "core/TMainTask.h"
#include "camera/TCamera.h"
#include "object/TObject.h"
#include "engine/DrawTask.h"
#include "engine/FogBlend.h"
#include "engine/TFootPrinter.h"
#include "ui/TAeObjStageTitle.h"
#include "ui/TAeObjPause.h"
#include "enemy/boss/TEneYellowBossGreen.h"
#include "ui/MessageTable.h"
#include "core/TPeriVibrate.h"
#include "engine/TFogCtrl.h"
#include "stage/StageTime.h"
#include "engine/FrameTex.h"
#include "core/MathUtil.h"
#include "core/Pad.h"
#include "Ninja/ninja.h"
struct TPauseMenu;
struct TIntro;
struct TObject;
struct TChrPlayer;
struct TAeObjPause;
struct TAeObjStageTitle;
struct ObjMsg;
struct CamView;
struct TObjAction;
struct TMainTask;
struct TCharaColliCtrl;
struct TCustomGameContext;

// How a stage gets loaded in order
// 1 Something builds a TModeStage and sets mName TGameFlow::startStage GameFlow.cpp
// 2 TModeStage::loadFiles queues k_<common>.prd and k_<stage>.prd
// 3 TModeStage::init runs after the PRDs arrive
// Not identified yet dtk's default fn_/lbl_ names

extern "C" GXColor sLightClearColor; // 0x802CD2D0 .sbss2
extern "C" void stageStub() { }

#include "stage/StageClassFuncs.h"

// The stage modules each stage starts and stops in order runInits / runTerms
void* sStageClassFuncs[0x48] = {
	(void*)initDrawTasks,       (void*)termDrawTasks,
	(void*)camCtrlInit,         (void*)camCtrlTerm,
	(void*)viewportInit,        (void*)viewportTerm,
	(void*)initColli,           (void*)exitColli,
	(void*)frameTexInit,        (void*)frameTexExit,
	(void*)frameBlurInit,       (void*)frameBlurReset,
	(void*)genTexInit,          (void*)genTexExit,
	(void*)shadowTexCreate,     (void*)shadowTexDestroy,
	(void*)spritesInit,         (void*)spritesTerm,
	(void*)fogLoad,             (void*)fogUnload,
	(void*)lightZoneInit,       (void*)lightZoneExit,
	(void*)lightSetCreate,      (void*)lightSetDestroy,
	(void*)eventInit,           (void*)eventQuit,
	(void*)eventScriptInit,     (void*)eventScriptExit,
	(void*)eventCamArcInit,     (void*)eventCamArcExit,
	(void*)fxSystemInit,        (void*)fxSystemShutdown,
	(void*)initObjIds,          (void*)shutdownObjIds,
	(void*)loadEggAssets,       (void*)unloadEggAssets,
	(void*)gameContextInit,     (void*)gameContextExit,
	(void*)initSetManager,      (void*)freeSetManager,
	(void*)initSetGenerator,    (void*)exitSetGenerator,
	(void*)stageLightsInit,     (void*)stageLightsExit,
	(void*)drawStateInit,       (void*)drawStateExit,
	(void*)fogBlendInit,        (void*)fogBlendExit,
	(void*)debugEditExec,       (void*)debugEditDraw,
	(void*)groundInit,          (void*)groundShutdown,
	(void*)createDrawStateSlot, (void*)deleteDrawStateSlot,
	(void*)fireListCreate,      (void*)fireListFree,
	(void*)redFireTrapStubA,    (void*)redFireTrapStubB,
	(void*)railMgrCreate,       (void*)railMgrDestroy,
	(void*)shadowSystemInit,    (void*)shadowSystemExit,
	(void*)loadSpeakIconArc,    (void*)freeSpeakIconArc,
	(void*)ringDrawInit,        (void*)ringDrawTerm,
	(void*)loadStageSelectText, (void*)freeStageSelectText,
	(void*)scoreInit,           (void*)scoreExit,
	(void*)loadEggMessages,     (void*)freeEggMessages,
};

extern "C" {
TArc* sStageObjMotionArc;   // 0x802C4CC8
int sStageFirstLoad;        // 0x802C4CCC
TArc* sPlayerMotionArc;     // 0x802C4CD0
TArc* sRegularObjMotionArc; // 0x802C4CD4
TStage* sCurrentStage;      // 0x802C4CD8
int sStageLoadCount;        // 0x802C4CDC
}

TStage::TStage()
{
	u32 lightColor;
	u32 ambientColor;

	mCollision = 0;
	_10        = 0;

	_00 = new SceneRecord;
	setSceneRecord(_00);

	mVis     = new PortalVis;
	_00->mVis = mVis;

	mLights    = new SceneLights;
	lightColor = 0xFFFFFFFF;
	sceneLightsSetSunColor(mLights, &lightColor);
	ambientColor = 0x708090FF;
	sceneLightsSetAmbient(mLights, &ambientColor);

	_24 = new LightGroup;
	_24->set(&mLights->mSet);
	_00->mLights = _24;

	mPath        = 0;
	sStageObjMotionArc = 0;
	sStageFirstLoad = 1;
	setCurrentStage(this);
}

TStage::~TStage()
{
	delete _24;

	delete mLights;
	delete mVis;
	delete _00;
}

void TStage::load(const char* name)
{
	mName = name;

	gjSetAmbient(0.6f, 0.65f, 0.6f);
	gjSetLightSwitch(0, 1);
	gjSetLightVector(0, 0.5f, -1.0f, 0.0f);
	gjSetLightColor(0, 1.0f, 1.0f, 1.0f);
	gjLoadLight(0);
	gjSetFogColor(0x7FFF00);
	gjSetFogRange(100.0f, 6000.0f);
	gjSetFogCurve(5);

	stageStub();
	runInits((const ObjInitPair*)sStageClassFuncs, 0x24);

	stageStub();
	StageDef* def = findStageDef(name);

	stageStub();
	if (def->mRes != 0) {
		if (def->mRes->mSoundBank != 0) {
			soundLoadBank(4, def->mRes->mSoundBank);
		}
		if (def->mRes->mWorldBank != 0) {
			soundLoadBank(7, def->mRes->mWorldBank);
		}
	}

	stageStub();
	loadObjCommon(def->mRes->mStageObjArc, def->mRes->mGeometryArc);
	loadSetTheme(def->mTheme);
	loadEnemyTheme(def->mTheme);

	stageStub();
	if (def->mEvent != 0) {
		eventScriptLoad(def->mEvent);
	}
	eventCreate();
	eventScriptRequest(9999);

	stageStub();
	if (def->mCollision != 0) {
		loadOwnedResource(def->mCollision, 'CSNI', &mCollision);
		groundSetWorldData(mCollision->mData);
	}

	stageStub();
	initLandscape();
	if (def->mLandscape != 0) {
		loadLandscape(def->mLandscape);
	}

	stageStub();
	if (def->mPath != 0) {
		loadOwnedResource(def->mPath, 'PATH', &mPath);
		railMgrLoad((RailSet*)mPath->mData);
	}

	stageStub();
	camLoadAreas(def->mSetCamera);
	aniModelInit();
	loadGoAnimalPad__Fv();

	stageStub();
	aniRaceInit();

	stageStub();
	mMessages = 0;
	if (def->mMessages != 0) {
		loadSizedResource(getLangFileName(def->mMessages), &mMessages);
		setMessageTable((MessageFile*)mMessages->mData);
	} else {
		setMessageTable(0);
	}

	_18 = 0;
	if (def->mEventCamArc != 0) {
		eventCamArcLoad(def->mEventCamArc);
	}

	if (def->mRes != 0) {
		if (def->mRes->mParticles != 0) {
			loadParticles(def->mRes->mParticles);
		}
		if (def->mRes->mObjMotion != 0) {
			loadArc(def->mRes->mObjMotion, &sStageObjMotionArc);
			if (sStageObjMotionArc != 0) {
				fxBankBind(8, sStageObjMotionArc->mData);
			}
		}
	}

	stageStub();
	loadSetEnemyFile(def->mSetEnemy);
	loadSetObjFile(def->mSetObj);
	loadSetDesignFile(def->mSetDesign);

	stageStub();
	loadArc("pa_player.pad", &sPlayerMotionArc);
	if (sPlayerMotionArc != 0) {
		fxBankBind(0, sPlayerMotionArc->mData);
	}
	loadArc("pa_obj_regular.pad", &sRegularObjMotionArc);
	if (sRegularObjMotionArc != 0) {
		fxBankBind(7, sRegularObjMotionArc->mData);
	}
}

void TStage::unload()
{
	if (sPlayerMotionArc != 0) {
		fxBankUnbind(0, sPlayerMotionArc->mData);
		releaseResource(sPlayerMotionArc);
	}
	if (sRegularObjMotionArc != 0) {
		fxBankUnbind(7, sRegularObjMotionArc->mData);
		releaseResource(sRegularObjMotionArc);
	}

	releaseResource(mMessages);
	releaseResource(_18);

	if (sStageObjMotionArc != 0) {
		fxBankUnbind(8, sStageObjMotionArc->mData);
		releaseResource(sStageObjMotionArc);
	}

	freeParticles();
	releaseResource(mPath);
	freeLandscape();
	releaseResource(mCollision);
	aniRaceEnd();
	freeGoAnimalPad__Fv();
	aniModelExit();
	freeEnemyThemes();
	freeSetThemes();
	releaseObjCommon();
	runTerms((const ObjInitPair*)sStageClassFuncs, 0x24);
}

void TStage::start(int first)
{
	stageStub();
	periVibrateCreate();
	createCameraCtrl();
	fogCreate();
	blindCreate();
	lightSetBlend(fogBlendUpdate());
	createSetSpawner();
	createTCharaColliCtrl();
	colliClearDebug();
	fontCreateTask();
	debugEditInit();

	TSceneManager* unk = new (gObjHeap) TSceneManager(gTL_02);
	unk->mRecord = _00;

	footPrinterCreate();
	stageStub();
	frameBlurCreate();
	createTPolygonAnim__Fv();
	windCreate();
	createLandscapeObject();

	if (sStageFirstLoad != 0 || first != 0) {
		sStageFirstLoad = 0;
		eventScriptRequest(1000);
	}

	bgmPlay((u16)getStageBgm());
	scoreCreate();
	createTEggTextureManager();
	eventScriptRequest(2000);
}

extern "C" void stageLoadBossConsole(TStage* stage)
{
	bossConsoleLoad(stage->mName);
}

extern "C" void stageFreeBossConsole()
{
	bossConsoleFree();
}

extern "C" void stageCamLockAll()
{
	camLockAll();
}

void TStage::calc()
{
	TChrPlayer* player = (TChrPlayer*)findPlayer(0);
	if (player != 0) {
		setRangeCenter(player->getPos());
	}
	spriteClear();
	gMainTask.draw();
	groundUpdateMoving();
}

void TStage::drawScene()
{
	gMainTask.vfn_10();
}

void TStage::drawEffects(int withExtras)
{
	GXSetCullMode(GX_CULL_BACK);
	runDrawTasks(0x20);
	if (withExtras != 0) {
		runDrawTasks(0x2C);
	}
	runDrawTasks(0x21);
	shadowAlphaOff();
	if (withExtras != 0) {
		runDrawTasks(0x2D);
	}
	sortDrawTasks(0x22);
	runDrawTasks(0x22);
	shadowAlphaOn();
	runDrawTasks(0x23);
	shadowDraw();
	frameTexRequestCur();
	frameTexUpdateCur();
	shadowDarkenScreen();
	runDrawTasks(0x2A);
	runDrawTasks(0x24);
	GXSetCullMode(GX_CULL_NONE);
	runDrawTasks(0x32);
	spriteDrawAll();
	frameTexRequestCur();
	frameTexUpdateCur();
	GXSetCullMode(GX_CULL_NONE);
	runDrawTasks(0x3A);
	runDrawTasks(0x3B);
	runDrawTasks(0x3C);
	if (eventIsBusy() == 0) {
		runDrawTasks(0x3D);
	}
	runDrawTasks(0x3E);
	runDrawTasks(0x3F);
	GXSetCullMode(GX_CULL_NONE);
}

void TStage::run(u32 flags)
{
	if (flags & 1) {
		calc();
	}
	// Three separate tests in the DOL on a value that cannot change between them
	u32 draw = flags & 2;
	if (draw) {
		camSetDrawView(0);
		drawScene();
	}
	if (draw) {
		updateLight();
	}
	if (draw) {
		setViewport(0);
		camSetDrawView(0);
		drawObjects();
		drawEffects(1);
	}
	sStageLoadCount++;
}

void setCurrentStage(TStage* stage)
{
	sCurrentStage = stage;
}

TStage* getCurrentStage()
{
	return sCurrentStage;
}

void TStage::drawObjects()
{
	lightSetBlend(fogBlendUpdate());
	lightSetLoad();
}

// 0x800227D4
void getSpawnPos(Vec3f* out, const TStage* stage, int index)
{
	StageDef* def = findStageDef(stage->mName);
	out->set(def->mSpawns[index].mPos.x, def->mSpawns[index].mPos.y, def->mSpawns[index].mPos.z);
}

TStage* createDefaultStage()
{
	return new TStageDefault;
}

TStageDefault::TStageDefault() { }

void updateLight()
{
	TChrPlayer* player = (TChrPlayer*)findPlayer(0);
	if (player != 0) {
		Vec3f pos     = *player->getPos();
		GXColor color = sLightClearColor;
		GXSetCopyClear(color, 0xFFFFFF);
		shadowTexGet()->begin(&pos);
		runDrawTasks(0x1F);
		shadowTexGet()->end();
	}
}

TStageDefault::~TStageDefault() { }
