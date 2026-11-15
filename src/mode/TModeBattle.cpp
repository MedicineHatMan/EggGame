#include "ui/TAeObjPause.h"
#include "mode/TModeBattle.h"
#include "camera/EventCamArc.h"
#include "core/Pad.h"
#include "core/TMainTask.h"
#include "stage/GameState.h"
#include "stage/TGame.h"
#include "ui/TBattleResult.h"
#include "player/Player.h"
#include "engine/FrameTex.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "engine/Landscape.h"
#include "ui/AeObjControl.h"
#include "object/TObjPlayer.h"
#include "engine/DrawTask.h"
#include "egg/ObjEgg.h"
#include "item/TItemManage.h"
#include "sound/SoundCtrl.h"
#include "sound/SoundBank.h"
#include "core/Vibration.h"
#include "stage/SetSpawner.h"
#include "camera/TCameraCtrl.h"
#include "stage/TStageBattle.h"
#include "animal/AniMount.h"
#include "animal/TAniManager.h"
#include "stage/SetManager.h"
#include "object/ObjAction.h"
#include "ui/NowLoading.h"
#include "Ninja/ninja.h"
#include "stage/Stage.h"
#include "core/Mem.h"
#include "object/TObjLauncher.h"
#include "core/MathUtil.h"
#include "object/TObjEggRing.h"
#include "object/TObjKey00.h"
#include "object/TObject.h"
#include "stage/TGameBattleSurvival.h"
#include "stage/TGameBattleTime.h"
#include "stage/TGameBattleHatch.h"
#include "stage/TGameSuddenDeath.h"
#include "stage/TGameState.h"
#include "ui/ConstPalette.h"
#include "stage/TEventCtrl.h"
#include "enemy/boss/TEnePurpleBoss.h"
struct TPauseMenu;
struct TGameContextBattle;
struct CamView;

#include "mode/BattleFlags.h"

BattleFlags sBattleFlags; // 0x802C4D50

// 0x8002EA0C
TModeBattle* TModeBattle::create(TModeAllocator* allocator)
{
	return new (allocator) TModeBattle;
}

// 0x8002EA44
TModeBattle::TModeBattle()
{
	mState    = BATTLE_NONE;
	mRule     = 0;
	mTime     = 1;
	mLives    = 1;
	mName     = 0;
	mSkipDraw = 0;
	for (int i = 0; i < 4; i++) {
		mEntries[i].mPlace  = 0;
		mEntries[i].mActive = 0;
		mEntries[i].mHuman  = 0;
		mEntries[i].mChara  = 0;
		mEntries[i].mWins   = 0;
	}
	mGame = 0;
}

// 0x8002EB14
BattleEntry::BattleEntry()
{
}

// 0x8002EB18
TModeBattle::~TModeBattle()
{
}

// 0x8002EB40
void TModeBattle::setChara(int slot, int chara)
{
	mEntries[slot].mHuman  = 1;
	mEntries[slot].mActive = 1;
	mEntries[slot].mChara  = chara;
}

// 0x8002EB5C
void TModeBattle::loadFiles()
{
	char name[64];
	char* dst;
	const char* src;

	nowLoadingDisable();
	name[0] = 'k';
	name[1] = '_';
	dst     = name + 2;
	src     = mName;
	while (*src != '\0') {
		*dst++ = *src++;
	}
	dst[0] = '.';
	dst[1] = 'p';
	dst[2] = 'r';
	dst[3] = 'd';
	dst[4] = '\0';
	loadPrd("k_battle_common.prd");
	loadPrd(name);
}

