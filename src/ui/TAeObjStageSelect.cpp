#include "ui/TAeObjStageSelect.h"
#include "engine/DrawTask.h"
#include "ui/AeArc.h"
#include "stage/Stage.h"
#include "object/yellow/TObjYellowDoubleFog.h"
#include "ui/AeUtil.h"
#include "object/yellow/TObjYellowTornadoScene.h"
#include "core/TMainTask.h"
struct NJS_TEXLIST;
struct TObjYellowTornadoScene;

// TAeObjStageSelect class at 0x8017BF7C Vtable 0x802623A8
// The destructor clears a global and does not remove the draw task

// 0x8017D5C0
// finishes construction The loader is 0x8017BC34

// Palette banks the last is unused
static const u32 gAeStageSelectBanks[] = { 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0xFFFFFFFF };
static const u32 gWorldCells[]         = { 0xDF, 0xE1, 0xE3, 0xE5, 0xE7, 0xDD, 0xE9, 0xD9, 0xDB };
static const int gWorldFrames[]        = { 0x12, 0x13, 0x14, 0x15, 0x16, 0x11, 0x17, 0xF, 0x10 };
static const int gStageKindCells[]     = { 0xB3, 0xB6, 0xB9, 0xBC };
static const u32 gStageIconSlots[]     = { 0xC3, 0xC2, 0xC1, 0xC0, 0xBF, 0xC6, 0xC5, 0xC4, 0xCB, 0xCA, 0xC9, 0xC8, 0xC7, 0xCE, 0xCD, 0xCC };
static const u32 gRankAnims[]          = { 0x8A, 0x86, 0x82, 0x75, 0x90, 0x8E };
static const u32 gGbaRowSlots[]        = { 0x145, 0x144, 0x143, 0x142, 0x141 };
static const int gGbaCells[]           = { 0x8, 0xA, 0xC, 0xE, 0x10 };
// Each world's colour as r g b and a
static const f32 gWorldTints[8][4] = {
	{ 1.0f, 0.01961f, 0.4902f, 0.92157f },
	{ 1.0f, 1.0f, 0.0f, 0.0f },
	{ 1.0f, 0.62745f, 0.23529f, 0.72941f },
	{ 1.0f, 1.0f, 0.5098f, 0.0f },
	{ 1.0f, 1.0f, 1.0f, 0.0f },
	{ 1.0f, 0.0f, 0.64706f, 0.31373f },
	{ 1.0f, 1.0f, 1.0f, 1.0f },
	{ 1.0f, 0.20392f, 0.95686f, 0.7451f },
};

const char* gTAeObjStageSelectName = "TAeObjStageSelect";

typedef char TAeObjStageSelect_size_check[(sizeof(TAeObjStageSelect) == 0xB8) ? 1 : -1];

// 0x8017BC34
extern "C" void loadAeStageSelect()
{
	loadArc(getLangFileName("ae_stage_select.arc"), &sAeStageSelectArc);
	if (sAeStageSelectArc != 0) {
		AeArcBlock* block = aeArcBlock(sAeStageSelectArc);
		sAeStageSelectAe  = block->mAe;
		relocateAe(sAeStageSelectAe);
		sAeStageSelectTexList = block->mTexList;
		bindGvmTextures((u8*)block->mGvm, sAeStageSelectTexList);
		u8* buf = new u8[0x800];
		if (block->mPalettes != 0) {
			loadE3Palette(block->mPalettes[0], (E3Palette*)&buf, gAeStageSelectBanks[0]);
			loadE3Palette(block->mPalettes[1], (E3Palette*)&buf, gAeStageSelectBanks[1]);
			loadE3Palette(block->mPalettes[2], (E3Palette*)&buf, gAeStageSelectBanks[2]);
			loadE3Palette(block->mPalettes[3], (E3Palette*)&buf, gAeStageSelectBanks[3]);
			loadE3Palette(block->mPalettes[4], (E3Palette*)&buf, gAeStageSelectBanks[4]);
			loadE3Palette(block->mPalettes[5], (E3Palette*)&buf, gAeStageSelectBanks[5]);
			loadE3Palette(block->mPalettes[6], (E3Palette*)&buf, gAeStageSelectBanks[6]);
			loadE3Palette(block->mPalettes[7], (E3Palette*)&buf, gAeStageSelectBanks[7]);
			loadE3Palette(block->mPalettes[8], (E3Palette*)&buf, gAeStageSelectBanks[8]);
			loadE3Palette(block->mPalettes[9], (E3Palette*)&buf, gAeStageSelectBanks[9]);
			loadE3Palette(block->mPalettes[10], (E3Palette*)&buf, gAeStageSelectBanks[10]);
		}
		delete[] buf;
		njSetTexture(sAeStageSelectTexList);
		njSetPaletteBankNum(0, gAeStageSelectBanks[0]);
		njSetPaletteBankNum(1, gAeStageSelectBanks[1]);
		njSetPaletteBankNum(2, gAeStageSelectBanks[2]);
		njSetPaletteBankNum(3, gAeStageSelectBanks[3]);
		njSetPaletteBankNum(4, gAeStageSelectBanks[4]);
		njSetPaletteBankNum(5, gAeStageSelectBanks[5]);
		njSetPaletteBankNum(6, gAeStageSelectBanks[6]);
		njSetPaletteBankNum(7, gAeStageSelectBanks[7]);
		njSetPaletteBankNum(8, gAeStageSelectBanks[8]);
		njSetPaletteBankNum(9, gAeStageSelectBanks[9]);
		njSetPaletteBankNum(10, gAeStageSelectBanks[10]);
		GXInvalidateTexAll();
	}
}

