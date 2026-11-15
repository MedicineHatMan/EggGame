#include "ui/TAeObjMainMenu.h"
#include "engine/DrawTask.h"
#include "ui/AeArc.h"
#include "ui/AeUtil.h"
#include "core/TMainTask.h"
struct NJS_TEXLIST;

// TAeObjMainMenu class at 0x8017E294 Vtable 0x80262410
// The destructor clears a global and does not remove the draw task
// The loader is 0x8017E00C

typedef char TAeObjMainMenu_size_check[(sizeof(TAeObjMainMenu) == 0x80) ? 1 : -1];

// The tables are this file's aeMainMenuShowFile reaches them all from the start of its .rodata 0x8024AD30
extern "C" {
// Two RGBA colours ahead of the bank list
extern const f32 gAeMainMenuColors[8];
const f32 gAeMainMenuColors[8] = { 1.0f, 0.94f, 0.94f, 0.157f, 1.0f, 0.0f, 0.94f, 0.157f };
const u32 gAeMainMenuBanks[8]  = { 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xFFFFFFFF };
const u32 gFilePanelSlots[3]   = { 0x9B, 0x9A, 0x99 };
const u32 gFileCursorSlots[3]  = { 0xB1, 0xB0, 0xAF };
const u32 gFileBarSlots[3]     = { 0x98, 0x97, 0x96 };
const int gFileWorldIcons[7]   = { 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D };
const int gFileWorldNames[7]   = { 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25 };
}

// Defined ahead of the loader its string comes before the archive name
const char* gTAeObjMainMenuName = "TAeObjMainMenu";

// 0x8017E00C
extern "C" void loadAeMainMenu()
{
	loadArc(getLangFileName("ae_main_menu.arc"), &sAeMainMenuArc);
	if (sAeMainMenuArc != 0) {
		AeArcBlock* block = aeArcBlock(sAeMainMenuArc);
		sAeMainMenuAe     = block->mAe;
		relocateAe(sAeMainMenuAe);
		sAeMainMenuTexList = block->mTexList;
		bindGvmTextures((u8*)block->mGvm, sAeMainMenuTexList);
		u8* buf = new u8[0x800];
		if (block->mPalettes != 0) {
			loadE3Palette(block->mPalettes[0], (E3Palette*)&buf, gAeMainMenuBanks[0]);
			loadE3Palette(block->mPalettes[1], (E3Palette*)&buf, gAeMainMenuBanks[1]);
			loadE3Palette(block->mPalettes[2], (E3Palette*)&buf, gAeMainMenuBanks[2]);
			loadE3Palette(block->mPalettes[3], (E3Palette*)&buf, gAeMainMenuBanks[3]);
			loadE3Palette(block->mPalettes[4], (E3Palette*)&buf, gAeMainMenuBanks[4]);
			loadE3Palette(block->mPalettes[5], (E3Palette*)&buf, gAeMainMenuBanks[5]);
			loadE3Palette(block->mPalettes[6], (E3Palette*)&buf, gAeMainMenuBanks[6]);
		}
		delete[] buf;
		njSetTexture(sAeMainMenuTexList);
		njSetPaletteBankNum(0, gAeMainMenuBanks[0]);
		njSetPaletteBankNum(1, gAeMainMenuBanks[1]);
		njSetPaletteBankNum(2, gAeMainMenuBanks[2]);
		njSetPaletteBankNum(3, gAeMainMenuBanks[3]);
		njSetPaletteBankNum(4, gAeMainMenuBanks[4]);
		njSetPaletteBankNum(5, gAeMainMenuBanks[5]);
		njSetPaletteBankNum(6, gAeMainMenuBanks[6]);
		GXInvalidateTexAll();
	}
}

// 0x8017E1E8
extern "C" void freeAeMainMenu()
{
	if (sAeMainMenuArc != 0) {
		releaseTextures((NJS_TEXLIST*)sAeMainMenuTexList);
		sAeMainMenuTexList = 0;
		releaseResource(sAeMainMenuArc);
		sAeMainMenuArc = 0;
		sAeMainMenuAe  = 0;
	}
}

// 0x8017E234
extern "C" TAeObjMainMenu* createAeMainMenu()
{
	TAeObjMainMenu* made;

	if (sAeMainMenuArc == 0) {
		return 0;
	}
	made        = new TAeObjMainMenu(gTL_07, sAeMainMenuAe, sAeMainMenuTexList);
	gAeMainMenu = made;
	return made;
}

static inline TAeLayer* make(TAeObjMainMenu* self, int id, u32 flags)
{
	TAeLayer* layer = self->createLayer(id, flags);
	if (layer != 0) {
		layer->mBanks = gAeMainMenuBanks;
	}
	return layer;
}

