#ifndef _MAIN_H
#define _MAIN_H

#include "core/types.h"

class TGameFlow; // GameFlow.h

/**
 * @size 0x30
 */
class CMain {
public:
	CMain(int argc, char** argv);
	~CMain();

	void run();
	int checkReset();
	void parseArgs(int argc, char** argv);
	void initSystem();
	void initGame();
	int isProgressiveRequested();
	void hotReset(int forceMenu);
	void restart();
	int checkDisc();
	void showFatalError();
	void showCoverOpen();
	void showNoDisc();
	void showWrongDisc();
	void showReadError();
	void drawMessage(const char* message);

	int mSSArg;                      // _00 set by the "-ss" command line argument
	void* mBuffer600;                // _04 0x600 bytes
	void* mBuffer1800;               // _08 0x1800 bytes
	void* mBuffer3600;               // _0C 0x3600 bytes
	void* mBuffer5A00;               // _10 0x5A00 bytes
	struct TextTexture* mTextObject; // _14
	TGameFlow* mFlow;                // _18
	int mPrevResetButton;            // _1C
	int mPrevPadReset;               // _20
	int _24;                         // _24
	int mHasDiscError;               // _28
	int _2C;                         // _2C
};

#include "Dolphin/GX/GXTypes.h"
extern "C" {
extern u32 sARStack[];
extern int sNoRestart;
extern const GXColor smainClearColor;
}

#endif