// 0x8017BED0
extern "C" void freeAeStageSelect()
{
	if (sAeStageSelectArc != 0) {
		releaseTextures((NJS_TEXLIST*)sAeStageSelectTexList);
		sAeStageSelectTexList = 0;
		releaseResource(sAeStageSelectArc);
		sAeStageSelectArc = 0;
		sAeStageSelectAe  = 0;
	}
}

// 0x8017BF1C
extern "C" TAeObjStageSelect* createAeStageSelect()
{
	TAeObjStageSelect* made;

	if (sAeStageSelectArc == 0) {
		return 0;
	}
	made           = new TAeObjStageSelect(gTL_07, sAeStageSelectAe, sAeStageSelectTexList);
	gAeStageSelect = made;
	return made;
}

// 0x8017BF7C
// Stores the new layer in field then gives it the palettes
#define MAKE_LAYER(field, id, flags)         \
	field = createLayer(id, flags);          \
	if (field != 0) {                        \
		field->mBanks = gAeStageSelectBanks; \
	}

TAeObjStageSelect::TAeObjStageSelect(TObject* parent, void* data, void* assets)
    : TAeObject(parent, data, assets)
{
	int i;

	_00   = (u32)gTAeObjStageSelectName;
	mSize = 0xB8;
	mTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3C, 0);
	MAKE_LAYER(mBack, 0x101, 0x10)
	MAKE_LAYER(mSide, 0x103, 0x10)
	MAKE_LAYER(mStatus, 0x106, 0x10)
	if (mStatus != 0) {
		mStatus->_A8 = 0.67f;
		mStatus->_AC = 0;
	}
	MAKE_LAYER(mBase, 0x10F, 0x10)
	MAKE_LAYER(mPanel, 0x111, 0x10)
	if (mPanel != 0) {
		mPanel->_A8 = 0.0f;
		mPanel->_AC = 0;
	}
	MAKE_LAYER(mTint, 0x5E, 0)
	for (i = 0; i < 8; i++) {
		MAKE_LAYER(mStageIcon[i], 0x3C, 0)
		MAKE_LAYER(mStageRing[i], 0x2B, 0x10)
		if (mStageIcon[i] != 0) {
			static_cast<TAeClip*>(mStageIcon[i])->pause();
		}
		if (mStageRing[i] != 0) {
			static_cast<TAeClip*>(mStageRing[i])->pause();
		}
	}
	MAKE_LAYER(mRank, 0x8A, 0)
	MAKE_LAYER(mStageInfo, 0x6F, 0)
	for (i = 0; i < 5; i++) {
		MAKE_LAYER(mGbaRow[i], 0x17, 0)
	}
	MAKE_LAYER(mGbaCursor, 0x24, 0)
	if (mStatus != 0) {
		mStatus->setCell(0x107, 0xB3);
	}
	attachWorld();
	mWorld = 0;
	mStage = 0;
	mDone  = 0;
}