// The ctor stores each layer first and tests the member it stored
#define MAKE_LAYER(member, id, flags)      \
	member = createLayer(id, flags);       \
	if (member != 0) {                     \
		member->mBanks = gAeMainMenuBanks; \
	}

// 0x8017E294
TAeObjMainMenu::TAeObjMainMenu(TObject* parent, void* data, void* assets)
    : TAeObject(parent, data, assets)
{
	int i;

	_00   = (u32)gTAeObjMainMenuName;
	mSize = 0x80;
	mTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3C, 0);
	MAKE_LAYER(mBack, 0xBA, 0x10);
	mReady = 0;
	MAKE_LAYER(mPage, 0x96, 0x10);
	if (mPage != 0) {
		mPage->setFrame(0xAA, 0x47);
	}
	MAKE_LAYER(mFileInfo, 3, 0);
	for (i = 0; i < 3; i++) {
		MAKE_LAYER(mPanel[i], 0x35, 0);
		MAKE_LAYER(mBar[i], 0x31, 0);
		if (mBar[i] != 0) {
			mBar[i]->_A8 = 0.0f;
			mBar[i]->_AC = 0;
		}
	}
	MAKE_LAYER(mCursor, 0x93, 0);
	MAKE_LAYER(mSoundRow[0], 0x50, 0);
	MAKE_LAYER(mSoundRow[1], 0x58, 0);
	_6C = 0.0f;
	_70 = 0.1f;
}

// 0x8017E4D4
TAeObjMainMenu::~TAeObjMainMenu()
{
	gAeMainMenu = 0;
}

// 0x8017E538
void TAeObjMainMenu::draw()
{
	int i;

	if (mBack != 0) {
		mBack->update();
	}
	if (mPage != 0) {
		mReady = mPage->play();
	}
	if (mFileInfo != 0) {
		mFileInfo->play();
	}
	for (i = 0; i < 3; i++) {
		if (mPanel[i] != 0) {
			mPanel[i]->update();
		}
		if (mBar[i] != 0) {
			mBar[i]->play();
		}
	}
	if (mCursor != 0) {
		mCursor->update();
	}
	if (mSoundRow[0] != 0) {
		mSoundRow[0]->update();
	}
	if (mSoundRow[1] != 0) {
		mSoundRow[1]->update();
	}
}

// 0x8017E600
void TAeObjMainMenu::render()
{
	if (mBack != 0) {
		mBack->present();
	}
	if (mPage != 0) {
		mPage->present();
	}
}

// The page slot each file's panel sits in
// The page slot each file's cursor sits in
// The page slot each file's bar sits in
// The file-info cells for the world the file has reached
// The sound page's slots for the two rows and each row's animation lit and dim

// 0x8017E648
// Plays the root page in with the three file bars on it
extern "C" void aeMainMenuShowRoot()
{
	int i;

	if (gAeMainMenu == 0) {
		return;
	}
	if (gAeMainMenu->mPage != 0) {
		gAeMainMenu->mReady = 0;
		gAeMainMenu->mPage->reset(0x96, 0x10);
		if (gAeMainMenu->mPage != 0) {
			gAeMainMenu->mPage->mBanks = gAeMainMenuBanks;
		}
		gAeMainMenu->mPage->_AC = 1;
		gAeMainMenu->mPage->_A8 = 2.0f;
		for (i = 0; i < 3; i++) {
			if (gAeMainMenu->mBar[i] != 0) {
				gAeMainMenu->mPage->bind(gFileBarSlots[i], gAeMainMenu->mBar[i], 0);
			}
		}
	}
	if (gAeMainMenu->mFileInfo != 0) {
		gAeMainMenu->mFileInfo->_AC = 1;
		gAeMainMenu->mFileInfo->_A8 = 2.0f;
	}
}

// 0x8017E758
// Puts the root page and the file info straight at their shown time
extern "C" void aeMainMenuSnapRoot()
{
	if (gAeMainMenu == 0) {
		return;
	}
	if (gAeMainMenu->mPage != 0) {
		gAeMainMenu->mPage->_A8   = 2.0f;
		gAeMainMenu->mPage->mTime = 2.0f;
		gAeMainMenu->mReady       = 0;
	}
	if (gAeMainMenu->mFileInfo != 0) {
		gAeMainMenu->mFileInfo->_A8   = 2.0f;
		gAeMainMenu->mFileInfo->mTime = 2.0f;
	}
}

