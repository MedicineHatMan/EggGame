#include "mode/TModeBattleSetup.h"
#include "ui/TextVec.h"
#include "io/LangFile.h"
#include "io/ResFile.h"
#include "io/Resource.h"
#include "ui/TAeObjBattleSelect.h"
#include "ui/TBattleModelPlayer.h"
#include "ui/TBattleModelHiyoko.h"
#include "stage/TStageSelectModel.h"
#include "ui/TSimpleText.h"
#include "ui/TAeObjMsgWindow.h"
#include "core/TMainTask.h"
#include "ui/MenuModelArc.h"
#include "core/Pad.h"
#include "camera/TCamera.h"
#include "camera/TCameraCtrl.h"
#include "engine/Viewport.h"
#include "engine/DrawTask.h"
#include "sound/SoundCtrl.h"
#include "ui/Font.h"
#include "object/TObjLauncher.h"
#include "core/dolphin.h"
#include "ui/TFade.h"
#include "stage/Stage.h"
#include "Ninja/ninja.h"
#include "ui/MessageTable.h"
struct TBattleModelHiyokoProp;

// The stage of each stage slot an index into mStages then the model the stage slot shows
static const int sBattleStageOrder[8] = { 6, 0, 1, 2, 3, 4, 5, 7 }; // 0x8024BF80
static const int sBattleStageModel[8] = { 0, 0, 1, 2, 3, 4, 5, 6 }; // 0x8024BFA0

// mes_battle_menu.bin fixed up like mes_main_menu.bin
BinFile* sBattleMes;      // 0x802C5998
int sBattleMesCount;   // 0x802C599C
int* sBattleMesText;   // 0x802C59A0

#define BATTLE_STRING(i) ((void*)sBattleMesText[i])

