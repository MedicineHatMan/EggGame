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

struct Unk8000FAC8 {
	u8 _00[0x24];
	u8 _24; // _24
};
struct PadQuery {
	u16 mType;  // _00
	u8 _02[0xE];
};
struct StagePad {
	u8 _00[0x8];
	u32 mTrigger; // _08
};
inline int isFlagSet(int flag);
inline int isFlagSet(int flag)
{
	return keyBitIsSet(flag + 0x400);
}
struct CountUnlock {
	u16 mFlag;
	u16 mCount;
};
// Used when a stage has no mission or names one missing from the table
#define MISSION_TYPE_NONE 3

struct StageFactory {
	const char* mTheme;
	TStage* (*mCreate)();
};

static const StageFactory sStageFactories[] = {
	{ "blue", createBlueStage },       { "red", createRedStage },        { "purple", createPurpleStage },
	{ "orange", createOrangeStage },     { "yellow", createYellowStage },     { "last", createLastStage },
	{ "greenboss", createGreenBossStage },  { "blueboss", createBlueBossStage },   { "redboss", createRedBossStage },
	{ "purpleboss", createPurpleBossStage }, { "orangeboss", createOrangeBossStage }, { "yellowboss", createYellowBossStage },
	{ "lastboss", createLastBossStage },   { "lastboss2", createLastBoss2Stage },  { 0, 0 },
};

struct MissionType {
	const char* mName;
	int mType;
};

static const MissionType sMissionTypes[15] = {
	{ "boss", 0 },     { "collect", 1 },   { "dino", 2 },    { "goal", 4 },       { "gold", 5 },
	{ "mini", 6 },     { "rescue", 8 },    { "snowman", 9 }, { "battery", 11 },   { "gear", 10 },
	{ "wipeout", 13 }, { "elder", 14 },    { "lastboss", 15 }, { "race", 7 },     { "fireworks", 12 },
};

TModeStage* TModeStage::create(TModeAllocator* allocator)
{
	return new (allocator) TModeStage;
}

TModeStage::TModeStage()
{
	mState    = 8;
	mPlayerNo = 0;
	_0E       = 5;
	mName     = 0;
	mSkipDraw = 0;
	_24       = 1;

	mPauseMenu = 0;
	mIntro     = 0;
	mResult    = 0;
	mContext    = 0;

	mStartTime.mHour   = 0;
	mStartTime.mMinute = 0;
	mStartTime.mSecond = 0;
	mStartTime.mMilli  = 0;
	mSavedTime.mHour   = 0;
	mSavedTime.mMinute = 0;
	mSavedTime.mSecond = 0;
	mSavedTime.mMilli  = 0;

	_98 = 0;
	_9C = 0;
	_A0 = 0;

	mStartX     = 0.0f;
	mStartY     = 0.0f;
	mStartZ     = 0.0f;
	mStartAngle = 0;

	stageRecordClear(&mRecord);
	stageRecordClear(&mSavedRecord);
}

static TStage* createStageForTheme(const char* theme)
{
	for (int i = 0; sStageFactories[i].mTheme != 0; i++) {
		if (strcmp(theme, sStageFactories[i].mTheme) == 0) {
			return sStageFactories[i].mCreate();
		}
	}
	return createDefaultStage();
}

// 0x80022BAC
void TModeStage::loadFiles()
{
	char path[0x40];

	nowLoadingDisable();

	path[0]         = 'k';
	path[1]         = '_';
	char* dst       = path + 2;
	const char* src = mName;
	while (*src != '\0') {
		*dst++ = *src++;
	}
	dst[0] = '.';
	dst[1] = 'p';
	dst[2] = 'r';
	dst[3] = 'd';
	dst[4] = '\0';

	if (strstr(mName, "boss") != 0) {
		loadPrd("k_boss_common.prd");
		loadPrd(path);
	} else {
		loadPrd("k_stage_common.prd");
		// The first stage of the last world is just called "last" but its file is k_last1.prd
		if (strcmp(mName, "last") == 0) {
			loadPrd("k_last1.prd");
		} else {
			loadPrd(path);
		}
	}
}