// 0x8017E7B8
// Plays the root page out
extern "C" void aeMainMenuCloseRoot()
{
	if (gAeMainMenu == 0) {
		return;
	}
	if (gAeMainMenu->mPage != 0) {
		gAeMainMenu->mPage->_A8 = 3.0f;
		gAeMainMenu->mReady     = 0;
	}
	if (gAeMainMenu->mFileInfo != 0) {
		gAeMainMenu->mFileInfo->_A8 = 3.0f;
	}
}

// 0x8017E800
// Plays the file page in
extern "C" void aeMainMenuShowFiles()
{
	if (gAeMainMenu != 0 && gAeMainMenu->mPage != 0) {
		gAeMainMenu->mPage->reset(0xAC, 0x10);
		gAeMainMenu->mPage->mBanks = gAeMainMenuBanks;
		gAeMainMenu->mPage->clearSlots();
		gAeMainMenu->mPage->_AC = 1;
		gAeMainMenu->mPage->_A8 = 1.0f;
		gAeMainMenu->mReady     = 0;
	}
}

// 0x8017E894
// Plays the file page on to the root page
extern "C" void aeMainMenuFilesToRoot()
{
	if (gAeMainMenu == 0) {
		return;
	}
	if (gAeMainMenu->mPage == 0) {
		return;
	}
	gAeMainMenu->mPage->_A8 = 2.0f;
	gAeMainMenu->mReady     = 0;
}

// 0x8017E8C4
// Plays the sound page in with its two rows
extern "C" void aeMainMenuShowSound()
{
	if (gAeMainMenu != 0 && gAeMainMenu->mPage != 0) {
		gAeMainMenu->mPage->reset(0xB3, 0x10);
		if (gAeMainMenu->mSoundRow[0] != 0) {
			gAeMainMenu->mPage->bind(0xB3, gAeMainMenu->mSoundRow[0], 0);
		}
		if (gAeMainMenu->mSoundRow[1] != 0) {
			gAeMainMenu->mPage->bind(0xB4, gAeMainMenu->mSoundRow[1], 0);
		}
		if (gAeMainMenu->mPage != 0) {
			gAeMainMenu->mPage->mBanks = gAeMainMenuBanks;
		}
		gAeMainMenu->mPage->_AC = 1;
		gAeMainMenu->mPage->_A8 = 1.0f;
		gAeMainMenu->mReady     = 0;
	}
}

// 0x8017E9A8
// Plays the sound page back out and lets go of its rows
extern "C" void aeMainMenuHideSound()
{
	if (gAeMainMenu != 0 && gAeMainMenu->mPage != 0) {
		gAeMainMenu->mPage->_AC = 2;
		gAeMainMenu->mPage->_A8 = 0.0f;
		gAeMainMenu->mReady     = 0;
		gAeMainMenu->mPage->clearSlot(0xB3);
		gAeMainMenu->mPage->clearSlot(0xB4);
	}
}

// 0x8017EA28
// Fills a file's bar to its share of the emblems
extern "C" void aeMainMenuSetFileBar(int file, FileInfo* info)
{
	if (gAeMainMenu != 0 && gAeMainMenu->mBar[file] != 0 && info->mWorld != -1) {
		f32 goal = 2.0f * ((f32)info->mEmblems / 60.0f);

		gAeMainMenu->mBar[file]->reset(0x2D, 0x10);
		gAeMainMenu->mBar[file]->mBanks = gAeMainMenuBanks;
		gAeMainMenu->mBar[file]->_A8    = goal;
		gAeMainMenu->mBar[file]->_AC    = 0;
	}
}

// 0x8017EB08
// Empties a file's bar
extern "C" void aeMainMenuClearFileBar(int file)
{
	if (gAeMainMenu != 0 && gAeMainMenu->mBar[file] != 0) {
		gAeMainMenu->mBar[file]->cue(0);
		gAeMainMenu->mBar[file]->_A8 = 0.0f;
		gAeMainMenu->mBar[file]->_AC = 0;
	}
}