inline Vec3f vec3f(f32 x, f32 y, f32 z)
{
	Vec3f v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

inline SetRot setRot(int x, int y, int z)
{
	SetRot r;
	r.x = x;
	r.y = y;
	r.z = z;
	return r;
}

// Built by the static initializer at 0x80196190
static TextVec sBattleGlyph      = textVec(0.91f, 0.91f, 0.91f);   // 0x80286688
static TextVec sBattleBox        = textVec(394.0f, 44.0f, 0.0f);   // 0x80286694
static TextVec sBattleTitleAt    = textVec(44.0f, 41.0f, -2.0f);   // 0x802866A0
static TextVec sBattleHelpBox    = textVec(432.0f, 50.0f, 0.0f);   // 0x802866AC
static TextVec sBattleHelpAt     = textVec(142.0f, 378.0f, -2.0f); // 0x802866B8
static TextVec sBattleTab0At = textVec(565.0f, 38.0f, -2.0f); // 0x802866C4
static TextVec sBattleTab1At = textVec(565.0f, 75.0f, -2.0f); // 0x802866D0
static TextVec sBattleHead0At = textVec(98.0f, 125.0f, -2.0f); // 0x802866DC
static TextVec sBattleHead1At = textVec(98.0f, 180.0f, -2.0f); // 0x802866E8
static TextVec sBattleHead2At = textVec(98.0f, 235.0f, -2.0f); // 0x802866F4
static TextVec sBattleKindAt     = textVec(294.0f, 121.0f, -2.0f); // 0x80286700
static TextVec sBattleAmountAt   = textVec(294.0f, 177.0f, -2.0f); // 0x8028670C
static TextVec sBattleYesAt = textVec(512.0f, 320.0f, -2.0f); // 0x80286718
static TextVec sBattleNoAt = textVec(512.0f, 373.0f, -2.0f); // 0x80286724
static TextVec sBattlePlayerPos[4] = {
	textVec(30.0f, -4.0f, 75.0f), textVec(10.0f, -4.0f, 75.0f), textVec(-10.0f, -4.0f, 75.0f), textVec(-30.0f, -4.0f, 75.0f),
}; // 0x80286730
static TextRot sBattlePlayerRot[4] = {
	textRot(0, 0x8B00, 0), textRot(0, 0x8500, 0), textRot(0, 0x7B00, 0), textRot(0, 0x7500, 0),
}; // 0x80286760
static TextVec sBattleStagePos   = textVec(-34.0f, 7.0f, 100.0f);   // 0x80286790
static TextRot sBattleStageRot  = textRot(0x1000, 0x7800, 0);      // 0x8028679C
static TextVec sBattleHiyokoPos  = textVec(40.0f, -27.5f, 85.0f);    // 0x802867A8
static TextRot sBattleHiyokoRot = textRot(-0x800, 0xA000, 0);      // 0x802867B4
static TextVec sBattleChickPos   = textVec(45.0f, -36.0f, 89.0f);    // 0x802867C0
static TextRot sBattleChickRot  = textRot(-0x800, 0x8000, 0);      // 0x802867CC

// Sets the object's retire bit and drops the pointer
template <typename T> inline void freeObj(T*& obj)
{
	if (obj != 0) {
		((TObject*)obj)->mFlags |= 1;
		obj = 0;
	}
}

inline void showObj(TObject* obj)
{
	if (obj != 0) {
		obj->mFlags &= ~0x10;
	}
}

inline void hideObj(TObject* obj)
{
	if (obj != 0) {
		obj->mFlags |= 0x10;
	}
}

// Gives a text the box of a tab
inline void setTabBox(TSimpleText* text)
{
	if (text != 0) {
		text->mBox.x = 0.0f;
		text->mBox.y = 24.0f;
		text->mBox.z = 0.0f;
		text->_A8 = 1;
	}
}

// A text the size of a tab shown
inline void showTab(TSimpleText* text)
{
	if (text != 0) {
		text->mFlags &= ~0x10;
		text->mBox.x = 0.0f;
		text->mBox.y = 24.0f;
		text->mBox.z = 0.0f;
		text->_A8 = 1;
	}
}

// 0x801935CC
TModeBattleSetup* TModeBattleSetup::create(TModeAllocator* allocator)
{
	return new (allocator) TModeBattleSetup;
}

// 0x80193604
void TModeBattleSetup::loadFiles()
{
	loadPrd("k_battle_select.prd");
}

// The DOL copies the rotations as blocks paired loads which CW does for a struct holding an array but not for SetRot's
// three ints
struct RotBlock {
	int v[3];
};

// 0x8019362C
int TModeBattleSetup::init()
{
	GXSetCopyClear(*(GXColor*)&sModeBattleSetupClearColor, 0xFFFFFF);
	camCtrlInit();
	viewportInit();
	initDrawTasks();
	loadMsgWindow();
	loadBattleSelect();
	loadBattleModelArc();
	loadBattleModelPlayer();
	loadBattleModelHiyoko();
	gjSetAmbient(0.7f, 0.7f, 0.7f);
	gjSetLightSwitch(0, 1);
	gjSetLightVector(0, 0.5f, -1.0f, 0.0f);
	gjSetLightColor(0, 1.0f, 1.0f, 1.0f);
	gjLoadLight(0);
	objClearLayers(&gMainTask);
	createCameraCtrl();
	camSetDrawView(0);
	setViewport(0);
	fontCreateTask();

	char* file = getLangFileName("mes_battle_menu.bin");
	loadSizedResource(file, &sBattleMes);
	MessageFile* header = (MessageFile*)sBattleMes->mData;
	int base        = header->mTextOffset;
	u32 size        = header->mTableSize;
	u32 text        = base + (u32)header;
	int* msgs       = (int*)header->mOffsets;
	sBattleMesText  = msgs;
	sBattleMesCount = size >> 2;
	int i;
	for (i = 0; i < sBattleMesCount; i++) {
		sBattleMesText[i] += text + 8;
	}

	createBattleSelect();
	buildText();
	hideAll();

	mStagePos = sBattleStagePos;
	*(RotBlock*)&mStageRot = *(RotBlock*)&sBattleStageRot;
	mStage    = createTStageSelectModel(gTL_05, &mStagePos, &mStageRot.x, sBattleStageModel[1], 0x21);
	if (mStage != 0) {
		mStage->mLightColor.set(1.0f, 1.0f, 1.0f);
		mStage->mAmbient.set(0.7f, 0.7f, 0.7f);
		((Vec3f*)&mStage->mLightX)->set(1.0f, -1.0f, -1.0f);
	}

	for (i = 0; i < 4; i++) {
		mPlayerPos[i] = sBattlePlayerPos[i];
		*(RotBlock*)&mPlayerRot[i] = *(RotBlock*)&sBattlePlayerRot[i];
		mPlayer[i]    = createTBattleModelPlayer(gTL_05, &mPlayerPos[i], &mPlayerRot[i].x, i);
	}

	mHiyokoPos = sBattleHiyokoPos;
	*(RotBlock*)&mHiyokoRot = *(RotBlock*)&sBattleHiyokoRot;
	mHiyoko    = createTBattleModelHiyoko(gTL_05, &mHiyokoPos, &mHiyokoRot.x);
	mChickPos  = sBattleChickPos;
	*(RotBlock*)&mChickRot = *(RotBlock*)&sBattleChickRot;
	mChick     = createTBattleModelHiyokoProp(gTL_05, &mChickPos, &mChickRot.x);

	StageDefTable* table = getStageDefTable();
	for (i = 0; i < table->mCount; i++) {
		const char* name = table->mEntries[i].mName;
		if (strncmp(name, "battle", 6) == 0) {
			mStages.push(name);
		}
	}

	start();
	bgmPlay(1);
	mStageOpen[0] = 1;
	mStageOpen[1] = 1;
	return 1;
}

// 0x801939A4
int TModeBattleSetup::shutdown()
{
	freeFade();
	freeObj(_58);
	freeObj(mWindow);
	freeObj(mTitle);
	freeObj(mTab[0]);
	freeObj(mTab[1]);
	freeObj(mRuleKind);
	freeObj(mRuleAmount);
	freeObj(mRuleHelp);
	freeObj(mRuleHead[0]);
	freeObj(mRuleHead[1]);
	freeObj(mRuleHead[2]);
	if (sBattleMes != 0) {
		releaseResource(sBattleMes);
		sBattleMes = 0;
	}
	objClearLayers(&gMainTask);
	freeBattleModelHiyoko();
	freeBattleModelPlayer();
	freeBattleModelArc();
	freeBattleSelect();
	freeMsgWindow();
	termDrawTasks();
	viewportTerm();
	camCtrlTerm();
	bgmStop();
	return 1;
}

// 0x80193B7C
TModeBattleSetup::TModeBattleSetup()
{
	mState       = BSETUP_CHARA;
	mFade        = 0;
	mFadeTimer   = 0;
	mTimer       = 0;
	mBoxPad      = 0;
	mTitle       = 0;
	mTab[0]      = 0;
	mTab[1]      = 0;
	mRuleHead[0] = 0;
	mRuleHead[1] = 0;
	mRuleHead[2] = 0;
	mRuleKind    = 0;
	mRuleAmount  = 0;
	mRuleHelp    = 0;
	mYes         = 0;
	mNo          = 0;
	mBox         = 0;
	mBoxKind     = 0;
	mChoice      = 0;
	mWindow      = 0;
	_58          = 0;
	for (s8 i = 0; i < 4; i++) {
		mPlayer[i] = 0;
	}
	mStage  = 0;
	mHiyoko = 0;
	mChick  = 0;
	for (int i = 0; i < 4; i++) {
		mTaken[i]  = 0;
		mCursor[i] = 0;
		mChara[i]  = -1;
		mPad[i]    = 0;
	}
	mRow      = 0;
	mRule     = 1;
	mAmount   = 1;
	mStageIdx = 0;
	openStages(1);
}

// 0x80193D5C
const char* TModeBattleSetup::getStageName()
{
	return mStages.mData[sBattleStageOrder[mStageIdx]];
}

// 0x80193D80
void TModeBattleSetup::update()
{
	gMainTask.draw();
	gjBeforeRender();
	switch (mState) {
	case BSETUP_NO_PADS:
		updateNoPads();
		break;
	case BSETUP_ENTRY_WAIT:
		updateEntryWait();
		break;
	case BSETUP_CHARA:
		updateChara();
		break;
	case BSETUP_CHARA_DONE:
		updateCharaDone();
		break;
	case BSETUP_RULE_WAIT:
		updateRuleWait();
		break;
	case BSETUP_RULE:
		updateRule();
		break;
	case BSETUP_RULE_BACK:
		updateRuleBack();
		break;
	case BSETUP_START:
		updateStart();
		break;
	case BSETUP_QUIT:
		updateQuit();
		break;
	case BSETUP_QUIT_FADE:
		updateQuitFade();
		break;
	}
	gjSetRenderMode(3);
	GXSetCullMode(GX_CULL_NONE);
	setViewport(0);
	camSetDrawView(0);
	gMainTask.vfn_10();
	gjDisableFog();
	runDrawTasks(0x3C);
	runDrawTasks(0x21);
	runDrawTasks(0x3E);
	runDrawTasks(0x3F);
	gjPrintFlush();
	gjDoneRender();
	endFrameSwap();
}

// 0x80193EAC
// Two pads are needed
void TModeBattleSetup::start()
{
	aeBattleSelectEnterChara();
	hideObj(mStage);
	int ready = 0;
	for (int i = 0; i < 4; i++) {
		if (padGet((u8)i)->mStatus == 0) {
			ready++;
		}
	}
	if (ready < 2) {
		alphaFadeDim(0x3E);
		openMessage(BATTLE_STRING(31));
		hideTitle();
		hideTabs();
		mState = BSETUP_NO_PADS;
	} else {
		enterEntry();
	}
}

// 0x80193F60
void TModeBattleSetup::updateNoPads()
{
	if (runBox() != 0) {
		alphaFadeClear();
		enterEntry();
	}
}

// 0x80193FA0
void TModeBattleSetup::enterEntry()
{
	aeBattleSelectEnterChara();
	for (int i = 0; i < 4; i++) {
		mTaken[i]  = 0;
		mCursor[i] = 0;
		mChara[i]  = -1;
		aeBattleSelectLeave(i, i);
	}
	hideObj(mStage);
	mTimer = 90;
	mState = BSETUP_ENTRY_WAIT;
}

// 0x80194038
void TModeBattleSetup::updateEntryWait()
{
	if (mTimer == 0) {
		enterChara();
	} else {
		mTimer--;
	}
}

// 0x80194070
void TModeBattleSetup::enterChara()
{
	hideAll();
	showTitle(0);
	showTabs();
	for (int i = 0; i < 4; i++) {
		aeBattleSelectSetDigits(i, mPad[i]);
		TBattleModelPlayer* player = mPlayer[i];
		player->mDrop              = 0.0f;
		player->mFalling           = 0;
		mPlayer[i]->mPos           = sBattlePlayerPos[i];
		mPlayer[i]->mAngle.v[1]    = sBattlePlayerRot[i].y;
		mPlayer[i]->play(0);
		mPlayer[i]->mFlags &= ~0x10;
	}
	hideObj(mStage);
	if (mHiyoko != 0) {
		mHiyoko->play(0);
	}
	mState = BSETUP_CHARA;
}

// 0x80194198
// Each pad moves its cursor over the four fighters and takes one with A
void TModeBattleSetup::updateChara()
{
	int hover[4];
	hover[3] = 0;
	hover[2] = 0;
	hover[1] = 0;
	hover[0] = 0;

	int i;
	int joined = 0;
	for (i = 0; i < 4; i++) {
		if (padGet((u8)i)->mStatus == 0) {
			aeBattleSelectBindIcon(i, mCursor[i]);
		} else {
			aeBattleSelectClearRow(i);
			if (mTaken[mCursor[i]] == 1 && mChara[i] != -1) {
				sePlay(0x20);
				mPlayer[mChara[i]]->play(0);
				aeBattleSelectLeave(i, mChara[i]);
				mTaken[mCursor[i]] = 0;
				mChara[i]          = -1;
			}
		}
		if (mChara[i] == -1) {
			if ((padGet((u8)i)->mRepeat & 0x400000) || (padGet((u8)i)->mRepeat & 1)) {
				sePlay(0x1B);
				if (mCursor[i] > 0) {
					mCursor[i]--;
				} else {
					mCursor[i] = 3;
				}
				aeBattleSelectShowIcon(i, mCursor[i]);
			} else if ((padGet((u8)i)->mRepeat & 0x800000) || (padGet((u8)i)->mRepeat & 2)) {
				sePlay(0x1B);
				if (mCursor[i] < 3) {
					mCursor[i]++;
				} else {
					mCursor[i] = 0;
				}
				aeBattleSelectShowIcon(i, mCursor[i]);
			}
			if (padGet((u8)i)->mStatus == 0) {
				hover[mCursor[i]] = 1;
			}
			if (padGet((u8)i)->mPressed & 0x100) {
				if (mTaken[mCursor[i]] == 0) {
					sePlay(0x1C);
					mChara[i]          = mCursor[i];
					mTaken[mCursor[i]] = 1;
					aeBattleSelectJoin(i, mChara[i]);
					mPlayer[mChara[i]]->play(2);
				} else {
					sePlay(0x22);
				}
			}
		} else {
			joined++;
			if ((padGet((u8)i)->mPressed & 0x200) && mTaken[mCursor[i]] == 1) {
				sePlay(0x20);
				mPlayer[mChara[i]]->play(0);
				aeBattleSelectLeave(i, mChara[i]);
				mTaken[mCursor[i]] = 0;
				mChara[i]          = -1;
				joined--;
				if (joined == 0) {
					return;
				}
			}
		}
	}

	for (i = 0; i < 4; i++) {
		if (mTaken[i] == 0) {
			if (hover[i] != 0) {
				mPlayer[i]->play(1);
			} else {
				mPlayer[i]->play(0);
			}
		}
	}

	if (joined == 0) {
		for (i = 0; i < 4; i++) {
			if (padGet((u8)i)->mPressed & 0x200) {
				sePlay(0x20);
				mBoxPad = i;
				enterQuit();
				return;
			}
		}
	} else if (joined >= 2) {
		for (i = 0; i < 4; i++) {
			if (padGet((u8)i)->mPressed & 0x1000) {
				sePlay(0x1C);
				aeBattleSelectResetBanner();
				enterCharaDone();
			}
		}
		aeBattleSelectBindBanner();
	} else {
		aeBattleSelectClearBanner();
	}
}

// 0x8019459C
void TModeBattleSetup::enterCharaDone()
{
	hideAll();
	aeBattleSelectLeaveChara();
	if (mHiyoko != 0) {
		mHiyoko->play(1);
	}
	mTimer = 90;
	mState = BSETUP_CHARA_DONE;
}

// 0x801945F0
// The chosen fighters drop away a second before the end
void TModeBattleSetup::updateCharaDone()
{
	if (mTimer == 30) {
		for (int i = 0; i < 4; i++) {
			if (mTaken[i] != 0) {
				mPlayer[i]->startFall();
			}
		}
		mTimer--;
	} else if (mTimer == 0) {
		enterRuleWait();
	} else {
		mTimer--;
	}
}

// 0x8019467C
void TModeBattleSetup::enterRuleWait()
{
	aeBattleSelectEnterRules();
	for (int i = 0; i < 4; i++) {
		mPlayer[i]->mFlags |= 0x10;
		TBattleModelPlayer* player = mPlayer[i];
		player->mDrop              = 0.0f;
		player->mFalling           = 0;
	}
	if (mHiyoko != 0) {
		mHiyoko->play(2);
	}
	mTimer = 60;
	mState = BSETUP_RULE_WAIT;
}

// 0x80194708
void TModeBattleSetup::updateRuleWait()
{
	if (mTimer == 0) {
		enterRule();
	} else {
		mTimer--;
	}
}

// 0x80194740
void TModeBattleSetup::enterRule()
{
	mRule     = 1;
	mStageIdx = 0;
	hideAll();
	showTitle(1);
	showTabs();
	showRuleKind();
	showRuleAmount();
	showRuleHelp();
	if (mStage != 0) {
		mStage->mFlags &= ~0x10;
		TStageSelectModel* stage = mStage;
		stage->mIndex            = sBattleStageModel[mStageIdx];
		stage->mFrame            = 0.0f;
	}
	mRow = 0;
	aeBattleSelectPickRule(mRow);
	aeBattleSelectShowStage(mStageIdx);
	showStageIcons();
	mState = BSETUP_RULE;
}

// 0x80194814
// Up and down pick the row A starts B goes back to the fighters
void TModeBattleSetup::updateRule()
{
	if (mTimer != 0) {
		mTimer--;
	} else {
		for (int i = 0; i < 4; i++) {
			if ((padGet((u8)i)->mRepeat & 0x100000) || (padGet((u8)i)->mRepeat & 8)) {
				sePlay(0x1B);
				if (mRow != 0) {
					mRow--;
				} else {
					mRow = 2;
				}
				aeBattleSelectPickRule(mRow);
				showRuleHelp();
			} else if ((padGet((u8)i)->mRepeat & 0x200000) || (padGet((u8)i)->mRepeat & 4)) {
				sePlay(0x1B);
				if (mRow < 2) {
					mRow++;
				} else {
					mRow = 0;
				}
				aeBattleSelectPickRule(mRow);
				showRuleHelp();
			}
			if (padGet((u8)i)->mPressed & 0x100) {
				sePlay(0x1C);
				enterStart();
				return;
			}
			if (padGet((u8)i)->mPressed & 0x200) {
				sePlay(0x20);
				enterRuleBack();
			}
		}
		switch (mRow) {
		case 0:
			updateRuleKind();
			break;
		case 1:
			updateRuleAmount();
			break;
		case 2:
			updateRuleStage();
			break;
		}
	}
}

// 0x801949C4
void TModeBattleSetup::enterRuleBack()
{
	hideAll();
	aeBattleSelectLeaveRules();
	aeBattleSelectClearIcons();
	if (mHiyoko != 0) {
		mHiyoko->play(1);
	}
	mTimer = 90;
	mState = BSETUP_RULE_BACK;
}

// 0x80194A1C
void TModeBattleSetup::updateRuleBack()
{
	if (mTimer == 0) {
		enterEntry();
	} else {
		mTimer--;
	}
}

// 0x80194A54
void TModeBattleSetup::updateRuleKind()
{
	for (int i = 0; i < 4; i++) {
		if ((padGet((u8)i)->mRepeat & 0x400000) || (padGet((u8)i)->mRepeat & 1)) {
			sePlay(0x1B);
			if (mRule != 0) {
				mRule--;
			} else {
				mRule = 2;
			}
			aeBattleSelectBindPairB(mRow);
			showRuleKind();
			showRuleAmount();
			showRuleHelp();
		} else if ((padGet((u8)i)->mRepeat & 0x800000) || (padGet((u8)i)->mRepeat & 2)) {
			sePlay(0x1B);
			if (mRule < 2) {
				mRule++;
			} else {
				mRule = 0;
			}
			aeBattleSelectBindPairC(mRow);
			showRuleKind();
			showRuleAmount();
			showRuleHelp();
		}
	}
}

// 0x80194B78
// Every rule has three amounts
void TModeBattleSetup::updateRuleAmount()
{
	int i;
	int count = 1;
	switch (mRule) {
	case 0:
		count = 3;
		break;
	case 1:
		count = 3;
		break;
	case 2:
		count = 3;
		break;
	}
	if (mAmount > count - 1) {
		mAmount = count - 1;
	}
	for (i = 0; i < 4; i++) {
		if ((padGet((u8)i)->mRepeat & 0x400000) || (padGet((u8)i)->mRepeat & 1)) {
			sePlay(0x1B);
			if (mAmount != 0) {
				mAmount--;
			} else {
				mAmount = count - 1;
			}
			aeBattleSelectBindPairB(mRow);
			showRuleAmount();
		} else if ((padGet((u8)i)->mRepeat & 0x800000) || (padGet((u8)i)->mRepeat & 2)) {
			sePlay(0x1B);
			if (mAmount < count - 1) {
				mAmount++;
			} else {
				mAmount = 0;
			}
			aeBattleSelectBindPairC(mRow);
			showRuleAmount();
		}
	}
}

// 0x80194CD4
// Steps to the next open stage wrapping round
void TModeBattleSetup::updateRuleStage()
{
	int i;
	int found = 0;
	for (i = 0; i < 4; i++) {
		if ((padGet((u8)i)->mRepeat & 0x400000) || (padGet((u8)i)->mRepeat & 1)) {
			sePlay(0x1B);
			if (mStageIdx != 0) {
				mStageIdx--;
				for (int s = mStageIdx; s >= 0; s--) {
					if (mStageOpen[s] != 0) {
						mStageIdx = s;
						found     = 1;
						break;
					}
				}
				if (!found) {
					for (int s = 7; s > mStageIdx; s--) {
						if (mStageOpen[s] != 0) {
							mStageIdx = s;
							break;
						}
					}
				}
			} else {
				mStageIdx = 7;
				for (int s = mStageIdx; s >= 0; s--) {
					if (mStageOpen[s] != 0) {
						mStageIdx = s;
						break;
					}
				}
			}
			aeBattleSelectShowStage(mStageIdx);
			TStageSelectModel* stage = mStage;
			stage->mIndex = sBattleStageModel[mStageIdx];
			stage->mFrame = 0.0f;
		} else if ((padGet((u8)i)->mRepeat & 0x800000) || (padGet((u8)i)->mRepeat & 2)) {
			sePlay(0x1B);
			if (mStageIdx < 7) {
				mStageIdx++;
				for (int s = mStageIdx; s < 8; s++) {
					if (mStageOpen[s] != 0) {
						mStageIdx = s;
						found     = 1;
						break;
					}
				}
				if (!found) {
					for (int s = 0; s < mStageIdx; s++) {
						if (mStageOpen[s] != 0) {
							mStageIdx = s;
							break;
						}
					}
				}
			} else {
				mStageIdx = 0;
				for (int s = mStageIdx; s < 8; s++) {
					if (mStageOpen[s] != 0) {
						mStageIdx = s;
						break;
					}
				}
			}
			aeBattleSelectShowStage(mStageIdx);
			TStageSelectModel* stage = mStage;
			stage->mIndex = sBattleStageModel[mStageIdx];
			stage->mFrame = 0.0f;
		}
	}
}

// 0x80194F98
void TModeBattleSetup::enterStart()
{
	hideAll();
	startFade();
	mState = BSETUP_START;
}

// 0x80194FD4
void TModeBattleSetup::updateStart()
{
	if (fadeDone() != 0) {
		_08 = 0;
	}
}

// 0x80195010
void TModeBattleSetup::enterQuit()
{
	alphaFadeDim(0x3E);
	openYesNo(BATTLE_STRING(30));
	hideTitle();
	hideTabs();
	mState = BSETUP_QUIT;
}

// 0x80195068
void TModeBattleSetup::updateQuit()
{
	if (runBox() != 0) {
		if (boxYes() != 0) {
			hideAll();
			closeBox();
			alphaFadeClear();
			startFade();
			mState = BSETUP_QUIT_FADE;
		} else {
			closeBox();
			alphaFadeClear();
			enterChara();
		}
	}
}

// 0x801950E8
void TModeBattleSetup::updateQuitFade()
{
	if (fadeDone() != 0) {
		_08 = 1;
	}
}

// 0x80195124
int TModeBattleSetup::padDecide(int pad)
{
	return padGet((u8)pad)->mPressed & 0x100;
}

// 0x80195150
int TModeBattleSetup::padCancel(int pad)
{
	return padGet((u8)pad)->mPressed & 0x200;
}

// 0x8019517C
int TModeBattleSetup::padRepeat(int pad, u32 mask)
{
	if (mask & 0x200004) {
		mask |= 0x200004;
	} else if (mask & 0x100008) {
		mask |= 0x100008;
	} else if (mask & 0x800002) {
		mask |= 0x800002;
	} else if (mask & 0x400001) {
		mask |= 0x400001;
	}
	return mask & padGet((u8)pad)->mRepeat;
}

// 0x80195220
void TModeBattleSetup::closeBox()
{
	mBoxKind = 0;
	mChoice  = 0;
	if (mBox != 0) {
		mBox->mHold = 1;
		mBox        = 0;
	}
	closeWindow();
}

// 0x80195264
void TModeBattleSetup::closeWindow()
{
	TAeObjMsgWindow* window = mWindow;
	if (window != 0) {
		window->mDieOnClose = TRUE;
		window->close();
		mWindow = 0;
	}
	hideYesNo();
}

// 0x801952B4
void TModeBattleSetup::openYesNo(void* text)
{
	closeBox();
	mChoice  = 1;
	mBoxKind = 1;
	if (mBox == 0) {
		mBox = createSimpleTextByKind(gTL_07, text, 3);
		if (mBox != 0) {
			mBox->_A8 = 1;
		}
	}
	if (mWindow == 0) {
		mWindow = createMsgWindow(gTL_07, 9);
		if (mWindow != 0) {
			mWindow->setPrompt(3);
			mWindow->setCursor(mChoice);
		}
	}
	showYesNo();
}

// 0x8019536C
void TModeBattleSetup::openMessage(void* text)
{
	closeBox();
	mBoxKind = 2;
	if (mBox == 0) {
		mBox = createSimpleTextByKind(gTL_07, text, -1);
		if (mBox != 0) {
			mBox->_A8 = 1;
		}
	}
}

// 0x801953D8
int TModeBattleSetup::runBox()
{
	int done = 0;
	switch (mBoxKind) {
	case 0:
	default:
		done = 1;
		break;
	case 1:
		if (padRepeat(mBoxPad, 4) != 0) {
			mChoice++;
			mChoice = mChoice >= 2 ? 0 : mChoice;
			sePlay(0x1B);
		} else if (padRepeat(mBoxPad, 8) != 0) {
			mChoice--;
			mChoice = mChoice < 0 ? 1 : mChoice;
			sePlay(0x1B);
		}
		if (mWindow != 0) {
			mWindow->setCursor(mChoice);
		}
		if (padCancel(mBoxPad)) {
			sePlay(0x20);
			done    = 1;
			mChoice = 1;
		} else if (padDecide(mBoxPad)) {
			sePlay(0x1C);
			done = 1;
		}
		break;
	case 2:
		if (mBox != 0 && mBox->mState == 3) {
			done = 1;
		}
		break;
	}
	return done;
}

// 0x8019554C
int TModeBattleSetup::boxYes()
{
	return mChoice == 0;
}

// 0x80195560
void TModeBattleSetup::freeFade()
{
	freeObj(mFade);
	mFadeTimer = 0;
}

// 0x8019558C
void TModeBattleSetup::startFade()
{
	freeFade();
	mFade      = new TWhiteFadeInOut(gTL_01, 1);
	mFadeTimer = 60;
}

// 0x801955E8
int TModeBattleSetup::fadeDone()
{
	if (mFadeTimer == 0) {
		return 1;
	}
	mFadeTimer--;
	return 0;
}

// 0x8019560C
#define SET_TAB_BOX(t) if ((t) != 0) { (t)->mBox.set(0.0f, 24.0f, 0.0f); (t)->_A8 = 1; }

void TModeBattleSetup::buildText()
{
	mTitle      = createSimpleTextOnTask(1, -1, &sBattleTitleAt, &sBattleBox, &sBattleGlyph, BATTLE_STRING(0));
	mTitle->_A9 = 1;
	mTab[0]     = createSimpleTextOnTask(1, -1, &sBattleTab0At, &sBattleBox, &sBattleGlyph, BATTLE_STRING(2));
	SET_TAB_BOX(mTab[0]);
	mTab[1] = createSimpleTextOnTask(1, -1, &sBattleTab1At, &sBattleBox, &sBattleGlyph, BATTLE_STRING(3));
	SET_TAB_BOX(mTab[1]);
	mRuleHead[0] = createSimpleTextOnTask(1, -1, &sBattleHead0At, &sBattleBox, &sBattleGlyph, BATTLE_STRING(6));
	SET_TAB_BOX(mRuleHead[0]);
	mRuleHead[1] = createSimpleTextOnTask(1, -1, &sBattleHead1At, &sBattleBox, &sBattleGlyph, BATTLE_STRING(7));
	SET_TAB_BOX(mRuleHead[1]);
	mRuleHead[2] = createSimpleTextOnTask(1, -1, &sBattleHead2At, &sBattleBox, &sBattleGlyph, BATTLE_STRING(10));
	SET_TAB_BOX(mRuleHead[2]);
	mRuleKind = createSimpleTextOnTask(1, -1, &sBattleKindAt, &sBattleBox, &sBattleGlyph, BATTLE_STRING(11));
	SET_TAB_BOX(mRuleKind);
	mRuleAmount = createSimpleTextOnTask(1, -1, &sBattleAmountAt, &sBattleBox, &sBattleGlyph, BATTLE_STRING(14));
	SET_TAB_BOX(mRuleAmount);
	mRuleHelp = createSimpleTextOnTask(1, -1, &sBattleHelpAt, &sBattleHelpBox, &sBattleGlyph, BATTLE_STRING(23));
	mYes      = createSimpleTextOnTask(1, -1, &sBattleYesAt, &sBattleBox, &sBattleGlyph, BATTLE_STRING(4));
	mNo       = createSimpleTextOnTask(1, -1, &sBattleNoAt, &sBattleBox, &sBattleGlyph, BATTLE_STRING(5));
}

// 0x80195908
void TModeBattleSetup::hideAll()
{
	hideTitle();
	hideTabs();
	hideRuleHeads();
	hideRuleKind();
	hideRuleAmount();
	hideRuleHelp();
	hideYesNo();
}

// 0x80195964
void TModeBattleSetup::showTitle(int page)
{
	if (mTitle != 0) {
		mTitle->resetPage(BATTLE_STRING(page));
		mTitle->mFlags &= ~0x10;
	}
}

// 0x801959B8
void TModeBattleSetup::hideTitle()
{
	hideObj(mTitle);
}

// 0x801959D4
void TModeBattleSetup::showTabs()
{
	showObj(mTab[0]);
	showObj(mTab[1]);
}

// 0x80195A08
void TModeBattleSetup::hideTabs()
{
	hideObj(mTab[0]);
	hideObj(mTab[1]);
}

// 0x80195A3C
void TModeBattleSetup::showRuleHeads(int page)
{
	showObj(mRuleHead[0]);
	if (mRuleHead[1] != 0) {
		mRuleHead[1]->resetPage(BATTLE_STRING(page));
		mRuleHead[1]->mFlags &= ~0x10;
	}
	showObj(mRuleHead[2]);
}

// 0x80195AC0
void TModeBattleSetup::hideRuleHeads()
{
	hideObj(mRuleHead[0]);
	hideObj(mRuleHead[1]);
	hideObj(mRuleHead[2]);
}

// 0x80195B0C
// The rule's name and the heading of its amount
void TModeBattleSetup::showRuleKind()
{
	switch (mRule) {
	case 0:
		if (mRuleKind != 0) {
			mRuleKind->resetPage(BATTLE_STRING(12));
		}
		showRuleHeads(8);
		break;
	case 1:
		if (mRuleKind != 0) {
			mRuleKind->resetPage(BATTLE_STRING(11));
		}
		showRuleHeads(7);
		break;
	case 2:
		if (mRuleKind != 0) {
			mRuleKind->resetPage(BATTLE_STRING(13));
		}
		showRuleHeads(9);
		break;
	}
	showObj(mRuleKind);
}

// 0x80195BE8
void TModeBattleSetup::hideRuleKind()
{
	hideObj(mRuleKind);
}

// 0x80195C04
// Three strings a rule from 14
void TModeBattleSetup::showRuleAmount()
{
	switch (mRule) {
	case 0:
		switch (mAmount) {
		case 0:
			if (mRuleAmount != 0) {
				mRuleAmount->resetPage(BATTLE_STRING(17));
			}
			break;
		case 1:
			if (mRuleAmount != 0) {
				mRuleAmount->resetPage(BATTLE_STRING(18));
			}
			break;
		case 2:
			if (mRuleAmount != 0) {
				mRuleAmount->resetPage(BATTLE_STRING(19));
			}
			break;
		}
		break;
	case 1:
		switch (mAmount) {
		case 0:
			if (mRuleAmount != 0) {
				mRuleAmount->resetPage(BATTLE_STRING(14));
			}
			break;
		case 1:
			if (mRuleAmount != 0) {
				mRuleAmount->resetPage(BATTLE_STRING(15));
			}
			break;
		case 2:
			if (mRuleAmount != 0) {
				mRuleAmount->resetPage(BATTLE_STRING(16));
			}
			break;
		}
		break;
	case 2:
		switch (mAmount) {
		case 0:
			if (mRuleAmount != 0) {
				mRuleAmount->resetPage(BATTLE_STRING(20));
			}
			break;
		case 1:
			if (mRuleAmount != 0) {
				mRuleAmount->resetPage(BATTLE_STRING(21));
			}
			break;
		case 2:
			if (mRuleAmount != 0) {
				mRuleAmount->resetPage(BATTLE_STRING(22));
			}
			break;
		}
		break;
	}
	showObj(mRuleAmount);
}

// 0x80195DDC
void TModeBattleSetup::hideRuleAmount()
{
	hideObj(mRuleAmount);
}

// 0x80195DF8
// The help line for the row under the cursor
void TModeBattleSetup::showRuleHelp()
{
	switch (mRule) {
	case 0:
		if (mRow == 0) {
			if (mRuleHelp != 0) {
				mRuleHelp->resetPage(BATTLE_STRING(24));
			}
		} else if (mRow == 1) {
			if (mRuleHelp != 0) {
				mRuleHelp->resetPage(BATTLE_STRING(26));
			}
		} else if (mRuleHelp != 0) {
			mRuleHelp->resetPage(BATTLE_STRING(29));
		}
		break;
	case 1:
		if (mRow == 0) {
			if (mRuleHelp != 0) {
				mRuleHelp->resetPage(BATTLE_STRING(23));
			}
		} else if (mRow == 1) {
			if (mRuleHelp != 0) {
				mRuleHelp->resetPage(BATTLE_STRING(27));
			}
		} else if (mRuleHelp != 0) {
			mRuleHelp->resetPage(BATTLE_STRING(29));
		}
		break;
	case 2:
		if (mRow == 0) {
			if (mRuleHelp != 0) {
				mRuleHelp->resetPage(BATTLE_STRING(25));
			}
		} else if (mRow == 1) {
			if (mRuleHelp != 0) {
				mRuleHelp->resetPage(BATTLE_STRING(28));
			}
		} else if (mRuleHelp != 0) {
			mRuleHelp->resetPage(BATTLE_STRING(29));
		}
		break;
	}
	showObj(mRuleHelp);
}

// 0x80195F94
void TModeBattleSetup::hideRuleHelp()
{
	hideObj(mRuleHelp);
}

// 0x80195FB0
static inline Vec3f tabBox()
{
	Vec3f v;
	v.x = 0.0f;
	v.y = 24.0f;
	v.z = 0.0f;
	return v;
}

void TModeBattleSetup::showYesNo()
{
	Vec3f box = tabBox();
	if (mYes != 0) {
		mYes->mFlags &= ~0x10;
		mYes->mBox = box;
		mYes->_A8 = 1;
	}
	if (mNo != 0) {
		mNo->mFlags &= ~0x10;
		mNo->mBox = box;
		mNo->_A8 = 1;
	}
}

// 0x80196068
void TModeBattleSetup::hideYesNo()
{
	hideObj(mYes);
	hideObj(mNo);
}

// 0x8019609C
// Stage slots 0 and 1 come with the first world then one a world
void TModeBattleSetup::openStages(u8 worlds)
{
	for (int i = 0; i < 8; i++) {
		mStageOpen[i] = 0;
	}
	if (worlds & 1) {
		mStageOpen[0] = 1;
		mStageOpen[1] = 1;
	}
	if (worlds & 2) {
		mStageOpen[2] = 1;
	}
	if (worlds & 4) {
		mStageOpen[3] = 1;
	}
	if (worlds & 8) {
		mStageOpen[4] = 1;
	}
	if (worlds & 0x10) {
		mStageOpen[5] = 1;
	}
	if (worlds & 0x20) {
		mStageOpen[6] = 1;
	}
	if (worlds & 0x40) {
		mStageOpen[7] = 1;
	}
}

// 0x80196138
void TModeBattleSetup::showStageIcons()
{
	for (int i = 0; i < 8; i++) {
		if (mStageOpen[i] != 0) {
			aeBattleSelectOpenStage(i);
		}
	}
}

inline FxVec::FxVec() { }
