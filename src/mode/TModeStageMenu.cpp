#include "ui/TAeObjStageSelect.h"
#include "ui/TextVec.h"
#include "mode/TModeStageMenu.h"
#include "camera/TCamera.h"
#include "io/LangFile.h"
#include "io/ResFile.h"
#include "io/Resource.h"
#include "ui/TSimpleText.h"
#include "stage/TStageSelectModel.h"
#include "core/TMainTask.h"
#include "ui/MenuModelArc.h"
#include "egg/ObjEgg.h"
#include "core/Pad.h"
#include "Ninja/ninja.h"
#include "ui/TAeObjMsgWindow.h"
#include "core/JoyCarry.h"
#include "engine/ShadowTex.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "camera/TCameraCtrl.h"
#include "engine/Viewport.h"
#include "engine/DrawTask.h"
#include "sound/SoundCtrl.h"
#include "engine/FxSystem.h"
#include "engine/TFogCtrl.h"
#include "ui/Font.h"
#include "egg/EggCreature.h"
#include "io/SaveData.h"
#include "stage/TStageScore.h"
#include "ui/TFade.h"
#include "engine/TPolygonAnim.h"
#include "core/Mem.h"
#include "object/TObjLauncher.h"
#include "core/MathUtil.h"
#include "stage/Stage.h"

extern "C" {

// The 2D overlay

}

// Rodata tables of this unit 0x8024A6A0
extern "C" {
const int sStageMenuWorldNodes[9] = { 7, 4, 3, 1, 2, 5, 6, 8, 9 };
const int sStageMenuModelNodes[10] = { 9, 3, 4, 6, 7, 5, 8, 2, 1, 0 };
const int sStageMenuBurstNodes[9] = { 1, 7, 6, 4, 3, 5, 2, 8, 9 };
const int sStageMenuLinks[9][8] = {
	{ 8, 1, 1, 4, 5, 5, -1, 8 },
	{ 8, -1, 2, 2, 4, 0, 0, 8 },
	{ 1, -1, 6, 6, 3, 4, 1, 1 },
	{ 2, 6, 6, -1, 7, 7, 5, 4 },
	{ 1, 2, 6, 3, 3, 5, 0, 0 },
	{ 0, 4, 3, 7, 7, -1, -1, 0 },
	{ 2, -1, -1, -1, 3, 3, 4, 2 },
	{ 3, 3, -1, -1, -1, -1, 5, 5 },
	{ -1, -1, -1, 1, 1, 0, 0, -1 }
};
// Two more key tables laid out like sStageMenuStageIds
const s16 sStageMenuStageKeysA[7][8] = {
	{ 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x10 },
	{ 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18 },
	{ 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20 },
	{ 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28 },
	{ 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30 },
	{ 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8 },
	{ 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38 }
};
const s16 sStageMenuStageKeysB[7][8] = {
	{ 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B },
	{ 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53 },
	{ 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B },
	{ 0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63 },
	{ 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B },
	{ 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43 },
	{ 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73 }
};
const s16 sStageMenuStageIds[7][8] = {
	{ 0x154, 0x159, 0x15E, 0x163, 0x168, 0x16D, 0x172, 0x177 },
	{ 0x17C, 0x181, 0x186, 0x18B, 0x190, 0x195, 0x19A, 0x19F },
	{ 0x1A4, 0x1A9, 0x1AE, 0x1B3, 0x1B8, 0x1BD, 0x1C2, 0x1C7 },
	{ 0x1CC, 0x1D1, 0x1D6, 0x1DB, 0x1E0, 0x1E5, 0x1EA, 0x1EF },
	{ 0x1F4, 0x1F9, 0x1FE, 0x203, 0x208, 0x20D, 0x212, 0x217 },
	{ 0x12C, 0x131, 0x136, 0x13B, 0x140, 0x145, 0x14A, 0x14F },
	{ 0x21C, 0x221, 0x226, 0x22B, 0x230, 0x235, 0x23A, 0x23F }
};
const s16 sStageMenuGbaFlags[5] = { 0x7B, 0x7D, 0x7F, 0x7E, 0x7C };
const int sStageMenuWorldBase[7] = { 8, 0x10, 0x18, 0x20, 0x28, 0, 0x30 };
// The Japanese list is stripped from the DOL
const char* sStageMenuGbaFilesJP[5] = { "Chu2ChallengeJP.bin", "NightsUS.gba", "BillyAGB.bin", "BillyAGB.bin", "puyojp.gba" };
const char* sStageMenuGbaFiles[5] = { "Chu2ChallengeUS.bin", "NightsUS.gba", "BillyAGB.bin", "BillyAGB.bin", "puyous.gba" };
}

// mes_stg_select.bin fixed up like mes_main_menu.bin
BinFile* gStageMesRes;     // 0x802C5870
int gStageMesCount;     // 0x802C5874
void** gStageMesStrings; // 0x802C5878
int gStageNodeIndex;    // 0x802C587C counts nodes while walking the map
int gStageMoveDelay;    // 0x802C5880
TArc* gStagePadArc;     // 0x802C5884 pa_menu.pad
u8* gStagePolyAnim;     // 0x802C5888

struct StageMesString {
	u8 _00[8];
	char mText[1]; // _08
};