static const CountUnlock sCountUnlocks[9] = {
	{ 0x80, 20 },  { 0x81, 40 },  { 0x82, 60 },  { 0x83, 80 },  { 0x84, 100 },
	{ 0x85, 140 }, { 0x86, 180 }, { 0x87, 220 }, { 0x77, 280 },
};

struct ResultFlag {
	s32 mFlag;
	u32 mBit;
};

static const ResultFlag sResultFlags[12] = {
	{ 0x74, 0x800 }, { 0x76, 0x1000 }, { 0x77, 0x400 }, { 0x75, 0x200 }, { 0x7B, 0x10 }, { 0x7D, 0x20 },
	{ 0x7E, 0x40 },  { 0x7F, 0x80 },   { 0x7C, 0x100 }, { 0x78, 0x2 },   { 0x79, 0x8 },  { 0x7A, 0x4 },
};

int TModeStage::init()
{
	StageDef* def = findStageDef(mName);

	aeLoadStage();
	playerLoadShared();
	if (mPlayerNo == 4) {
		playerEnsureArc(0);
	}
	playerEnsureArc(mPlayerNo);
	playerLoadSoundBank(mPlayerNo, 8);
	keySetWordBit(&gPlayerFlags.mBits, 2, 1);

	mStage = createStageForTheme(def->mTheme);
	mStage->load(mName);
	mStage->loadAssets();
	createItemManage();

	mMissionType = MISSION_TYPE_NONE;
	if (def->mMission != 0) {
		for (u32 i = 0; i < 15; i++) {
			if (strcmp(sMissionTypes[i].mName, def->mMission) == 0) {
				mMissionType = sMissionTypes[i].mType;
				break;
			}
		}
	}

	mStartX     = def->mSpawns[0].mPos.x;
	mStartY     = def->mSpawns[0].mPos.y;
	mStartZ     = def->mSpawns[0].mPos.z;
	mStartAngle = (int)(182.04445f * def->mSpawns[0].mAngle); // 0x10000 / 360 degrees to Ninja angle

	GXSetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
	soundSetMix(1);
	setState(0);
	return 1;
}

int TModeStage::shutdown()
{
	freePlayer();
	objClearAllLayers(&gMainTask);
	destroyItemManage();
	aeFreeStage();

	mStage->freeAssets();
	mStage->unload();
	delete mStage;

	playerFreeShared();
	seStopAll();
	bgmStop();
	bgmFadeTo(1.0f, 0);
	soundSetMix(0);
	vibStopAll();
	return 1;
}

static int isPausePressed()
{
	PadQuery query;
	query.mType = 0xA4;
	if (sendGameMsg((ObjMsg*)&query) & 1) {
		return 0;
	}

	int pressed = 0;
	if ((((StagePad*)padGet(0))->mTrigger & 0x1000) && debugEditActive() == 0 && eventIsBusy() == 0) {
		pressed = 1;
	}
	return (u8)pressed;
}

void TModeStage::update()
{
	gjBeforeRender();
	runState(mState, STATE_UPDATE);
	runState(mState, STATE_DRAW);
	objPrintTree((TObject*)&gMainTask);
	memDebugDrawBegin();
	memShowStats();
	memDebugDrawEnd();
	frameTickerGet()->update();
	endFrame();
}

void TModeStage::endFrame()
{
	if (mSkipDraw != 0) {
		mSkipDraw = 0;
		endFrameSwap();
	} else {
		gjPrintFlush();
		gjDoneRender();
		int retrace = VIGetRetraceCount();
		if (retrace > gjLastRetrace) {
			mSkipDraw    = 1;
			gjLastRetrace = retrace;
		} else {
			mSkipDraw = 0;
			endFrameSwap();
		}
	}
}