// 0x8002EC04
int TModeBattle::init()
{
	setConstPalette((ConstPalette*)sConstPalette1);
	StageDef* def = findStageDef(mName);
	int players;
	TStageBattle* stage = (TStageBattle*)createBattleStage();
	mStage              = stage;
	mStage->load(mName);
	mStage->loadAssets();

	players = 0;
	for (int j = 0; j < 4; j++) {
		if (mEntries[j].mActive) {
			stage->setCamera(players, j);
			players++;
		}
	}
	int i;
	stage->setPlayers(players);
	setLandscapeFlag(0);
	aeLoadBattle();
	GXColor clear = sModeBattleClearColor;
	GXSetCopyClear(clear, 0xFFFFFF);
	frameTexInit();
	frameTexDelete(0);
	frameTexCreate(2, 640, 480, 5);
	frameTexCreate(0, 320, 240, 5);
	playerLoadShared();
	for (i = 0; i < 4; i++) {
		playerEnsureBattleArc(i);
	}
	playerLoadSoundBank(0, 8);
	playerLoadSoundBank(1, 9);
	playerLoadSoundBank(2, 10);
	playerLoadSoundBank(3, 4);
	gBattleGroups[0] = 8;
	gBattleGroups[1] = 9;
	gBattleGroups[2] = 10;
	gBattleGroups[3] = 4;
	setDrawTaskDepthRange(1000.0f, -1000.0f);
	setRangeCheck(0);
	gEggFeedFlag     = 1;
	gEggAssetsLoaded = 0;
	createItemManage();
	gItemBattle = 1;
	sBattleFlags.clear();
	sBattleFlags.set(0);
	sBattleFlags.set(1);
	keySetWordBit(&gPlayerFlags.mBits, 0, 1);
	keySetWordBit(&gPlayerFlags.mBits, 1, 1);
	GXSetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
	for (int k = 0; k < 4; k++) {
		Vec3f pos;
		getSpawnPos(&pos, mStage, k);
		mEntries[k].mPos   = pos;
		mEntries[k].mAngle = 182.04445f * def->mSpawns[k].mAngle;
		playerSetStart((u16)k, &mEntries[k].mPos, mEntries[k].mAngle);
	}
	setState(BATTLE_START);
	soundSet3DMono(1);
	soundSetMix(1);
	return 1;
}

// 0x8002EEA8
int TModeBattle::shutdown()
{
	callState(mState, BATTLE_EXIT);
	if (mGame != 0) {
		delete mGame;
	}
	soundSet3DMono(0);
	objClearLayers(&gMainTask);
	aeFreeBattle();
	destroyItemManage();
	playerFreeShared();
	mStage->freeAssets();
	mStage->unload();
	delete mStage;
	seStopAll();
	bgmStop();
	bgmFadeTo(1.0f, 0);
	soundSetMix(0);
	vibStopAll();
	return 1;
}

// 0x8002EF84
void TModeBattle::update()
{
	gjBeforeRender();
	callState(mState, BATTLE_UPDATE);
	callState(mState, BATTLE_DRAW);
	objPrintTree(&gMainTask);
	memDebugDrawBegin();
	memShowStats();
	memDebugDrawEnd();
	if (mSkipDraw != 0) {
		mSkipDraw = 0;
		endFrameSwap();
	} else {
		gjPrintFlush();
		gjDoneRender();
		int count = VIGetRetraceCount();
		if (count > gjLastRetrace) {
			mSkipDraw     = 1;
			gjLastRetrace = count;
		} else {
			mSkipDraw = 0;
			endFrameSwap();
		}
	}
}

// 0x8002F040
void TModeBattle::callState(int state, int event)
{
	switch (state) {
	case BATTLE_START:
		stateStart(event);
		break;
	case BATTLE_QUIT:
		stateQuit(event);
		break;
	case BATTLE_PLAY:
		statePlay(event);
		break;
	case BATTLE_PAUSE:
		statePause(event);
		break;
	case BATTLE_RESULT:
		stateResult(event);
		break;
	}
}

// 0x8002F0C0
void TModeBattle::setState(int state)
{
	if (mState != BATTLE_NONE) {
		callState(mState, BATTLE_EXIT);
	}
	if (state != BATTLE_NONE) {
		callState(state, BATTLE_ENTER);
	}
	mState = state;
}

// This file passes the character as a full word it saw playerCreateBattle with an int last parameter
#define playerCreateBattleInt(parent, no, chara) (((void* (*)(TObject*, int, int))playerCreateBattle)(parent, no, chara))