inline Vec3f vec3f(f32 x, f32 y, f32 z)
{
	Vec3f v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

// Built by the static initializer at 0x80178E80
static TextVec sTitleGlyph    = textVec(0.91f, 0.91f, 0.91f);
static TextVec sTitleBox      = textVec(394.0f, 44.0f, 0.0f);
static TextVec sTitleAt       = textVec(44.0f, 41.0f, -2.0f);
static TextVec sInfoGlyph     = textVec(0.81f, 0.81f, 0.81f);
static TextVec sInfoBox       = textVec(240.0f, 60.0f, 0.0f);
static TextVec sInfoAt        = textVec(465.0f, 186.0f, -2.0f);
static TextVec sHeadGlyph     = textVec(0.91f, 0.91f, 0.91f);
static TextVec sHeadBox       = textVec(132.0f, 22.0f, 0.0f);
static TextVec sHeadAt        = textVec(465.0f, 132.0f, -2.0f);
static TextVec sHelpGlyph     = textVec(0.91f, 0.91f, 0.91f);
static TextVec sHelpBox       = textVec(110.0f, 22.0f, 0.0f);
static TextVec sHelpAt        = textVec(171.0f, 316.0f, -2.0f);
static TextVec sGbaInfoGlyph  = textVec(0.81f, 0.81f, 0.81f);
static TextVec sGbaInfoBox    = textVec(240.0f, 60.0f, 0.0f);
static TextVec sGbaInfoAt     = textVec(464.0f, 274.0f, -2.0f);
static TextVec sHelpLowAt     = textVec(171.0f, 339.0f, -2.0f);
static TextVec sTabAt0 = textVec(565.0f, 38.0f, -2.0f);
static TextVec sTabAt1 = textVec(565.0f, 75.0f, -2.0f);
static TextVec sYesNoAt0 = textVec(512.0f, 320.0f, -2.0f);
static TextVec sYesNoAt1 = textVec(512.0f, 373.0f, -2.0f);
static TextVec sChoice3At0 = textVec(512.0f, 305.0f, -2.0f);
static TextVec sChoice3At1 = textVec(512.0f, 345.0f, -2.0f);
static TextVec sChoice3At2 = textVec(512.0f, 385.0f, -2.0f);
static TextVec sMarkerFrom    = textVec(-20.0f, 87.5f, 115.0f);
static TextVec sMarkerTo      = textVec(0.5f, 87.5f, 100.0f);
static BinFile* sGbaRes[5];
static HitVec sWorldPos[9];

inline void freeText(TSimpleText*& text)
{
	if (text != 0) {
		text->mFlags |= 1;
		text = 0;
	}
}

static inline void hideText(TSimpleText* text)
{
	if (text != 0) {
		text->mFlags |= 0x10;
	}
}

#define STAGE_STRING(i) (gStageMesStrings[i])

// 0x80175114
TModeStageMenu* TModeStageMenu::create(TModeAllocator* allocator)
{
	return new (allocator) TModeStageMenu;
}

// 0x8017514C
void TModeStageMenu::loadFiles()
{
	loadPrd("k_stage_select.prd");
}

// 0x80175174
int TModeStageMenu::init()
{
	f32 mtx[16];
	Vec3f pos;
	int angle[3];
	Vec3f camPos;
	Vec3f camDir;
	GXSetCopyClear(sModeStageMenuClearColor, 0xFFFFFF);
	camCtrlInit();
	viewportInit();
	initDrawTasks();
	loadMsgWindow();
	loadAeStageSelect();
	loadLandModelArc();
	loadStageSelectText();
	fxSystemInit();
	fogLoad();
	fogOff();

	loadSizedResource(getLangFileName("mes_stg_select.bin"), &gStageMesRes);
	int* header      = (int*)gStageMesRes->mData;
	u8* strings      = (u8*)header + header[0];
	gStageMesStrings = (void**)(header + 2);
	gStageMesCount   = (u32)header[1] >> 2;
	for (int i = 0; i < gStageMesCount; i++) {
		gStageMesStrings[i] = ((StageMesString*)(strings + (int)gStageMesStrings[i]))->mText;
	}

	loadArc("pa_menu.pad", &gStagePadArc);
	gStagePolyAnim = (u8*)findArcExport(gStagePadArc, "polyanim");
	fxBankBind(6, gStagePolyAnim);
	gjLoadLight(0);
	for (int i = 0; i < 5; i++) {
		loadSizedResource(sStageMenuGbaFiles[i], &sGbaRes[i]);
	}
	objClearLayers(&gMainTask);
	createCameraCtrl();

	camDir.set(0.0f, 183.08218f, 330.56439f);
	camPos.set(0.0f, 24.012760f, -1.5446500f);
	mLight       = createTCamFix(0, 2, &camPos, &camDir);
	if (mLight != 0) {
		((TCamera*)mLight)->setFov(0xEEE);
	}
	camSetDrawView(0);
	setViewport(0);
	fontCreateTask();
	createTPolygonAnim();
	createAeStageSelect();
	aeStageSelectSetLives(_29C);
	aeStageSelectSetEmblems(_29D);

	createTitle(5);
	createYesNo();
	createTabs();
	createInfo();
	createHelp(2);
	createCancel();

	MenuModelSet* set = gMenuModelSet;
	NJS_OBJECT* root  = (NJS_OBJECT*)set->mModels[12];
	gStageNodeIndex   = 0;
	findWorldPos(root);

	mtxIdentity(mtx);
	angle[0]  = 0;
	pos.set(0.0f, 0.0f, 0.0f);
	angle[1]  = 0;
	angle[2]  = 0;

	mModels[10] = createTStageSelectModel(gTL_05, &pos, angle, 10, 0x21);
	if (mModels[10] != 0) {
		mModels[10]->mPlaying = 0;
	}
	for (int i = 0; i < 10; i++) {
		if ((u32)(i - 4) <= 1 || i == 6) {
			mModels[i] = createTStageSelectModel(gTL_05, &pos, angle, i, 0x22);
		} else if (i == 9) {
			mModels[i] = createTStageSelectModel(gTL_05, &pos, angle, i, 0x21);
			if (mModels[i] != 0) {
				mModels[i]->mRate = 0.5f;
			}
		} else {
			mModels[i] = createTStageSelectModel(gTL_05, &pos, angle, i, 0x21);
		}
		if (i == 1 && mModels[i] != 0) {
			mModels[i]->spawnIndex1();
		}
		if (i == 2 && mModels[i] != 0) {
			mModels[i]->spawnIndex2();
		}
		if (i == 4 && mModels[i] != 0) {
			mModels[i]->spawnIndex4();
		}
		if (i == 5 && mModels[i] != 0) {
			mModels[i]->spawnIndex5();
		}
		if (i == 8 && mModels[i] != 0) {
			mModels[i]->mPlaying = 0;
			mModels[i]->mFlags |= 0x10;
		}
		if (i == 6 && mModels[i] != 0) {
			mModels[i]->mFlags |= 0x10;
		}
	}
	mModels[11] = createTStageSelectModel(gTL_05, &pos, angle, 11, 0x22);
	if (mModels[11] != 0) {
		mModels[11]->mPlaying = 0;
	}
	root            = (NJS_OBJECT*)set->mModels[9];
	gStageNodeIndex = 0;
	placeModels(root);

	mMarkerPos = sMarkerFrom;
	mMarker    = createTStageSelectModel(gTL_05, &mMarkerPos, angle, 0, 0x3E);
	if (mMarker != 0) {
		mMarker->mAmbient.set(0.7f, 0.7f, 0.7f);
		mMarker->mLightColor.set(1.0f, 1.0f, 1.0f);
		((Vec3f*)&mMarker->mLightX)->set(1.0f, -1.0f, -1.0f);
	}
	unlockGba();
	bgmPlay(1);
	enterMap();
	return 1;
}

// 0x80175644
int TModeStageMenu::shutdown()
{
	freeText(mTitle);
	freeText(mInfoHead);
	freeText(mInfo);
	freeText(mBox);
	freeText(mTab[0]);
	freeText(mTab[1]);
	freeText(mYes);
	freeText(mNo);
	freeText(mCancel);
	freeText(mHelp);
	if (mLight != 0) {
		((TObject*)mLight)->mFlags |= 1;
		mLight = 0;
	}
	if (mWindow != 0) {
		((TObject*)mWindow)->mFlags |= 1;
		mWindow = 0;
	}
	objClearLayers(&gMainTask);
	fxRemoveBank(6);
	fxBankUnbind(6, gStagePolyAnim);
	gStagePolyAnim = 0;
	fogUnload();
	fxSystemShutdown();
	termDrawTasks();
	viewportTerm();
	camCtrlTerm();
	freeMsgWindow();
	freeAeStageSelect();
	freeLandModelArc();
	freeStageSelectText();
	bgmStop();
	for (int i = 0; i < 5; i++) {
		releaseResource(sGbaRes[i]);
	}
	releaseResource(gStagePadArc);
	gStagePadArc = 0;
	if (gStageMesRes != 0) {
		releaseResource(gStageMesRes);
		gStageMesRes = 0;
	}
	return 1;
}

// 0x8017588C
TModeStageMenu::TModeStageMenu()
{
	int i;
	mTimer    = 0;
	mMoveT    = 0.0f;
	mLight    = 0;
	mState    = 0;
	mSub      = 0;
	mWorld    = 5;
	mStage    = 0;
	_1C       = 0;
	mGbaBack  = 1;
	mGba      = 0;
	mTitle    = 0;
	mInfoHead = 0;
	mInfo     = 0;
	mBox      = 0;
	mTab[0]   = 0;
	mTab[1]   = 0;
	mYes      = 0;
	mNo       = 0;
	mCancel   = 0;
	mHelp     = 0;
	mBoxKind  = 0;
	mChoice   = 0;
	mBoxTimer = 0;
	mWindow   = 0;
	mMoveFrom.x = 0.0f;
	mMoveFrom.y = 0.0f;
	mMoveFrom.z = 0.0f;
	for (i = 0; i < 12; i++) {
		mModels[i] = 0;
	}
	mMarker      = 0;
	mMarkerPos.x = 0.0f;
	mMarkerPos.y = 0.0f;
	mMarkerPos.z = 0.0f;
	mFlags       = 0;
	mNext        = 1;
	for (i = 0; i < 7; i++) {
		mWorldOpen[i] = 0;
	}
	_C0     = 1;
	mHasGba = 0;
	for (i = 0; i < 7; i++) {
		for (int j = 0; j < 8; j++) {
			mStageClear[i][j] = 0;
			mStageOpen[i][j]  = 0;
		}
	}
	for (i = 0; i < 5; i++) {
		mGbaOpen[i] = 0;
	}
	_29C = 5;
	_29D = 0;
}

// 0x80175A04
TModeStageMenu::~TModeStageMenu()
{
}

// 0x80175A2C
void TModeStageMenu::update()
{
	gMainTask.draw();
	gjBeforeRender();
	switch (mState) {
	case 0:
		updateMap();
		break;
	case 1:
		updateWorlds();
		break;
	case 2:
		updateStagesIn();
		break;
	case 3:
		updateStagesWait();
		break;
	case 4:
		updateStages();
		break;
	case 5:
		updateGbaIn();
		break;
	case 6:
		updateGbaIn2();
		break;
	case 7:
		updateGba();
		break;
	case 8:
		updateGbaSend();
		break;
	case 9:
		updateGbaBack();
		break;
	case 10:
		updateOptionsIn();
		break;
	case 11:
		updateOptions();
		break;
	case 12:
		updateStart();
		break;
	case 13:
		updateLeave();
		break;
	case 15:
		updateQuit();
		break;
	}
	mModels[10]->mAngle[1]++;
	if (mMarker != 0) {
		mMarker->mAngle[1] += 0x100;
	}
	moveCursor();
	gjSetRenderMode(3);
	GXSetCullMode(GX_CULL_NONE);
	setViewport(0);
	camSetDrawView(0);
	gMainTask.vfn_10();
	gjDisableFog();
	runDrawTasks(0x21);
	sortDrawTasks(0x22);
	runDrawTasks(0x22);
	runDrawTasks(0x3C);
	runDrawTasks(0x3E);
	runDrawTasks(0x3F);
	memDebugDrawBegin();
	memShowStats();
	memDebugDrawEnd();
	frameTickerGet()->update();
	gjPrintFlush();
	gjDoneRender();
	endFrameSwap();
}

// 0x80175BEC
// Opens the GBA page once any game's save flag is set
void TModeStageMenu::unlockGba()
{
	for (s16 i = 0; i < 5; i++) {
		if (saveGetKeyBit(sStageMenuGbaFlags[i]) != 0) {
			mGbaOpen[i] = 1;
			mHasGba     = 1;
			if (mModels[8] != 0) {
				mModels[8]->mFlags &= ~0x10;
			}
		}
	}
}

// 0x80175C78
int TModeStageMenu::pressedA()
{
	return padGet(0)->mPressed & 0x100;
}

// 0x80175CA4
int TModeStageMenu::pressedB()
{
	return padGet(0)->mPressed & 0x200;
}

// 0x80175CD0
// A stick direction counts as its d-pad button too
int TModeStageMenu::repeated(u32 mask)
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
	return mask & padGet(0)->mRepeat;
}