// 0x8017C2DC
TAeObjStageSelect::~TAeObjStageSelect()
{
	gAeStageSelect = 0;
}

// 0x8017C340
void TAeObjStageSelect::draw()
{

	if (mBack != 0) {
		mBack->update();
		if (mBack->isDone() != 0) {
			mDone = 1;
		}
	}
	if (mSide != 0) {
		mSide->update();
	}
	if (mStatus != 0 && mStatus->play() != 0) {
		mPanel->update();
	}
	if (mBase != 0) {
		mBase->update();
	}
	if (mPanel != 0) {
		mPanel->play();
	}
	if (mTint != 0) {
		mTint->update();
	}
	if (mStageInfo != 0) {
		mStageInfo->update();
	}
	if (mRank != 0) {
		mRank->update();
	}
	for (s8 i = 0; i < 8; i++) {
		if (mStageIcon[i] != 0) {
			mStageIcon[i]->update();
		}
		if (mStageRing[i] != 0) {
			mStageRing[i]->update();
		}
	}
	for (s8 i = 0; i < 5; i++) {
		if (mGbaRow[i] != 0) {
			mGbaRow[i]->update();
		}
	}
	if (mGbaCursor != 0) {
		mGbaCursor->update();
	}
}

// 0x8017C49C
void TAeObjStageSelect::render()
{
	if (mBase != 0) {
		mBase->present();
	}
	if (mPanel != 0) {
		mPanel->present();
	}
	if (mStatus != 0) {
		mStatus->present();
	}
	if (mSide != 0) {
		mSide->present();
	}
	if (mBack != 0) {
		mBack->present();
	}
}

// 0x8024AB98
// The world panel's cell for each world and its title frame
// 0x8024ABE0
// The status bar's cell for each kind of stage normal boss and the two last ones
// 0x8024ABF0
// The world panel's slot for each stage icon
// 0x8024AC30
// The rank animation for each rank 5 for a stage not cleared
// 0x8024AC48
// The GBA panel's slot for each game row and the picture for each game
// 0x8024AC70
// The tint for each world

enum {
	STAGE_ICON_OPEN      = 0x30,
	STAGE_ICON_OPEN_LIT  = 0x32,
	STAGE_ICON_CLEAR     = 0x36,
	STAGE_ICON_CLEAR_LIT = 0x38,
	STAGE_ICON_NONE      = 0x3C,
	GBA_ROW_LOCKED       = 0x17,
	GBA_ROW_OPEN         = 0x19,
	GBA_ROW_LIT          = 0x1B,
	GBA_CURSOR           = 0x24,
	GBA_CURSOR_BACK      = 0x26,
	PANEL_WORLD          = 0x111,
	PANEL_OPTIONS        = 0x130,
	PANEL_GBA            = 0x139,
};

// 0x8017C514
// Plays the backdrop in
extern "C" void aeStageSelectOpen()
{
	if (gAeStageSelect != 0 && gAeStageSelect->mBack != 0) {
		gAeStageSelect->mBack->mPlayFlags &= ~0x500;
		gAeStageSelect->mBack->setMode(0x10);
		gAeStageSelect->mBack->wake();
		gAeStageSelect->mDone = FALSE;
	}
}

// 0x8017C588
// Plays the backdrop out
extern "C" void aeStageSelectClose()
{
	if (gAeStageSelect != 0 && gAeStageSelect->mBack != 0) {
		gAeStageSelect->mBack->mPlayFlags &= ~0x500;
		gAeStageSelect->mBack->setMode(0x20);
		gAeStageSelect->mBack->wake();
		gAeStageSelect->mDone = FALSE;
	}
}

// 0x8017C5FC
// Whether the backdrop finished playing
extern "C" BOOL aeStageSelectIsDone()
{
	if (gAeStageSelect != 0) {
		return gAeStageSelect->mDone;
	}
	return FALSE;
}

// 0x8017C618
// Brings the world panel in tinted for the world
extern "C" void aeStageSelectShowWorld(s8 world)
{
	if (gAeStageSelect == 0) {
		return;
	}
	if (gAeStageSelect->mTint != 0) {
		gAeStageSelect->mTint->setColor((f32*)gWorldTints[world]);
	}
	if (gAeStageSelect->mPanel != 0) {
		gAeStageSelect->attachWorld();
		gAeStageSelect->mPanel->_A8 = 0.67f;
		gAeStageSelect->mPanel->wake();
	}
}