// 0x8002F120
void TModeBattle::startRound(int sudden)
{
	int active[4];

	mStage->start(0);
	mStage->vfn_14();
	setSpawnerNone();
	camSetModeOne();
	for (int i = 0; i < 4; i++) {
		if (mEntries[i].mActive == 0) {
			active[i] = 0;
		} else {
			active[i] = 1;
			if (sudden != 0 && mEntries[i].mHuman == 0) {
				battleStageSetHuman(i, 0);
			} else {
				battleStageSetHuman(i, 1);
				TObjPlayer* player    = (TObjPlayer*)playerCreateBattleInt(gTL_03, (u16)i, mEntries[i].mChara);
				*((u8*)player + 0x24) = i;
				if (sudden != 0) {
					player->vfn_70(10);
				}
				camSetTarget(i, player);
				StageCamera* camera = (StageCamera*)camGetView(i);
				camera->mFov        = 0x2AAA;
				camera->mNear       = 1.0f;
				camera->mFar        = 50000.0f;
			}
		}
	}
	aeSetupBattle(active[0], active[1], active[2], active[3]);
	battleStageSetupCameras();
	createAniManager();
	if (sudden != 0) {
		mGame = (TGameContextBattle*)createGameSuddenDeath();
	} else if (mRule == 0) {
		aeBattleRuleSurvival();
		mGame = (TGameContextBattle*)createGameBattleSurvival();
		((TGameBattleSurvival*)mGame)->setLives(mLives);
	} else if (mRule == 1) {
		aeBattleRuleTime();
		mGame                               = (TGameContextBattle*)createGameBattleTime();
		((TGameBattleTime*)mGame)->mSetting = mTime;
	} else if (mRule == 2) {
		aeBattleRuleHatch();
		mGame = (TGameContextBattle*)createGameBattleHatch();
		((TGameBattleHatch*)mGame)->setGoal(mGoal);
	}
	if (mGame != 0) {
		setGameContext(mGame);
		for (int i = 0; i < 4; i++) {
			if (mEntries[i].mActive != 0 && (sudden == 0 || mEntries[i].mHuman != 0)) {
				battleAddPlayer(mGame, i, mEntries[i].mChara);
			}
		}
		mGame->startState();
	}
}

// 0x8002F360
void TModeBattle::endRound()
{
	if (mGame != 0) {
		delete mGame;
		mGame = 0;
		setGameContext(0);
	}
	objClearLayers(&gMainTask);
	resetSets();
	initObjIds();
}

// The tail of BattlePlayer from _08 what a round leaves for the results
struct BattleScore {
	s8 mPlace;    // _00
	int mPoints;  // _04
	int mLost;    // _08
	int mHatched; // _0C
};

// 0x8002F3D0
void TModeBattle::readResults()
{
	for (int i = 0; i < 4; i++) {
		if (mEntries[i].mHuman != 0) {
			BattleScore* fight  = (BattleScore*)&mGame->mPlayers[battleFindPlayer(mGame, i)].mPlace;
			mEntries[i].mPlace  = fight->mPlace;
			mEntries[i].mPoints = fight->mPoints;
			mEntries[i].mLost   = fight->mLost;
			mEntries[i]._1C     = fight->mHatched;
		}
	}
}

// 0x8002F45C
void TModeBattle::resetEntries()
{
	for (int i = 0; i < 4; i++) {
		if (mEntries[i].mActive != 0) {
			mEntries[i].mPlace = 0;
			mEntries[i].mHuman = 1;
		}
	}
}

// 0x8002F494
void TModeBattle::drawPlay()
{
	if (mGame != 0) {
		mGame->draw();
	}
	if (mSkipDraw != 0) {
		mStage->run(1);
	} else {
		mStage->run(3);
	}
}

// 0x8002F518
void TModeBattle::drawPause()
{
	objUpdateTree(gTL_08);
	if (mSkipDraw == 0) {
		mStage->run(2);
	}
}

// 0x8002F56C
void TModeBattle::countWins()
{
	for (int i = 0; i < 4; i++) {
		if (mEntries[i].mActive != 0 && (int)mEntries[i].mPlace == 1) {
			mEntries[i].mWins++;
		}
	}
}

// 0x8002F5AC
int TModeBattle::checkPause()
{
	if (eventIsBusy() != 0) {
		return 0;
	}
	for (int i = 0; i < 4; i++) {
		if (mEntries[i].mActive != 0 && (padGet(i)->mPressed & 0x1000)) {
			mPause = (TPauseMenu*)createPause(gTL_08);
			((TAeObjPause*)mPause)->setPad(i);
			soundPause();
			vibPauseAll();
			return 1;
		}
	}
	return 0;
}