// Copies a look onto a model
#define applyLook(model, look)                                                         \
	(model)->mAmbient.set((look).mAmbient.x, (look).mAmbient.y, (look).mAmbient.z);   \
	(model)->mLightColor.set((look).mColor.x, (look).mColor.y, (look).mColor.z);     \
	((Vec3f*)&(model)->mLightX)->set((look).mDir.x, (look).mDir.y, (look).mDir.z)

// 0x80175D74
void TModeStageMenu::enterMap()
{
	hideAllText();
	aeStageSelectSetWorld(mWorld);
	aeStageSelectOpen();
	for (int i = 0; i < 7; i++) {
		if (mStageOpen[i][0] == 0) {
			mWorldOpen[i] = 0;
		}
	}

	s8 i;
	int look = 0;
	NJS_OBJECT* bursts;
	if (mStageClear[6][0] != 0) {
		if (mModels[10] != 0) {
			mModels[10]->setMaterial(4);
		}
		look = 4;
	} else if (mStageClear[4][2] != 0) {
		if (mModels[10] != 0) {
			mModels[10]->setMaterial(3);
		}
		look = 3;
	} else if (mStageClear[2][1] != 0) {
		if (mModels[10] != 0) {
			mModels[10]->setMaterial(2);
		}
		look = 2;
	} else if (mStageClear[0][1] != 0) {
		if (mModels[10] != 0) {
			mModels[10]->setMaterial(1);
		}
		look = 1;
	}

	bursts = (NJS_OBJECT*)gMenuModelSet->mModels[13];
	for (i = 0; i < 7; i++) {
		if (mStageOpen[i][0] == 0) {
			applyLook(mModels[i], sStageMenuLooks[look]);
		}
		if (mStageOpen[i][1] != 0 && look != 4) {
			f32 mtx[12];
			gStageNodeIndex = 0;
			findBurstMtx(mtx, bursts, i);
			mModels[i]->attachBurst(mtx);
		}
		if (look > 2) {
			applyLook(mModels[i], sStageMenuLooks[look]);
		}
	}
	if (mStageClear[4][1] != 0) {
		mModels[6]->spawnIndex6();
	}
	if (mStageClear[4][2] != 0) {
		mModels[6]->mFlags &= ~0x10;
	}
	applyLook(mModels[9], sStageMenuLooks[look]);
	if (mMarker != 0) {
		mMarker->mFlags |= 0x10;
	}
	mMoveT              = 1.0f;
	mModels[11]->mPos   = sWorldPos[mWorld];
	mTimer              = 0x3C;
	mState              = 0;
	mSub                = 0;
}