// 0x8017C69C
extern "C" void aeStageSelectHideWorld()
{
	if (gAeStageSelect != 0 && gAeStageSelect->mPanel != 0) {
		gAeStageSelect->mPanel->_A8 = 0.0f;
		gAeStageSelect->mPanel->wake();
	}
}

// 0x8017C6E8
// Brings the GBA panel in
extern "C" void aeStageSelectShowGba()
{
	if (gAeStageSelect == 0) {
		return;
	}
	if (gAeStageSelect->mTint != 0) {
		gAeStageSelect->mTint->setColor((f32*)gWorldTints[7]);
	}
	if (gAeStageSelect->mPanel != 0) {
		if (gAeStageSelect->mPanel->getId() != PANEL_GBA) {
			gAeStageSelect->mPanel->reset(PANEL_GBA, 0x10);
			gAeStageSelect->mPanel->mBanks = gAeStageSelectBanks;
			gAeStageSelect->attachGba();
		}
		gAeStageSelect->mPanel->_A8 = 0.67f;
		gAeStageSelect->mPanel->wake();
	}
}

// 0x8017C7A4
extern "C" void aeStageSelectHideGba()
{
	if (gAeStageSelect != 0 && gAeStageSelect->mPanel != 0) {
		gAeStageSelect->mPanel->_A8 = 0.0f;
		gAeStageSelect->mPanel->wake();
	}
}

// 0x8017C7F0
// Brings the options panel in
extern "C" void aeStageSelectShowOptions()
{
	if (gAeStageSelect != 0 && gAeStageSelect->mPanel != 0) {
		if (gAeStageSelect->mPanel->getId() != PANEL_OPTIONS) {
			gAeStageSelect->mPanel->reset(PANEL_OPTIONS, 0x10);
			gAeStageSelect->mPanel->mBanks = gAeStageSelectBanks;
			gAeStageSelect->attachOptions();
		}
		gAeStageSelect->mPanel->_A8 = 0.67f;
		gAeStageSelect->mPanel->wake();
	}
}

static inline void playOut(TAeLayer* layer)
{
	if (layer != 0) {
		layer->mPlayFlags &= ~0x500;
		layer->setMode(0x20);
		layer->wake();
	}
}

// 0x8017C888
// Plays everything out for the stage starting
extern "C" void aeStageSelectCloseAll()
{
	if (gAeStageSelect == 0) {
		return;
	}
	if (gAeStageSelect->mBack != 0) {
		gAeStageSelect->mBack->mPlayFlags &= ~0x500;
		gAeStageSelect->mBack->setMode(0x20);
		gAeStageSelect->mBack->wake();
	}
	// The side layer is played out twice
	if (gAeStageSelect->mSide != 0) {
		gAeStageSelect->mSide->mPlayFlags &= ~0x500;
		gAeStageSelect->mSide->setMode(0x20);
		gAeStageSelect->mSide->wake();
	}
	if (gAeStageSelect->mSide != 0) {
		gAeStageSelect->mSide->mPlayFlags &= ~0x500;
		gAeStageSelect->mSide->setMode(0x20);
		gAeStageSelect->mSide->wake();
	}
	if (gAeStageSelect->mStatus != 0) {
		gAeStageSelect->mStatus->_A8 = 2.0f;
		gAeStageSelect->mStatus->wake();
	}
	if (gAeStageSelect->mPanel != 0) {
		gAeStageSelect->mPanel->_A8 = 2.0f;
		gAeStageSelect->mPanel->wake();
	}
}

// 0x8017C9C8
// Shows the world's name and picture on the world panel
extern "C" void aeStageSelectSetWorld(s8 world)
{
	gAeStageSelect->mWorld = world;
	if (gAeStageSelect != 0 && gAeStageSelect->mPanel != 0) {
		if (gAeStageSelect->mPanel->getId() != PANEL_WORLD) {
			gAeStageSelect->mPanel->reset(PANEL_WORLD, 1);
			gAeStageSelect->mPanel->mBanks = gAeStageSelectBanks;
		}
		gAeStageSelect->mPanel->setCell(0x117, gWorldCells[world]);
		gAeStageSelect->mPanel->setFrame(0x119, gWorldFrames[world]);
	}
}