void TModeStage::startStage(int first)
{
	setRangeCheck(1);
	mStage->start(first);
	mStage->vfn_14();

	Unk8000FAC8* unk = (Unk8000FAC8*)findObjById(0);
	if (unk != 0) {
		unk->_24 = 0;
		camSetTarget(0, (TObjAction*)findObjById(0));
		StageCamera* camera = (StageCamera*)camGetView(0);
		camera->mFov        = 0x2AAA;
		camera->mNear       = 1.0f;
		camera->mFar        = 50000.0f;
	}
	createGoAnimal__Fv();
}

// 0x800230C0
void TModeStage::beginPlay()
{
	resetSets();
	clearKeyGroupLow();
	eventScriptReset();
	eventReset();
	aeCreateStageHud();
	playerSetStart(0, (const Vec3f*)&mStartX, mStartAngle);

	mContext         = createGameContext(mMissionType);
	mContext->mLives = _0E;
	if (mPlayerNo == 4) {
		mContext->mSlot = 0;
	} else {
		mContext->mSlot = mPlayerNo;
	}
	setGameContext(mContext);
	mContext->start();

	mContext->mTime = mStartTime.toMillis();

	if (mMissionType == 0 || mMissionType == 15) {
		fadeModel10();
	} else {
		fadeModel00();
	}

	if (mMissionType == 0 || mMissionType == 14 || mMissionType == 15) {
		setCameraActive(1);
		setState(4);
	} else {
		mIntro = (TIntro*)createStageTitle();
		setCameraActive(0);
		setState(1);
	}

	startStage(1);
	scoreSetRecord(&mRecord);
}

void TModeStage::restart()
{
	clearSets();
	clearKeyGroupMid();
	eventScriptReset();
	eventReset();
	aeCreateStageHud();
	mContext->respawn();
	setCameraActive(1);
	startStage(0);
	_A0 = 1;
	scoreSetRecord(&mRecord);
}

void TModeStage::stopPlay()
{
	objClearLayers(&gMainTask);
	initObjIds();
	bgmStop();
}

static int countFlags(int first, int last)
{
	int count = 0;
	for (int i = first; i <= last; i++) {
		if (isFlagSet(i)) {
			count++;
		}
	}
	return count;
}

static int countSaved(int first, int last)
{
	int count = 0;
	for (int i = first; i <= last; i++) {
		if (saveGetKeyBit(i)) {
			count++;
		}
	}
	return count;
}

void TModeStage::updateUnlocks()
{
	if (!keyBitIsSet(0x474) && countFlags(0x3C, 0x73) == 0x38) {
		setKeyBit(0x474);
	}
	if (!keyBitIsSet(0x475) && countFlags(0xC9, 0x110) == 0x48) {
		setKeyBit(0x475);
	}

	int total = countFlags(300, 0x243);
	for (u32 i = 0; i < 9 && total >= sCountUnlocks[i].mCount; i++) {
		setKeyBit(sCountUnlocks[i].mFlag + 0x400);
	}

	StageDefTable* table = getStageDefTable();
	int topRanks         = 0;
	for (int i = 0; i < 0x38; i++) {
		int points    = saveGetPoints(i);
		int time      = saveGetTime(i);
		int score     = scoreWithTime(points, time, table->mEntries[i].mTimeLimit * 1000);
		StageDef* def = &table->mEntries[i];
		if (scoreRank(score, def->mRankScores[0], def->mRankScores[1], def->mRankScores[2], def->mRankScores[3]) == 4) {
			topRanks++;
		}
	}
	if (topRanks == 0x38) {
		setKeyBit(0x476);
	}
}