// 0x801760EC
void TModeStageMenu::enterWorlds()
{
	hideAllText();
	showTitle(5);
	showTabs();
	mTimer = 0x168;
	mState = 1;
	mSub   = 2;
}

// 0x80176144
void TModeStageMenu::enterStages()
{
	aeStageSelectOpen();
	aeStageSelectShowWorld(mWorld);
	aeStageSelectSetWorld(mWorld);
	mMarker->setModel(mWorld);
	mStage = 0;
	aeStageSelectClearStages();
	for (s8 i = 0; i < 8; i++) {
		if (mStageClear[mWorld][i] != 0) {
			aeStageSelectSetStageIcon(i, 1);
		} else if (mStageOpen[mWorld][i] != 0) {
			aeStageSelectSetStageIcon(i, 0);
			if (i < 5) {
				mStage = i;
			}
		}
	}
	if (mStageClear[mWorld][4] != 0) {
		for (s8 i = 5; i < 8; i++) {
			if (mStageOpen[mWorld][i] != 0 && mStageClear[mWorld][i] == 0) {
				mStage = i;
			}
		}
	}
	showStageInfo();
	aeStageSelectSetStage(mStage);
	mTimer = 0x18;
	mState = 3;
}

static inline void showMarker(TModeStageMenu* menu, int playing)
{
	if (menu->mMarker != 0) {
		menu->mMarker->mFlags &= ~0x10;
		menu->mMarker->setModel(menu->mWorld);
		menu->mMarker->mScale   = sStageMenuMarkerRates[menu->mWorld];
		menu->mMarker->mPlaying = playing;
	}
}

// 0x801762A8
void TModeStageMenu::enterStageWait()
{
	showTitle(6);
	showTabs();
	showInfo();
	showMarker(this, 1);
	mState = 4;
}

// 0x8017634C
void TModeStageMenu::enterGba()
{
	aeStageSelectOpen();
	aeStageSelectShowGba();
	aeStageSelectLockGbaRows();
	for (s8 i = 0; i < 5; i++) {
		if (mGbaOpen[i] != 0) {
			aeStageSelectOpenGbaRow(i);
		}
	}
	s8 i;
	for (i = 0; i < 5; i++) {
		if (mGbaOpen[i] != 0) {
			mGba     = i;
			mGbaBack = 0;
			aeStageSelectSetGba(mGba);
			break;
		}
	}
	if (i == 5) {
		mGba     = -1;
		mGbaBack = 1;
		aeStageSelectSetGba(i);
	}
	mTimer = 0x18;
	mState = 5;
}

// 0x80176434
void TModeStageMenu::enterGbaWait()
{
	showTitle(7);
	showTabs();
	showHelp(3);
	showGbaInfo();
	showMarker(this, 0);
	mState = 7;
}

// 0x801764E4
void TModeStageMenu::enterGbaSend()
{
	hideAllText();
	showTabs();
	openMessage(STAGE_STRING(10));
	mState   = 8;
	mGbaStep = 0;
}

// 0x80176538
void TModeStageMenu::enterGbaBack()
{
	hideAllText();
	showTabs();
	openMessage(STAGE_STRING(9));
	mState = 9;
}

// 0x80176584
void TModeStageMenu::enterOptions()
{
	aeStageSelectOpen();
	aeStageSelectShowOptions();
	mTimer = 0x18;
	mState = 10;
}

// 0x801765C4
void TModeStageMenu::enterOptionWait()
{
	showTitle(8);
	showHelp(2);
	showMarker(this, 1);
	mState = 11;
}

// 0x80176664
void TModeStageMenu::enterStart()
{
	hideAllText();
	aeStageSelectCloseAll();
	mTimer = 0x3C;
	mState = 12;
}

// 0x801766A4
void TModeStageMenu::enterLeave()
{
	aeStageSelectClose();
	hideAllText();
	mTimer = 0x3C;
	if (mNext == 7) {
		fadeToBlack();
	} else {
		fadeModel01();
	}
	mState = 13;
}

// 0x80176700
void TModeStageMenu::enterQuit()
{
	aeStageSelectOpen();
	hideAllText();
	showTabs();
	openYesNo(getStageSelectWarning(0x29));
	mState = 15;
}

// 0x80176758
void TModeStageMenu::updateMap()
{
	if (mTimer == 0) {
		if (mWindow != 0) {
			if (mWindow->mState == TAeObjMsgWindow::MSGWIN_CLOSED && mWindow != 0) {
				((TObject*)mWindow)->mFlags |= 1;
				mWindow = 0;
			}
		} else {
			enterWorlds();
		}
	} else {
		mTimer--;
	}
}

static inline int stickDir(int angle)
{
	int dir = 0;
	if (angle <= 0x1000 || angle >= 0xF000) {
		dir = 0;
	} else if (angle > 0x1000 && angle < 0x3000) {
		dir = 1;
	} else if (angle >= 0x3000 && angle <= 0x5000) {
		dir = 2;
	} else if (angle > 0x5000 && angle < 0x7000) {
		dir = 3;
	} else if (angle >= 0x7000 && angle <= 0x9000) {
		dir = 4;
	} else if (angle > 0x9000 && angle < 0xB000) {
		dir = 5;
	} else if (angle >= 0xB000 && angle <= 0xD000) {
		dir = 6;
	} else if (angle > 0xD000 && angle < 0xF000) {
		dir = 7;
	}
	return dir;
}