// 0x8017EB80
// Picks a file its panel lit the others out and its world lives and emblems in the file info
extern "C" void aeMainMenuShowFile(int file, const FileInfo* info, BOOL instant)
{
	int i;

	gAeMainMenu->mFile = file;
	if (gAeMainMenu == 0) {
		return;
	}
	if (gAeMainMenu->mPage == 0) {
		return;
	}
	for (i = 0; i < 3; i++) {
		if (gAeMainMenu->mPanel[i] != 0) {
			if (i == file) {
				gAeMainMenu->mPage->bind(gFilePanelSlots[i], gAeMainMenu->mPanel[i], 0);
			} else {
				gAeMainMenu->mPage->clearSlot(gFilePanelSlots[i]);
			}
		}
	}
	if (gAeMainMenu->mFileInfo != 0) {
		if (info->mEmblems == 60) {
			gAeMainMenu->mFileInfo->reset(0xA, 0x10);
			gAeMainMenu->mFileInfo->mBanks = gAeMainMenuBanks;
		} else if (info->mWorld == -1) {
			gAeMainMenu->mFileInfo->reset(3, 0x10);
			gAeMainMenu->mFileInfo->mBanks = gAeMainMenuBanks;
		} else {
			gAeMainMenu->mFileInfo->reset(5, 0x10);
			gAeMainMenu->mFileInfo->mBanks = gAeMainMenuBanks;
			gAeMainMenu->mFileInfo->setFrame(8, gFileWorldIcons[info->mWorld]);
			gAeMainMenu->mFileInfo->setFrame(7, gFileWorldNames[info->mWorld]);
		}
		gAeMainMenu->mFileInfo->_AC = 1;
		gAeMainMenu->mFileInfo->_A8 = 2.0f;
		if (!instant) {
			gAeMainMenu->mFileInfo->mTime = 2.0f;
		}
		gAeMainMenu->mPage->bind(0xAA, gAeMainMenu->mFileInfo, 0);
	}
	aeClipSetDigits2(gAeMainMenu->mPage, info->mLives, 0x3C, 0xA6, 0xA7);
	aeClipSetDigits2(gAeMainMenu->mPage, info->mEmblems, 0x3C, 0xA8, 0xA9);
}

// 0x8017EDC0
// Takes every file's panel off the page
extern "C" void aeMainMenuHidePanels()
{
	int i;

	if (gAeMainMenu != 0 && gAeMainMenu->mPage != 0) {
		for (i = 0; i < 3; i++) {
			gAeMainMenu->mPage->clearSlot(gFilePanelSlots[i]);
		}
	}
}

// 0x8017EE30
// Puts the cursor on one file's slot
extern "C" void aeMainMenuSetFileCursor(int file)
{
	int i;

	if (gAeMainMenu == 0) {
		return;
	}
	for (i = 0; i < 3; i++) {
		if (gAeMainMenu->mPage != 0) {
			if (i == file) {
				if (gAeMainMenu->mCursor != 0) {
					gAeMainMenu->mPage->bind(gFileCursorSlots[i], gAeMainMenu->mCursor, 0);
				}
			} else {
				gAeMainMenu->mPage->clearSlot(gFileCursorSlots[i]);
			}
		}
	}
}

// 0x8017EED8
extern "C" void aeMainMenuHideFileCursor()
{
	int i;

	if (gAeMainMenu == 0) {
		return;
	}
	for (i = 0; i < 3; i++) {
		if (gAeMainMenu->mPage != 0) {
			gAeMainMenu->mPage->clearSlot(gFileCursorSlots[i]);
		}
	}
}

// A macro the DOL reads the animation id at each use
#define setSoundRow(row, anim)                                                                \
	if (gAeMainMenu->mSoundRow[row] != 0 && (anim) != gAeMainMenu->mSoundRow[row]->getId()) { \
		gAeMainMenu->mSoundRow[row]->reset((anim), 1);                                        \
		gAeMainMenu->mSoundRow[row]->mBanks = gAeMainMenuBanks;                               \
	}

// 0x8017EF44
// Lights the chosen sound row with the cursor on it and dims the other
extern "C" void aeMainMenuSetSoundRow(int choice)
{
	int i;

	if (gAeMainMenu == 0) {
		return;
	}
	for (i = 0; i < 2; i++) {
		if (gAeMainMenu->mPage != 0) {
			if (i == choice) {
				setSoundRow(i, gSoundRowLit[i]);
				if (gAeMainMenu->mCursor != 0) {
					gAeMainMenu->mPage->bind(gSoundRowSlots[i], gAeMainMenu->mCursor, 0);
				}
			} else {
				setSoundRow(i, gSoundRowDim[i]);
				gAeMainMenu->mPage->clearSlot(gSoundRowSlots[i]);
			}
		}
	}
}

// 0x8017F0A4
// Dims both sound rows and takes the cursor off them
extern "C" void aeMainMenuClearSoundRows()
{
	int i;

	if (gAeMainMenu == 0) {
		return;
	}
	for (i = 0; i < 2; i++) {
		setSoundRow(i, gSoundRowDim[i]);
		if (gAeMainMenu->mPage != 0) {
			gAeMainMenu->mPage->clearSlot(gSoundRowSlots[i]);
		}
	}
}

// 0x8017F168
// Whether the page has reached the time it is playing toward
extern "C" BOOL aeMainMenuIsReady()
{
	if (gAeMainMenu != 0) {
		return gAeMainMenu->mReady;
	}
	return FALSE;
}