// 0x800234F0
int TModeStage::updateRecord(int* outNewBest)
{
	StageDef* def = findStageDef(mName);
	int limit     = def->mTimeLimit * 1000;
	int points    = stageRecordTotal(&mSavedRecord);
	int time      = mSavedTime.toMillis();
	int oldPoints = saveGetPoints(mStageIndex);
	int oldTime   = saveGetTime(mStageIndex);
	int score     = scoreWithTime(points, time, limit);
	int oldScore  = scoreWithTime(oldPoints, oldTime, limit);
	int rank      = scoreRank(score, def->mRankScores[0], def->mRankScores[1], def->mRankScores[2], def->mRankScores[3]);

	*outNewBest = (u8)(score >= oldScore);
	if (*outNewBest != 0) {
		saveSetTime(mStageIndex, time);
		saveSetPoints(mStageIndex, points);
	}
	return rank;
}

void TModeStage::runStage()
{
	if (mSkipDraw != 0) {
		mStage->run(1);
	} else {
		mStage->run(3);
	}
}

void TModeStage::runStagePaused()
{
	objUpdateTree(gTL_08);
	if (mSkipDraw == 0) {
		mStage->run(2);
	}
}

void TModeStage::runState(int state, int phase)
{
	switch (state) {
	case 0:
		stateLoad(phase);
		break;
	case 1:
		stateIntro(phase);
		break;
	case 2:
		stateWait(phase);
		break;
	case 3:
		stateExit(phase);
		break;
	case 4:
		statePlay(phase);
		break;
	case 5:
		stateResult(phase);
		break;
	case 6:
		statePause(phase);
		break;
	case 7:
		stateMenu(phase);
		break;
	}
}

void TModeStage::setState(int state)
{
	if (mState != 8) {
		runState(mState, STATE_EXIT);
	}
	if (state != 8) {
		runState(state, STATE_ENTER);
	}
	mState = state;
}

void TModeStage::setCameraActive(int active)
{
	if (mPlayerNo == 4) {
		playerCreate(gTL_03, 0, 0, active);
	} else {
		playerCreate(gTL_03, 0, mPlayerNo, active);
	}
}

void TModeStage::freePlayer()
{
	if (mContext != 0) {
		delete mContext;
		mContext = 0;
		setGameContext(0);
	}
}

void TModeStage::setTime(const StageTime& time)
{
	mStartTime = time;
	mSavedTime = time;
}

void TModeStage::setRecord(const StageRecord& record)
{
	mRecord      = record;
	mSavedRecord = record;
}

void TModeStage::stateLoad(int phase)
{
	if (phase == STATE_UPDATE) {
		beginPlay();
	}
}

void TModeStage::stateIntro(int phase)
{
	if (phase == STATE_UPDATE) {
		TIntro* intro = mIntro;
		if (intro != 0 && intro->mDone != 0) {
			intro->mFlags |= 1;
			mIntro       = 0;
			TChrPlayer* player = (TChrPlayer*)findPlayer(0);
			if (player != 0) {
				player->resetCamera();
			}
			setState(2);
		}
	} else if (phase == STATE_DRAW) {
		runStage();
	}
}

void TModeStage::stateWait(int phase)
{
	if (phase == STATE_ENTER) {
		mTimer = 0;
	} else if (phase == STATE_UPDATE) {
		if (mTimer == 180) {
			setState(4);
		}
	} else if (phase == STATE_DRAW) {
		if (mTimer == 60) {
			aeCreateGo();
		}
		mTimer++;
		runStage();
	}
}

void TModeStage::stateExit(int phase)
{
	if (phase == STATE_ENTER) {
		freePlayer();
	}
}

static inline int isEggFlag(int flag)
{
	return keyBitIsSet(flag + 0x400);
}