// 0x801767C8
void TModeStageMenu::updateWorlds()
{
	switch (mSub) {
	case 0:
		if (aeStageSelectIsDone() != 0) {
			mSub = 2;
			showTitle(5);
		}
		break;
	case 1:
		if (mTimer == 0) {
			aeStageSelectOpen();
			mSub = 0;
		} else {
			mTimer--;
		}
		break;
	case 2:
		break;
	}

	if (padGet(0)->mPressed & 0x100) {
		sePlay(0x1C);
		if (mWorld == 7) {
			enterOptions();
		} else if (mWorld == 8) {
			enterGba();
		} else {
			enterStages();
		}
	} else if (padGet(0)->mPressed & 0x200) {
		sePlay(0x20);
		enterQuit();
	} else if (gStageMoveDelay == 0) {
		int dir    = -1;
		PadState* pad = padGet(0);
		if (pad->mLength > 0.95f) {
			if (pad->mAngle <= 0x1000 || pad->mAngle >= 0xF000) {
				dir = 0;
			} else if (pad->mAngle > 0x1000 && pad->mAngle < 0x3000) {
				dir = 1;
			} else if (pad->mAngle >= 0x3000 && pad->mAngle <= 0x5000) {
				dir = 2;
			} else if (pad->mAngle > 0x5000 && pad->mAngle < 0x7000) {
				dir = 3;
			} else if (pad->mAngle >= 0x7000 && pad->mAngle <= 0x9000) {
				dir = 4;
			} else if (pad->mAngle > 0x9000 && pad->mAngle < 0xB000) {
				dir = 5;
			} else if (pad->mAngle >= 0xB000 && pad->mAngle <= 0xD000) {
				dir = 6;
			} else if (pad->mAngle > 0xD000 && pad->mAngle < 0xF000) {
				dir = 7;
			}
			int to = sStageMenuLinks[mWorld][dir];
			if (to != -1 && mWorldOpen[to] != 0) {
				sePlay(0x1B);
				mWorld = to;
				moveToWorld();
				mTimer = 0x168;
			}
		} else {
			dir = -1;
			if ((padGet(0)->mRepeat & 2) && (padGet(0)->mRepeat & 8)) {
				dir = 1;
			} else if ((padGet(0)->mRepeat & 2) && (padGet(0)->mRepeat & 4)) {
				dir = 7;
			} else if ((padGet(0)->mRepeat & 1) && (padGet(0)->mRepeat & 8)) {
				dir = 3;
			} else if ((padGet(0)->mRepeat & 1) && (padGet(0)->mRepeat & 4)) {
				dir = 5;
			} else if (padGet(0)->mRepeat & 2) {
				dir = 0;
			} else if (padGet(0)->mRepeat & 8) {
				dir = 2;
			} else if (padGet(0)->mRepeat & 1) {
				dir = 4;
			} else if (padGet(0)->mRepeat & 4) {
				dir = 6;
			}
			if (dir != -1) {
				int to = sStageMenuLinks[mWorld][dir];
				if (to != -1 && mWorldOpen[to] != 0) {
					sePlay(0x1B);
					mWorld = to;
					moveToWorld();
					mTimer = 0x168;
				}
			}
		}
	} else {
		gStageMoveDelay--;
	}
}

// 0x80176BF0
void TModeStageMenu::updateStagesIn()
{
	if (mTimer == 0) {
		enterStages();
	} else {
		mTimer--;
	}
}

// 0x80176C28
void TModeStageMenu::updateStagesWait()
{
	if (mTimer == 0) {
		enterStageWait();
	} else {
		mTimer--;
	}
}

// 0x80176C60
void TModeStageMenu::updateStages()
{
	if ((padGet(0)->mRepeat & 0x400000) || (padGet(0)->mRepeat & 1)) {
		int i;
		for (i = mStage - 1; i >= -1; i--) {
			if (i == -1) {
				for (int j = 7; j > mStage; j--) {
					if (mStageOpen[mWorld][j] != 0) {
						mStage = j;
						break;
					}
				}
				break;
			}
			if (mStageOpen[mWorld][i] != 0) {
				mStage = i;
				break;
			}
		}
		sePlay(0x1B);
		showStageInfo();
		showInfo();
	} else if ((padGet(0)->mRepeat & 0x800000) || (padGet(0)->mRepeat & 2)) {
		int i;
		for (i = mStage + 1; i <= 8; i++) {
			if (i == 8) {
				for (int j = 0; j < mStage; j++) {
					if (mStageOpen[mWorld][j] != 0) {
						mStage = j;
						break;
					}
				}
				break;
			}
			if (mStageOpen[mWorld][i] != 0) {
				mStage = i;
				break;
			}
		}
		sePlay(0x1B);
		showStageInfo();
		showInfo();
	}

	if (padGet(0)->mPressed & 0x100) {
		sePlay(0x1C);
		mNext = 0;
		enterStart();
	} else if (padGet(0)->mPressed & 0x200) {
		sePlay(0x20);
		aeStageSelectHideWorld();
		if (mMarker != 0) {
			mMarker->mFlags |= 0x10;
		}
		enterWorlds();
	}
}

// 0x80176ED0
void TModeStageMenu::updateGbaIn()
{
	if (mTimer == 0) {
		enterGbaWait();
	} else {
		mTimer--;
	}
}

// 0x80176F08
void TModeStageMenu::updateGbaIn2()
{
	if (mTimer == 0) {
		enterGbaWait();
	} else {
		mTimer--;
	}
}

// 0x80176F40
void TModeStageMenu::updateGba()
{
	if ((padGet(0)->mRepeat & 0x100000) || (padGet(0)->mRepeat & 8) || (padGet(0)->mRepeat & 0x200000)
	    || (padGet(0)->mRepeat & 4)) {
		if (mGba == -1) {
			mGbaBack = 1;
			aeStageSelectSetGba(5);
			hideGbaInfo();
		} else if (mGbaBack == 0) {
			mGbaBack = 1;
			aeStageSelectSetGba(5);
			hideGbaInfo();
		} else {
			mGbaBack = 0;
			aeStageSelectSetGba(mGba);
			showGbaInfo();
		}
	}

	if (mGbaBack == 0) {
		if ((padGet(0)->mRepeat & 0x400000) || (padGet(0)->mRepeat & 1)) {
			for (s8 i = mGba - 1; i >= -1; i--) {
				if (i == -1) {
					for (i = 4; i > mGba; i--) {
						if (mGbaOpen[i] != 0) {
							mGba = i;
							break;
						}
					}
					break;
				}
				if (mGbaOpen[i] != 0) {
					mGba = i;
					break;
				}
			}
			sePlay(0x1B);
			aeStageSelectSetGba(mGba);
			showGbaInfo();
		} else if ((padGet(0)->mRepeat & 0x800000) || (padGet(0)->mRepeat & 2)) {
			for (s8 i = mGba + 1; i <= 5; i++) {
				if (i == 5) {
					for (i = 0; i < mGba; i++) {
						if (mGbaOpen[i] != 0) {
							mGba = i;
							break;
						}
					}
					break;
				}
				if (mGbaOpen[i] != 0) {
					mGba = i;
					break;
				}
			}
			sePlay(0x1B);
			aeStageSelectSetGba(mGba);
			showGbaInfo();
		}
	}

	if (padGet(0)->mPressed & 0x100) {
		sePlay(0x1C);
		if (mGbaBack == 0) {
			enterGbaSend();
		} else {
			enterGbaBack();
		}
	} else if (padGet(0)->mPressed & 0x200) {
		sePlay(0x20);
		if (mMarker != 0) {
			mMarker->mFlags |= 0x10;
		}
		aeStageSelectHideGba();
		enterWorlds();
	}
}

