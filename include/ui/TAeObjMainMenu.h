#ifndef _TAEOBJMAINMENU_H
#define _TAEOBJMAINMENU_H
struct TArc;

#include "ui/TAeLayer.h"

struct DrawTask;

/** What readFileInfo reads back for one save file */
struct FileInfo {
	s8 mWorld;    // _00 the furthest world reached -1 for an empty file
	u8 _01;
	s16 mLives;   // _02
	s16 mEmblems; // _04 of 60
	u8 _06[6];
};

/**
 * Main menu screen. Not TModeMainMenu. Draw list 0x3C, two backdrop layers, and
 * three pairs of option layers. draw() steps them. render() draws the two backdrops.
 *
 * @size 0x80
 */
class TAeObjMainMenu : public TAeObject {
public:
	TAeObjMainMenu(TObject* parent, void* data, void* assets);
	virtual ~TAeObjMainMenu();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void render(); // _1C

	DrawTask* mTask;       // _34
	TAeLayer* mBack;       // _38 the backdrop
	TAeLayer* mPage;       // _3C the root file or sound page with the others bound into its slots
	TAeLayer* mPanel[3];   // _40 each file's panel
	TAeLayer* mBar[3];     // _4C each file's emblem bar
	TAeLayer* mFileInfo;   // _58 the chosen file's world lives and emblems
	TAeLayer* mCursor;     // _5C
	TAeLayer* mSoundRow[2]; // _60
	BOOL mReady;           // _68 the page reached the time it plays toward
	f32 _6C;               // _6C
	f32 _70;               // _70
	int mFile;             // _74 the file shown
	u32 _78;               // _78
	u32 _7C;               // _7C
};

extern "C" {
void aeMainMenuShowRoot();                                     // 0x8017E648
void aeMainMenuSnapRoot();                                     // 0x8017E758
void aeMainMenuCloseRoot();                                    // 0x8017E7B8
void aeMainMenuShowFiles();                                    // 0x8017E800
void aeMainMenuFilesToRoot();                                  // 0x8017E894
void aeMainMenuShowSound();                                    // 0x8017E8C4
void aeMainMenuHideSound();                                    // 0x8017E9A8
void aeMainMenuSetFileBar(int file, FileInfo* info);           // 0x8017EA28
void aeMainMenuClearFileBar(int file);                         // 0x8017EB08
void aeMainMenuShowFile(int file, const FileInfo* info, BOOL instant); // 0x8017EB80
void aeMainMenuHidePanels();                                   // 0x8017EDC0
void aeMainMenuSetFileCursor(int file);                        // 0x8017EE30
void aeMainMenuHideFileCursor();                               // 0x8017EED8
void aeMainMenuSetSoundRow(int choice);                        // 0x8017EF44
void aeMainMenuClearSoundRows();                               // 0x8017F0A4
BOOL aeMainMenuIsReady();                                      // 0x8017F168
}

extern "C" {
void freeAeMainMenu();
void loadAeMainMenu();
}

extern "C" {
TAeObjMainMenu* createAeMainMenu();
}

#include "Ninja/njtypes.h"
extern "C" {
extern const char* gTAeObjMainMenuName;
extern TAeObjMainMenu* gAeMainMenu;
extern const u32 gAeMainMenuBanks[8];
extern TArc* sAeMainMenuArc;
extern void* sAeMainMenuAe;
extern NJS_TEXLIST* sAeMainMenuTexList;
extern const u32 gFilePanelSlots[3];
extern const u32 gFileCursorSlots[3];
extern const u32 gFileBarSlots[3];
extern const int gFileWorldIcons[7];
extern const int gFileWorldNames[7];
extern const u32 gSoundRowSlots[2];
extern const u32 gSoundRowLit[2];
extern const u32 gSoundRowDim[2];
}

#endif