static inline void setIcon(TAeLayer* icon, u32 anim)
{
	icon->reset(anim, 1);
	icon->mBanks = gAeStageSelectBanks;
}

// 0x8017CA90
// Lights the chosen stage's icon and dims the others
extern "C" void aeStageSelectSetStage(s8 stage)
{
	int i;

	gAeStageSelect->mStage = stage;
	if (gAeStageSelect == 0) {
		return;
	}
	for (i = 0; i < 8; i++) {
		TAeLayer* icon = gAeStageSelect->mStageIcon[i];
		if (icon != 0) {
			if (i == stage) {
				switch (icon->getId()) {
				case STAGE_ICON_OPEN:
					setIcon(icon, STAGE_ICON_OPEN_LIT);
					break;
				case STAGE_ICON_CLEAR:
					setIcon(icon, STAGE_ICON_CLEAR_LIT);
					break;
				}
			} else {
				switch (icon->getId()) {
				case STAGE_ICON_OPEN_LIT:
					setIcon(icon, STAGE_ICON_OPEN);
					break;
				case STAGE_ICON_CLEAR_LIT:
					setIcon(icon, STAGE_ICON_CLEAR);
					break;
				}
			}
		}
	}
}

// 0x8017CBBC
// Lights the chosen game's row or the back button for 5 and shows the game's picture
extern "C" void aeStageSelectSetGba(s8 gba)
{
	s8 i;

	gAeStageSelect->mGba = gba;
	if (gAeStageSelect == 0) {
		return;
	}
	for (i = 0; i < 5; i++) {
		if (gAeStageSelect->mGbaRow[i] != 0) {
			if (i == gba) {
				gAeStageSelect->mGbaRow[i]->reset(GBA_ROW_LIT, 1);
				gAeStageSelect->mGbaRow[i]->mBanks = gAeStageSelectBanks;
			} else if (gAeStageSelect->mGbaRow[i]->getId() == GBA_ROW_LIT) {
				gAeStageSelect->mGbaRow[i]->reset(GBA_ROW_OPEN, 1);
				gAeStageSelect->mGbaRow[i]->mBanks = gAeStageSelectBanks;
			}
		}
	}
	if (gAeStageSelect->mGbaCursor != 0) {
		if (gba == 5) {
			gAeStageSelect->mGbaCursor->reset(GBA_CURSOR_BACK, 1);
			gAeStageSelect->mGbaCursor->mBanks = gAeStageSelectBanks;
		} else {
			gAeStageSelect->mGbaCursor->reset(GBA_CURSOR, 1);
			gAeStageSelect->mGbaCursor->mBanks = gAeStageSelectBanks;
		}
	}
	if (gba < 5 && gAeStageSelect->mPanel != 0) {
		gAeStageSelect->mPanel->setCell(0x146, gGbaCells[gba]);
	}
}

// 0x8017CD48
// Blanks the first five stage icons and takes the rest off the panel
extern "C" void aeStageSelectClearStages()
{
	TAeLayer* panel;
	s8 i;

	if (gAeStageSelect == 0) {
		return;
	}
	panel = gAeStageSelect->mPanel;
	if (panel == 0) {
		return;
	}
	for (i = 0; i < 8; i++) {
		TAeLayer* icon = gAeStageSelect->mStageIcon[i];
		if (i < 5) {
			if (icon != 0) {
				icon->reset(STAGE_ICON_NONE, 1);
			}
		} else {
			panel->clearSlot(gStageIconSlots[i]);
		}
	}
}

// 0x8017CDE8
// Puts a stage's icon on the panel cleared or not
extern "C" void aeStageSelectSetStageIcon(s8 stage, BOOL cleared)
{
	if (gAeStageSelect != 0 && gAeStageSelect->mPanel != 0 && gAeStageSelect->mStageIcon[stage] != 0) {
		if (cleared) {
			gAeStageSelect->mStageIcon[stage]->reset(STAGE_ICON_CLEAR, 1);
		} else {
			gAeStageSelect->mStageIcon[stage]->reset(STAGE_ICON_OPEN, 1);
		}
		gAeStageSelect->mStageIcon[stage]->mBanks = gAeStageSelectBanks;
		gAeStageSelect->mPanel->attach(gStageIconSlots[stage], gAeStageSelect->mStageIcon[stage], 0);
	}
}