// One GBA game's data the arc's mData + 0x2C is the program + 0x30 its size
// 0x80177244
// Steps 0 ask 1 asked 2 sending 3 failed 4 done
void TModeStageMenu::updateGbaSend()
{
	switch (mGbaStep) {
	case 0:
		if (runBox() != 0) {
			openChoice3(STAGE_STRING(14 + mGba));
			mGbaStep = 1;
		}
		break;
	case 1:
		if (runBox() != 0) {
			if (mChoice == 0) {
				BinFile* game = sGbaRes[mGba];
				u8* program   = (u8*)game->mData;
				u32 size      = game->mSize;
				if (mGba == 2) {
					program[200] = 1;
				} else if (mGba == 3) {
					program[200] = 0;
				}
				if (JoyCarry::get()->boot(program, size) == 0) {
					openMessage(STAGE_STRING(13));
					mGbaStep = 3;
				} else {
					openTimed(STAGE_STRING(11));
					mGbaStep = 2;
				}
			} else if (mChoice == 1) {
				closeBox();
				mTimer = 0x1E;
				mState = 6;
			} else if (mChoice == 2) {
				openMessage(STAGE_STRING(24 + mGba));
				mGbaStep = 0;
			}
		}
		break;
	case 2: {
		int status = JoyCarry::get()->getResult();
		if (status == 1) {
			break;
		}
		if (status == 0) {
			if (runBox() != 0) {
				openMessage(STAGE_STRING(12));
				mGbaStep = 4;
			}
		} else {
			openMessage(STAGE_STRING(13));
			mGbaStep = 3;
		}
		break;
	}
	case 3:
		if (runBox() != 0) {
			closeBox();
			mGbaStep = 4;
		}
		break;
	case 4:
		if (runBox() != 0) {
			closeBox();
			enterGbaWait();
		}
		break;
	}
}

// 0x8017747C
void TModeStageMenu::updateGbaBack()
{
	if (runBox() != 0) {
		closeBox();
		enterGbaWait();
	}
}

// 0x801774C0
void TModeStageMenu::updateOptionsIn()
{
	if (mTimer == 0) {
		enterOptionWait();
	} else {
		mTimer--;
	}
}

// 0x801774F8
void TModeStageMenu::updateOptions()
{
	if (pressedA()) {
		sePlay(0x1C);
		if (mMarker != 0) {
			mMarker->mFlags |= 0x10;
		}
		mNext = 7;
		enterStart();
	} else if (pressedB()) {
		sePlay(0x20);
		aeStageSelectHideWorld();
		if (mMarker != 0) {
			mMarker->mFlags |= 0x10;
		}
		enterWorlds();
	}
}

// The marker's and cursor's vector math results returned by value
static inline Vec3f menuSub(const Vec3f* a, const Vec3f* b)
{
	Vec3f r;
	gjSubVector(&r, a, b);
	return r;
}

static inline Vec3f menuScale(f32 s, const Vec3f* v)
{
	Vec3f r;
	gjScaleVector(s, &r, v);
	return r;
}

static inline Vec3f menuAddBy(const Vec3f* a, Vec3f b)
{
	Vec3f r;
	gjAddVector(&r, a, &b);
	return r;
}

// 0x8017759C
// The marker slides from sMarkerFrom to sMarkerTo over frames 45 to 25
static inline Vec3f menuScaleBy(f32 s, Vec3f v)
{
	Vec3f r;
	gjScaleVector(s, &r, &v);
	return r;
}

void TModeStageMenu::updateStart()
{
	if (mTimer == 0) {
		enterLeave();
		return;
	}
	mTimer--;
	int t = mTimer;
	if (t < 45 && t >= 25) {
		Vec3f from;
		Vec3f to;
		from = sMarkerFrom;
		to   = sMarkerTo;
		Vec3f pos = menuAddBy(&from, menuScaleBy((f32)(45 - t) / 20.0f, menuSub(&to, &from)));
		mMarkerPos = pos;
		if (mMarker != 0) {
			mMarker->mPos = mMarkerPos;
		}
	}
}

// 0x8017770C
void TModeStageMenu::updateLeave()
{
	if (mTimer == 0) {
		_08 = mNext;
		return;
	}
	mTimer--;
}

// 0x80177730
void TModeStageMenu::updateQuit()
{
	if (runBox() != 0) {
		if (choseYes()) {
			closeBox();
			mNext = 1;
			enterLeave();
		} else {
			closeBox();
			enterWorlds();
		}
	}
}

// 0x801777A0
void TModeStageMenu::moveToWorld()
{
	aeStageSelectSetWorld(mWorld);
	mMoveT          = 0.0f;
	mMoveFrom       = mModels[11]->mPos;
	gStageMoveDelay = 10;
	TStageSelectModel* marker = mMarker;
	marker->mIndex = mWorld;
	marker->mFrame = 0.0f;
	if (!(mFlags & 2)) {
		aeStageSelectClose();
		hideTitle();
		mSub = 1;
	}
}

// 0x80177830
void TModeStageMenu::moveCursor()
{
	if (mMoveT < 1.0f) {
		mMoveT += 0.1f;
	} else {
		mMoveT = 1.0f;
	}
	Vec3f from = mMoveFrom;
	Vec3f to   = sWorldPos[mWorld];
	Vec3f diff = menuSub(&to, &from);
	to = menuAddBy(&from, menuScale(mMoveT, &diff));
	mModels[11]->mPos = to;
	mModels[11]->mAngle[1] += 0x222;
}