// 0x80023AB0
void TModeStage::statePlay(int phase)
{
	if (phase == STATE_UPDATE) {
		switch (mContext->getStatus()) {
		case 1:
			stopPlay();
			restart();
			setState(4);
			break;
		case 3:
			_0E = mContext->mLives;
			if ((mMissionType == 0 || mMissionType == 15) && _A0 == 0) {
				scoreAddBonus();
			}
			scoreGetRecord(&mSavedRecord);
			stageTimeSet(&mSavedTime, mContext->mTime);
			storySeqEval(_98);
			if (_9C != 0) {
				_08 = 2;
				stopPlay();
				setState(3);
			} else {
				setState(5);
			}
			break;
		case 2:
			_0E = mContext->mLives;
			_08 = 3;
			stopPlay();
			setState(3);
			break;
		case 4:
			_0E = mContext->mLives;
			_08 = 6;
			stopPlay();
			setState(3);
			break;
		default:
			if (isPausePressed()) {
				mPauseMenu = (TPauseMenu*)createPause(gTL_08);
				if (mContext->mLives <= 1) {
					mPauseMenu->mNoRetry = 1;
				}
				if (_24 == 0) {
					mPauseMenu->_64 = 1;
				}
				int eggs     = 0;
				int firstEgg = mStageIndex * 5 + 300;
				for (int i = 0; i < 5; i++) {
					int flag = firstEgg;
					flag = flag + i;
					if (isEggFlag(flag)) {
						eggs++;
					}
				}
				mPauseMenu->mEggCount = eggs;
				soundPause();
				vibPauseAll();
				setState(6);
			}
			break;
		}
	} else if (phase == STATE_DRAW) {
		mContext->draw();
		runStage();
	}
}

static int isNewlySet(int flag)
{
	int result = 0;
	if (!saveGetKeyBit(flag) && isFlagSet(flag)) {
		result = 1;
	}
	return (u8)result;
}

// 0x80023D40
void TModeStage::stateResult(int phase)
{
	if (phase == STATE_ENTER) {
		loadResult();
		mResult          = createResult(gTL_01);
		mResult->mRecord = mSavedRecord;
		mResult->mTime   = mSavedTime;
		findStageDef(mName);

		int newBest;
		int rank = updateRecord(&newBest);
		updateUnlocks();
		mResult->mRank    = rank;
		mResult->mCleared = countSaved(0x3C, 0x77);
		if (newBest != 0) {
			mResult->mNewBest = 1;
		}

		u32 i;
		u32 flags = 0;
		int flagged = countFlags(0x3C, 0x73);
		int saved = countSaved(0x3C, 0x73);
		if (saved != flagged) {
			flags |= 1;
		}
		for (i = 0; i < 12; i++) {
			if (isNewlySet(sResultFlags[i].mFlag)) {
				flags |= sResultFlags[i].mBit;
			}
		}
		mResult->mFlags |= flags;
	} else if (phase == STATE_UPDATE) {
		if (mResult->mDone != 0) {
			_08 = 2;
			stopPlay();
			setState(3);
		}
	} else if (phase == STATE_DRAW) {
		mContext->draw();
		runStage();
	}
}

void TModeStage::statePause(int phase)
{
	if (phase == STATE_UPDATE) {
		TPauseMenu* menu = mPauseMenu;
		int choice;
		if (menu->mClosed != 0) {
			choice = menu->mChoice;
		} else {
			choice = -1;
		}
		if (choice != -1) {
			delete menu;
			switch (choice) {
			case 0:
				soundResume();
				vibResumeAll();
				setState(4);
				break;
			case 1:
				_0E = mContext->mLives - 1;
				seStopAll();
				bgmStop();
				stopPlay();
				freePlayer();
				beginPlay();
				break;
			case 2:
				_0E = mContext->mLives;
				_08 = 1;
				stopPlay();
				setState(3);
				break;
			case 3:
				_08 = 5;
				stopPlay();
				setState(3);
				break;
			}
		}
	} else if (phase == STATE_DRAW) {
		runStagePaused();
	}
}

void TModeStage::stateMenu(int phase)
{
	if (phase == STATE_UPDATE) {
		if (((StagePad*)padGet(0))->mTrigger & 0x1000) {
			soundResume();
			vibResumeAll();
			setState(4);
		}
	} else if (phase == STATE_DRAW) {
		if (((StagePad*)padGet(0))->mTrigger & 4) {
			mContext->draw();
			runStage();
		} else {
			runStagePaused();
		}
	}
}