// 0x8017CEB4
extern "C" void aeStageSelectSetRank(s8 rank)
{
	if (gAeStageSelect != 0 && gAeStageSelect->mRank != 0 && gRankAnims[rank] != gAeStageSelect->mRank->getId()) {
		gAeStageSelect->mRank->reset(gRankAnims[rank], 1);
		gAeStageSelect->mRank->mBanks = gAeStageSelectBanks;
	}
}

// The digit cells count from frame 0x2E
#define DIGIT(n) ((s8)((n) + 0x2E))

// 0x8017CF40
// Six digits clamped to 0-999999
extern "C" void aeStageSelectSetScore(int score)
{
	TAeLayer* panel;
	int v;
	int n;
	int q;
	s8 d5, d4, d3, d2, d1;

	if (gAeStageSelect != 0 && (panel = gAeStageSelect->mPanel) != 0) {
		v  = score < 0 ? 0 : (score > 999999 ? 999999 : score);
		d5 = DIGIT(v / 100000);
		n  = v % 100000;
		d4 = DIGIT(n / 10000);
		n %= 10000;
		d3 = DIGIT(n / 1000);
		n %= 1000;
		d2 = DIGIT(n / 100);
		n %= 100;
		d1 = DIGIT(n / 10);
		n %= 10;
		panel->setFrame(0x12D, d5);
		gAeStageSelect->mPanel->setFrame(0x12C, d4);
		gAeStageSelect->mPanel->setFrame(0x12B, d3);
		gAeStageSelect->mPanel->setFrame(0x12A, d2);
		gAeStageSelect->mPanel->setFrame(0x129, d1);
		gAeStageSelect->mPanel->setFrame(0x128, DIGIT(n));
	}
}

// 0x8017D0A4
// The golden eggs found in the stage
extern "C" void aeStageSelectSetEggs(s8 eggs)
{
	TAeLayer* panel;
	int n;
	s8 hi;

	if (gAeStageSelect != 0 && (panel = gAeStageSelect->mPanel) != 0) {
		if (eggs < 0) {
			n = 0;
		} else {
			n = eggs > 99 ? 99 : eggs;
		}
		hi = DIGIT(n / 10);
		n %= 10;
		panel->setFrame(0x11E, hi);
		gAeStageSelect->mPanel->setFrame(0x11D, DIGIT(n));
	}
}

// 0x8017D148
// The golden eggs the stage has
extern "C" void aeStageSelectSetEggTotal(s8 total)
{
	TAeLayer* panel;
	int n;
	s8 hi;

	if (gAeStageSelect != 0 && (panel = gAeStageSelect->mPanel) != 0) {
		if (total < 0) {
			n = 0;
		} else {
			n = total > 99 ? 99 : total;
		}
		hi = DIGIT(n / 10);
		n %= 10;
		panel->setFrame(0x11C, hi);
		gAeStageSelect->mPanel->setFrame(0x11B, DIGIT(n));
	}
}

// 0x8017D1EC
// The best time's minutes
extern "C" void aeStageSelectSetMinutes(s8 minutes)
{
	TAeLayer* panel;
	int n;
	s8 hi;

	if (gAeStageSelect != 0 && (panel = gAeStageSelect->mPanel) != 0) {
		if (minutes < 0) {
			n = 0;
		} else {
			n = minutes > 99 ? 99 : minutes;
		}
		hi = DIGIT(n / 10);
		n %= 10;
		panel->setFrame(0x124, hi);
		gAeStageSelect->mPanel->setFrame(0x123, DIGIT(n));
	}
}

// 0x8017D290
// The best time's seconds
extern "C" void aeStageSelectSetSeconds(s8 seconds)
{
	TAeLayer* panel;
	int n;
	s8 hi;

	if (gAeStageSelect != 0 && (panel = gAeStageSelect->mPanel) != 0) {
		if (seconds < 0) {
			n = 0;
		} else {
			n = seconds > 99 ? 99 : seconds;
		}
		hi = DIGIT(n / 10);
		n %= 10;
		panel->setFrame(0x122, hi);
		gAeStageSelect->mPanel->setFrame(0x121, DIGIT(n));
	}
}