// 0x8017797C
void TModeStageMenu::showStageInfo()
{
	aeStageSelectSetStage(mStage);
	s8 kind = 0;
	if (mStage == 5) {
		kind = 1;
	} else if (mStage == 6) {
		kind = 2;
	} else if (mStage == 7) {
		kind = 3;
	}
	aeStageSelectSetStageKind(kind);

	s16 eggs = 0;
	for (int i = 0; i < 5; i++) {
		if (saveGetKeyBit(i + sStageMenuStageIds[mWorld][mStage]) != 0) {
			eggs++;
		}
	}
	aeStageSelectSetEggs(eggs);
	aeStageSelectSetEggTotal(5);

	StageDefTable* table = getStageDefTable();
	int stage            = mStage + sStageMenuWorldBase[mWorld];
	int score            = saveGetPoints(stage);
	int time             = saveGetTime(stage);
	int total            = scoreWithTime(score, time, table->mEntries[stage].mTimeLimit * 1000);
	StageDef* defs       = table->mEntries;
	int rank = scoreRank(total, defs[stage].mRankScores[0], defs[stage].mRankScores[1], defs[stage].mRankScores[2],
	                      defs[stage].mRankScores[3]);
	if (mStageClear[mWorld][mStage] == 0) {
		rank = 5;
	}
	aeStageSelectSetRank(rank);
	aeStageSelectSetScore(score);
	s16 secs    = time / 1000;
	s16 minutes = secs / 60;
	secs %= 60;
	aeStageSelectSetMinutes(minutes);
	aeStageSelectSetSeconds(secs);
	aeStageSelectSetMsecs(time % 1000);
}

// 0x80177B60
// Walks the map's position nodes and keeps the ones that mark a world
void TModeStageMenu::findWorldPos(NJS_OBJECT* node)
{
	do {
		for (int i = 0; i < 9; i++) {
			if (gStageNodeIndex == sStageMenuWorldNodes[i]) {
				sWorldPos[i].set(node->pos[0], node->pos[1], node->pos[2]);
				break;
			}
		}
		gStageNodeIndex++;
		if (node->child != 0) {
			findWorldPos(node->child);
		}
		njPopMatrixEx();
		node = node->sibling;
	} while (node != 0);
}

// 0x80177C1C
// Puts each world model where its node is
struct StageAngle {
	int v[3];
};

void TModeStageMenu::placeModels(NJS_OBJECT* node)
{
	do {
		for (int i = 0; i < 10; i++) {
			if (gStageNodeIndex == sStageMenuModelNodes[i]) {
				StageAngle angle = *(StageAngle*)node->ang;
				Vec3f pos;
				pos.set(node->pos[0], node->pos[1], node->pos[2]);
				if (mModels[i] != 0) {
					mModels[i]->mPos = pos;
					*(StageAngle*)mModels[i]->mAngle = angle;
				}
				break;
			}
		}
		gStageNodeIndex++;
		if (node->child != 0) {
			placeModels(node->child);
		}
		njPopMatrixEx();
		node = node->sibling;
	} while (node != 0);
}

// 0x80177CF8
void TModeStageMenu::findBurstMtx(f32* mtx, NJS_OBJECT* node, s8 world)
{
	do {
		if (gStageNodeIndex == sStageMenuBurstNodes[world]) {
			Vec3f pos;
			pos.set(node->pos[0], node->pos[1], node->pos[2]);
			int angle[3];
			int y;
			int z;
			z = node->ang[2];
			y = node->ang[1];
			angle[0] = node->ang[0];
			angle[1] = y;
			angle[2] = z;
			mtxSetTransV(mtx, &pos);
			mtxRotateZYX(mtx, mtx, angle);
			return;
		}
		gStageNodeIndex++;
		if (node->child != 0) {
			findBurstMtx(mtx, node->child, world);
		}
		njPopMatrixEx();
		node = node->sibling;
	} while (node != 0);
}

// 0x80177DD0
void TModeStageMenu::hideAllText()
{
	hideTitle();
	hideInfo();
	hideYesNo();
	hideTabs();
	hideHelp();
	hideChoices3();
	hideGbaInfo();
}

// 0x80177E2C
void TModeStageMenu::createTitle(int msg)
{
	mTitle      = createSimpleTextOnTask(1, -1, &sTitleAt, &sTitleBox, &sTitleGlyph, STAGE_STRING(msg));
	mTitle->_A9 = 1;
}

// 0x80177E90
void TModeStageMenu::showTitle(int msg)
{
	if (mTitle != 0) {
		mTitle->resetPage(STAGE_STRING(msg));
		mTitle->mFlags &= ~0x10;
	}
}

// 0x80177EE4
void TModeStageMenu::hideTitle()
{
	hideText(mTitle);
}

// 0x80177F00
void TModeStageMenu::createInfo()
{
	Vec3f box = vec3f(0.0f, 24.0f, 0.0f);
	mInfoHead = createSimpleTextOnTask(1, -1, &sHeadAt, &sHeadBox, &sHeadGlyph, STAGE_STRING(29));
	mInfoHead->mBox = box;
	mInfoHead->_A8  = 1;
	mInfo = createSimpleTextOnTask(1, -1, &sInfoAt, &sInfoBox, &sInfoGlyph, STAGE_STRING(37));
	mInfo->mBox = box;
	mInfo->_A8  = 1;
}

// 0x8017800C
void TModeStageMenu::showInfo()
{
	if (mInfoHead != 0) {
		mInfoHead->resetPage(STAGE_STRING(29 + mStage));
		mInfoHead->mFlags &= ~0x10;
	}
	if (mInfo != 0) {
		mInfo->resetPage(STAGE_STRING(mStage + mWorld * 8 + 37));
		mInfo->mFlags &= ~0x10;
		mInfo->mAt = sInfoAt;
	}
}

// 0x801780D4
void TModeStageMenu::hideInfo()
{
	hideText(mInfoHead);
	hideText(mInfo);
}

// 0x80178108
void TModeStageMenu::createYesNo()
{
	mYes = createSimpleTextOnTask(1, -1, &sTitleAt, &sTitleBox, &sTitleGlyph, getStageSelectWarning(0x27));
	mNo  = createSimpleTextOnTask(1, -1, &sTitleAt, &sTitleBox, &sTitleGlyph, getStageSelectWarning(0x28));
}

#define showAt(text, at, box) \
	text->mFlags &= ~0x10; \
	text->mAt  = at; \
	text->mBox = box; \
	text->_A8  = 1

// 0x80178188
void TModeStageMenu::showYesNo()
{
	Vec3f box = vec3f(0.0f, 24.0f, 0.0f);
	if (mYes != 0) {
		showAt(mYes, sYesNoAt0, box);
	}
	if (mNo != 0) {
		showAt(mNo, sYesNoAt1, box);
	}
}

// 0x80178280
void TModeStageMenu::hideYesNo()
{
	hideText(mYes);
	hideText(mNo);
}

// 0x801782B4
void TModeStageMenu::createTabs()
{
	mTab[0] = createSimpleTextOnTask(1, -1, &sTitleAt, &sTitleBox, &sTitleGlyph, STAGE_STRING(0));
	mTab[1] = createSimpleTextOnTask(1, -1, &sTitleAt, &sTitleBox, &sTitleGlyph, STAGE_STRING(1));
}

// 0x8017832C
void TModeStageMenu::showTabs()
{
	Vec3f box = vec3f(0.0f, 24.0f, 0.0f);
	if (mTab[0] != 0) {
		showAt(mTab[0], sTabAt0, box);
	}
	if (mTab[1] != 0) {
		showAt(mTab[1], sTabAt1, box);
	}
}

