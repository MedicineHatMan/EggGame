#ifndef _MEMCARD_H
#define _MEMCARD_H

#include "core/types.h"
#include "Dolphin/card.h"

struct SaveData;

/**
 * The comment, banner and icons at the front of the save file. The names are ours.
 * The images come from the 0x2000 block at gCardImages: an RGB5A3 banner (0x1800)
 * or a C8 one (0xC00 and a 0x200 palette), then the icons (0x800 each, or 0x400 and a palette).
 *
 * @size 0x30
 */
struct CardImage {
	CardImage(void* banner, void* bannerTlut, void* icons, void* iconTlut, int iconCount, int anim, int speed, const char* title,
	          const char* comment); // 0x801A27B4
	~CardImage();                   // 0x801A2914

	int getSize();      // 0x801A2970
	char* getComment(); // 0x801A2978 the second comment line
	int build(u8* dst); // 0x801A2994 returns the size
	int bannerFormat(); // 0x801A2AAC
	int iconFormat();   // 0x801A2AD4
	int iconSpeed();    // 0x801A2AE8
	int animMode();     // 0x801A2B18

	int mSize;         // _00 comment banner and icons
	char* mComment;    // _04 0x40 bytes the title then the second line
	void* mBanner;     // _08
	void* mBannerTlut; // _0C 0 for RGB5A3
	void* mIcons;      // _10 0 draws a placeholder
	void* mIconTlut;   // _14 0 for RGB5A3
	int mCommentAddr;  // _18
	int mIconAddr;     // _1C
	int mIconData;     // _20 where the icons start
	int mIconCount;    // _24 1 to 8
	int mAnim;         // _28 1 bounces
	int mSpeed;        // _2C
};

/**
 * The CARD work area and the open file. The names are ours.
 * @size 0xA8
 */
struct CardBuf {
	CardBuf();  // 0x801A0EFC
	~CardBuf(); // 0x801A0F60

	u8* mWork;          // _00 0xA000 for CARDMount
	char mName[32];     // _04
	CARDFileInfo mInfo; // _24
	int mFileNo;        // _38
	CARDStat mStat;     // _3C
};

/**
 * Runs one CARD request at a time as a chain of async steps. The names are ours.
 * mState is what the last probe saw (0 ready, 3 none, 4 wrong device, 5 broken, 6 bad sector size).
 * mResult is how the last request ended (0 done, 1 running, else an error kind).
 *
 * @size 0x38
 */
struct CardWorker {
	CardWorker(int chan); // 0x801A1018
	~CardWorker();        // 0x801A10C0

	int getState();                                                            // 0x801A1124
	int isBusy();                                                              // 0x801A112C
	int getResult();                                                           // 0x801A1134
	int hasRoom(int size);                                                     // 0x801A113C
	int begin(int kind);                                                       // 0x801A1164
	int requestFormat();                                                       // 0x801A11C8 kind 1
	int requestCreate(const char* name, int size, u8* data, CardImage* image); // 0x801A1218 kind 2
	int requestDelete(const char* name);                                       // 0x801A1298 kind 3
	int requestRead(const char* name, int size, u8* data);                     // 0x801A1300 kind 4
	int requestWrite(const char* name, int size, u8* data);                    // 0x801A1378 kind 5
	int requestFree();                                                         // 0x801A13F0 kind 6

	int startStep(int step);                    // 0x801A1440
	void pollStep(int step);                    // 0x801A1504
	void update();                              // 0x801A15AC
	s32 probe(s32* sectorSize);                 // 0x801A1618
	void close();                               // 0x801A1670
	void goStep(int step);                      // 0x801A16CC
	void setStepPending(int step);              // 0x801A1720
	void setStepStarted(int step);              // 0x801A1730
	int probeState(s32 result, s32 sectorSize); // 0x801A1740
	int errorKind(s32 result);                  // 0x801A17B4
	s32 probeCard();                            // 0x801A183C
	void finishOk();                            // 0x801A1888
	void finishFail();                          // 0x801A18D0
	void fail(s32 result);                      // 0x801A1914
	int cardGone();                             // 0x801A194C

	void pollProbe();     // 0x801A199C step 0
	void pollReady();     // 0x801A19F4 step 1
	void pollEmpty();     // 0x801A1A34 step 2
	int startMount();     // 0x801A1A74 step 3
	void pollMount();     // 0x801A1AB8
	int startCheck();     // 0x801A1B8C step 4
	void pollCheck();     // 0x801A1BC4
	int startOpen();      // 0x801A1CF0 step 5
	void pollOpen();      // 0x801A1D60
	int startStatus();    // 0x801A1E48 step 6
	void pollStatus();    // 0x801A1EB8
	int startFormat();    // 0x801A1FE4 step 7
	void pollFormat();    // 0x801A201C
	int startCreate();    // 0x801A20A0 step 8
	void pollCreate();    // 0x801A20EC
	int startDelete();    // 0x801A21CC step 9
	void pollDelete();    // 0x801A220C
	int startRead();      // 0x801A2290 step 10
	void pollRead();      // 0x801A22D8
	int startWrite();     // 0x801A235C step 11
	void pollWrite();     // 0x801A23A4
	int startFree();      // 0x801A250C step 12
	void pollFree();      // 0x801A2514
	int startSetStatus(); // 0x801A25B0 step 13
	void pollSetStatus(); // 0x801A25F4