// 0x8017D334
// The best time's hundredths from milliseconds
extern "C" void aeStageSelectSetMsecs(s16 msecs)
{
	TAeLayer* panel;
	int n;
	int lo;
	s8 hi;

	if (gAeStageSelect != 0 && (panel = gAeStageSelect->mPanel) != 0) {
		n = msecs / 10;
		if (n < 0) {
			n = 0;
		} else {
			n = n > 99 ? 99 : n;
		}
		for (int i = 0; i < 1; i++) {
			hi = DIGIT(n / 10);
			lo = n % 10;
		}
		panel->setFrame(0x120, hi);
		gAeStageSelect->mPanel->setFrame(0x11F, DIGIT(lo));
	}
}

// 0x8017D3D8
// Locks every game row
extern "C" void aeStageSelectLockGbaRows()
{
	s8 i;

	if (gAeStageSelect == 0) {
		return;
	}
	for (i = 0; i < 5; i++) {
		if (gAeStageSelect->mGbaRow[i] != 0) {
			gAeStageSelect->mGbaRow[i]->reset(GBA_ROW_LOCKED, 1);
			gAeStageSelect->mGbaRow[i]->mBanks = gAeStageSelectBanks;
		}
	}
}

// 0x8017D464
// Opens a game's row
extern "C" void aeStageSelectOpenGbaRow(s8 gba)
{
	if (gAeStageSelect != 0 && gAeStageSelect->mGbaRow[gba] != 0) {
		gAeStageSelect->mGbaRow[gba]->reset(GBA_ROW_OPEN, 1);
		gAeStageSelect->mGbaRow[gba]->mBanks = gAeStageSelectBanks;
	}
}

// 0x8017D4D4
extern "C" void aeStageSelectSetStageKind(s8 kind)
{
	if (gAeStageSelect != 0 && gAeStageSelect->mStatus != 0) {
		gAeStageSelect->mStatus->setCell(0x107, gStageKindCells[kind]);
	}
}

// 0x8017D528
extern "C" void aeStageSelectSetLives(s8 lives)
{
	if (gAeStageSelect != 0 && gAeStageSelect->mStatus != 0) {
		aeClipSetDigits2(gAeStageSelect->mStatus, lives, 0x63, 0x10B, 0x10A);
	}
}

// 0x8017D574
// The count TGameFlow reads from 0x80042994
extern "C" void aeStageSelectSetEmblems(s8 emblems)
{
	if (gAeStageSelect != 0 && gAeStageSelect->mStatus != 0) {
		aeClipSetDigits2(gAeStageSelect->mStatus, emblems, 0x63, 0x108, 0x109);
	}
}

// 0x8017D5C0
// Binds the world panel's parts the tint the rank the stage info and the first five icons
void TAeObjStageSelect::attachWorld()
{
	s8 i;

	if (mPanel == 0) {
		return;
	}
	mPanel->setFrame(0x117, gWorldCells[0]);
	mPanel->attach(0x49, mTint, 0);
	mPanel->attach(0x12E, mRank, 0);
	mPanel->attach(0x113, mStageInfo, 0);
	for (i = 0; i < 5; i++) {
		mPanel->attach(gStageIconSlots[i], mStageIcon[i], 0);
	}
}

// 0x8017D6C8
// Binds the GBA panel's parts the tint the game rows and the cursor
void TAeObjStageSelect::attachGba()
{
	s8 i;

	if (mPanel == 0) {
		return;
	}
	mPanel->attach(0x3F, mTint, 0);
	for (i = 0; i < 5; i++) {
		if (mGbaRow[i] != 0) {
			mPanel->attach(gGbaRowSlots[i], mGbaRow[i], 0);
		}
	}
	if (mGbaCursor != 0) {
		mPanel->attach(0x140, mGbaCursor, 0);
	}
	mPanel->setCell(0x146, 8);
}

// 0x8017D7AC
// Binds the options panel's back button
void TAeObjStageSelect::attachOptions()
{
	if (mPanel != 0 && mGbaCursor != 0) {
		mGbaCursor->reset(GBA_CURSOR_BACK, 1);
		mGbaCursor->mBanks = gAeStageSelectBanks;
		mPanel->attach(0x137, mGbaCursor, 0);
	}
}