// 0x80178424
void TModeStageMenu::hideTabs()
{
	hideText(mTab[0]);
	hideText(mTab[1]);
}

// 0x80178458
void TModeStageMenu::createHelp(int msg)
{
	Vec3f box = vec3f(0.0f, 22.0f, 0.0f);
	mHelp = createSimpleTextOnTask(1, -1, &sHelpAt, &sHelpBox, &sHelpGlyph, STAGE_STRING(msg));
	if (mHelp != 0) {
		mHelp->mBox = box;
		mHelp->_A8  = 1;
	}
}

// 0x8017850C
void TModeStageMenu::showHelp(int kind)
{
	if (mHelp != 0) {
		mHelp->resetPage(STAGE_STRING(kind));
		if (kind == 2) {
			mHelp->mAt = sHelpAt;
		} else {
			mHelp->mAt = sHelpLowAt;
		}
		mHelp->mFlags &= ~0x10;
	}
}

// 0x801785B0
void TModeStageMenu::hideHelp()
{
	hideText(mHelp);
}

// 0x801785CC
void TModeStageMenu::createCancel()
{
	mCancel = createSimpleTextOnTask(1, -1, &sTitleAt, &sTitleBox, &sTitleGlyph, STAGE_STRING(4));
}

// 0x80178620
void TModeStageMenu::showChoices3()
{
	Vec3f box = vec3f(0.0f, 24.0f, 0.0f);
	if (mYes != 0) {
		showAt(mYes, sChoice3At0, box);
	}
	if (mNo != 0) {
		showAt(mNo, sChoice3At1, box);
	}
	if (mCancel != 0) {
		showAt(mCancel, sChoice3At2, box);
	}
}

// 0x80178780
void TModeStageMenu::hideChoices3()
{
	hideText(mYes);
	hideText(mNo);
	hideText(mCancel);
}

// 0x801787CC
void TModeStageMenu::showGbaInfo()
{
	if (mInfo != 0) {
		mInfo->resetPage(STAGE_STRING(19 + mGba));
		mInfo->mFlags &= ~0x10;
		mInfo->mAt = sGbaInfoAt;
	}
}

// 0x8017884C
void TModeStageMenu::hideGbaInfo()
{
	hideText(mInfo);
}

// 0x80178868
void TModeStageMenu::closeBox()
{
	mBoxKind  = 0;
	mChoice   = 0;
	mBoxTimer = 0;
	if (mBox != 0) {
		mBox->mHold = 1;
		mBox        = 0;
	}
	closeYesNoWindow();
	closeChoiceWindow();
	alphaFadeClear();
}

// 0x801788CC
void TModeStageMenu::closeYesNoWindow()
{
	TAeObjMsgWindow* window = mWindow;
	if (window != 0) {
		window->mDieOnClose = TRUE;
		window->close();
		mWindow = 0;
	}
	hideYesNo();
}

// 0x8017891C
void TModeStageMenu::closeChoiceWindow()
{
	TAeObjMsgWindow* window = mWindow;
	if (window != 0) {
		window->mDieOnClose = TRUE;
		window->close();
		mWindow = 0;
	}
	hideChoices3();
}

// 0x8017896C
void TModeStageMenu::openYesNo(void* message)
{
	closeBox();
	mChoice  = 1;
	mBoxKind = 1;
	if (mBox == 0) {
		mBox = createSimpleTextByKind(gTL_07, message, -1);
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
	alphaFadeDim(0x3C);
}

// 0x80178A2C
void TModeStageMenu::openChoice3(void* message)
{
	closeBox();
	mBoxKind  = 2;
	mBoxTimer = 0;
	if (mBox == 0) {
		mBox = createSimpleTextByKind(gTL_07, message, -1);
		if (mBox != 0) {
			mBox->_A8 = 1;
		}
	}
	if (mWindow == 0) {
		mWindow = createMsgWindow(gTL_07, 8);
		if (mWindow != 0) {
			mWindow->setPrompt(3);
			mWindow->setCursor(mChoice);
		}
	}
	showChoices3();
	alphaFadeDim(0x3C);
}

// 0x80178AF0
void TModeStageMenu::openMessage(void* message)
{
	closeBox();
	mBoxKind = 3;
	if (mBox == 0) {
		mBox = createSimpleTextByKind(gTL_07, message, -1);
		if (mBox != 0) {
			mBox->_A8 = 1;
		}
	}
	alphaFadeDim(0x3C);
}

// 0x80178B64
void TModeStageMenu::openTimed(void* message)
{
	closeBox();
	mBoxKind  = 4;
	mBoxTimer = 0x1E;
	if (mBox == 0) {
		mBox = createSimpleTextId2(gTL_07, message);
		if (mBox != 0) {
			mBox->_A8 = 1;
		}
	}
	alphaFadeDim(0x3C);
}

// 0x80178BDC
// 1 once the box is answered or done
int TModeStageMenu::runBox()
{
	int done = 0;
	switch (mBoxKind) {
	case 0:
	default:
		done = 1;
		break;
	case 1:
		if (repeated(4) != 0) {
			mChoice++;
			mChoice = mChoice >= 2 ? 0 : mChoice;
			sePlay(0x1B);
		} else if (repeated(8) != 0) {
			mChoice--;
			mChoice = mChoice < 0 ? 1 : mChoice;
			sePlay(0x1B);
		}
		if (mWindow != 0) {
			mWindow->setCursor(mChoice);
		}
		if (pressedB()) {
			sePlay(0x20);
			done    = 1;
			mChoice = 1;
		} else if (pressedA()) {
			sePlay(0x1C);
			done = 1;
		}
		break;
	case 2:
		if (repeated(4) != 0) {
			mChoice++;
			mChoice = mChoice >= 3 ? 0 : mChoice;
			sePlay(0x1B);
		} else if (repeated(8) != 0) {
			mChoice--;
			mChoice = mChoice < 0 ? 2 : mChoice;
			sePlay(0x1B);
		}
		if (mWindow != 0) {
			mWindow->setCursor(mChoice);
		}
		if (pressedA()) {
			sePlay(0x1C);
			done = 1;
		} else if (pressedB()) {
			mChoice = 1;
			sePlay(0x20);
			done = 1;
		}
		break;
	case 3:
		if (mBox != 0 && mBox->mState == 3) {
			done = 1;
		}
		break;
	case 4:
		if (mBoxTimer == 0) {
			if (mBox != 0) {
				mBox->closeText();
				done = 1;
			}
		} else {
			mBoxTimer--;
		}
		break;
	}
	return done;
}

// 0x80178E6C
int TModeStageMenu::choseYes()
{
	return mChoice == 0;
}