// 0x8002F650
void TModeBattle::stateStart(int event)
{
	if (event == BATTLE_ENTER) {
	} else if (event == BATTLE_UPDATE) {
		startRound(0);
		setState(BATTLE_PLAY);
	}
}

// 0x8002F69C
void TModeBattle::stateQuit(int event)
{
	if (event == BATTLE_ENTER) {
		_08 = 0;
	}
}

// 0x8002F6B0
void TModeBattle::statePlay(int event)
{
	if (event == BATTLE_UPDATE) {
		if (mGame != 0) {
			if (mGame->getStatus() == 6) {
				readResults();
				for (int i = 0; i < 4; i++) {
					if (mEntries[i].mHuman != 0 && (int)mEntries[i].mPlace != 1) {
						mEntries[i].mHuman = 0;
					}
				}
				bgmStop();
				endRound();
				startRound(1);
			} else if (mGame->getStatus() == 5) {
				readResults();
				countWins();
				seStopAll();
				setState(BATTLE_RESULT);
			} else if (checkPause() != 0) {
				setState(BATTLE_PAUSE);
			}
		}
	} else if (event == BATTLE_DRAW) {
		drawPlay();
	}
}

// 0x8002F7CC
void TModeBattle::statePause(int event)
{
	if (event == BATTLE_UPDATE) {
		TPauseMenu* pause = mPause;
		int choice;
		if (pause->mClosed != 0) {
			choice = pause->mChoice;
		} else {
			choice = -1;
		}
		if (choice != -1) {
			delete pause;
			switch (choice) {
			case 0:
				soundResume();
				vibResumeAll();
				setState(BATTLE_PLAY);
				break;
			case 1:
				bgmStop();
				endRound();
				resetEntries();
				startRound(0);
				setState(BATTLE_PLAY);
				break;
			case 2:
				_08 = 1;
				break;
			case 3:
				_08 = 5;
				break;
			}
		}
	} else if (event == BATTLE_DRAW) {
		drawPause();
	}
}

// 0x8002F8D0
void TModeBattle::stateResult(int event)
{
	if (event == BATTLE_ENTER) {
		setSpawnerOff();
		mResult = createTBattleResult(gTL_08);
		for (int i = 0; i < 4; i++) {
			if (mEntries[i].mActive != 0) {
				if (mEntries[i].mHuman != 0) {
					TCamPath* pose;
					if ((int)mEntries[i].mPlace == 1) {
						pose = eventCamArcPlayFor(1.0f, 0.0f, (u16)i, 11);
					} else {
						pose = eventCamArcPlayFor(1.0f, 0.0f, (u16)i, 12);
					}
					pose->attachToMtx();
					ObjMsg msg;
					msg.mType = 0x82;
					sendMsgByIdU16(i, &msg);
					f32 mtx[12];
					Vec3f pos;
					findPlayer((u16)i)->vfn_A8(0x38, mtx);
					mtxGetPos(mtx, &pos);
					mtxSetTransV(mtx, &pos);
					mtxRotateY(mtx, mtx, mEntries[i].mAngle);
					mtxCopy(pose->getMtx(), mtx);
				}
				BattleResultRow row;
				row.mPlace  = mEntries[i].mPlace;
				row.mWins   = mEntries[i].mWins;
				row.mPoints = mEntries[i].mPoints;
				row.mLost   = mEntries[i].mLost;
				row._08     = mEntries[i]._1C;
				mResult->setRow(i, &row);
			}
		}
	} else if (event == BATTLE_EXIT) {
		mResult->mFlags |= 1;
	} else if (event == BATTLE_UPDATE) {
		if (mResult->mDone != 0) {
			bgmFadeTo(0.0f, 60);
			setState(BATTLE_QUIT);
		}
	} else if (event == BATTLE_DRAW) {
		mStage->run(3);
	}
}

// 0x8002FAA4
// Declared in the class and defined here so startRound's call only marks it and it comes out last ahead of __sinit
inline void TGameContextBattle::startState()
{
}