	CardBuf* mBuf;     // _00
	int mChan;         // _04
	int mState;        // _08
	int mStep;         // _0C
	int mStarted;      // _10
	int mBusy;         // _14
	int mRequest;      // _18
	int mResult;       // _1C
	u32 mFlags;        // _20 1 mounted 2 open 4 status read
	int mSize;         // _24
	u8* mData;         // _28
	CardImage* mImage; // _2C
	s32 mFreeBytes;    // _30
	s32 mFreeFiles;    // _34
};

/**
 * The save flow the menus drive: probe, load, create, format, delete and write, as 20 states.
 * The names are ours. Each state has a run handler (event 0 enter, 1 update, 2 exit) and some
 * also answer requests (event 3 asks, 4 acts).
 *
 * @size 0x34
 */
struct CardManager {
	CardManager();  // 0x8019F80C
	~CardManager(); // 0x8019F910

	void writeComment();                   // 0x8019F9B8
	int getState();                        // 0x8019FB0C
	int resultIs8();                       // 0x8019FB14
	int cardStateIs6();                    // 0x8019FB44
	void update();                         // 0x8019FB74
	u8 request(int want);                  // 0x8019FC04
	void setState(int state);              // 0x8019FC58
	void runState(int state, int event);   // 0x8019FCB4
	int handleState(int state, int event); // 0x8019FDDC
	void syncWorker();                     // 0x8019FE9C
	int checkCard();                       // 0x8019FEDC
	int checkResult();                     // 0x8019FF84

	void runNoCard(int event);      // 0x801A0028 state 0
	void runWrongDevice(int event); // 0x801A007C state 1
	void runBroken(int event);      // 0x801A00D0 state 2
	int askBroken(int event);       // 0x801A0104
	void runError(int event);       // 0x801A0150 state 3
	void runNoSpace(int event);     // 0x801A0184 state 4
	void runNoFile(int event);      // 0x801A01B8 state 5
	int askNoFile(int event);       // 0x801A01EC
	void runCorrupt(int event);     // 0x801A0238 state 6
	int askCorrupt(int event);      // 0x801A026C
	void runOtherSave(int event);   // 0x801A02B8 state 7
	void runFreeCheck(int event);   // 0x801A02EC state 13
	void runCrcCheck(int event);    // 0x801A03DC state 14
	void runFormat(int event);      // 0x801A0464 state 8
	void runCreate(int event);      // 0x801A0520 state 9
	void runDelete(int event);      // 0x801A0618 state 10
	void runLoad(int event);        // 0x801A06D8 state 11
	void runSave(int event);        // 0x801A07BC state 12
	void runFormatted(int event);   // 0x801A08F0 state 15
	int askFormatted(int event);    // 0x801A0924
	void runCreated(int event);     // 0x801A0970 state 16
	int askCreated(int event);      // 0x801A09A4
	void runDeleted(int event);     // 0x801A09F0 state 17
	int askDeleted(int event);      // 0x801A0A24
	void runReady(int event);       // 0x801A0A70 state 18
	int askReady(int event);        // 0x801A0AA4
	void runSaved(int event);       // 0x801A0AF0 state 19
	int askSaved(int event);        // 0x801A0B24

	CardImage* mImage;     // _00
	CardWorker* mWorker;   // _04
	int mBusy;             // _08 the worker's each frame
	int mResult;           // _0C the worker's each frame
	const char* mFileName; // _10
	int mSize;             // _14 the whole file rounded to 0x2000
	u8* mData;             // _18
	SaveData* mSave;       // _1C in mData after the image
	int mSaveSize;         // _20 0x728
	int mState;            // _24
	int mPhase;            // _28
	int mWant;             // _2C the pending request
	int mChanged;          // _30 the state changed this frame
};

extern "C" {
CardImage* createCardImage(int bannerKind, int iconKind, int iconCount, int anim, int speed, u8* images, const char* title,
                           const char* comment); // 0x801A2678
CardImage* newCardImage(void* banner, void* bannerTlut, void* icons, void* iconTlut, int iconCount, int anim, int speed, const char* title,
                        const char* comment); // 0x801A272C
CardWorker* createCardWorker(int chan);       // 0x801A0FBC

void cardInit();        // 0x801A0B70
void cardShutdown();    // 0x801A0B7C
void cardCreate();      // 0x801A0BB4
void cardUpdate();      // 0x801A0BF8
int cardGetState();     // 0x801A0C24
int cardResultIs8();    // 0x801A0C58
int cardStateIs6();     // 0x801A0C8C
int cardIsWriting();    // 0x801A0CC0 states 8 9 10 and 12
u8 cardRequestCreate(); // 0x801A0D0C
u8 cardRequestFormat(); // 0x801A0D44
u8 cardRequestDelete(); // 0x801A0D7C
u8 cardConfirm();       // 0x801A0DB4 request 5
u8 cardConfirm2();      // 0x801A0DEC request 5
u8 cardConfirm3();      // 0x801A0E24 request 5
u8 cardRequestSave();   // 0x801A0E5C
u8 cardConfirm4();      // 0x801A0E94 request 5
void cardLoadSave();    // 0x801A0ECC

extern CardManager* gCardManager; // gCardManager
}

#endif
