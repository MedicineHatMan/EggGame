#ifndef _CORE_TPERIVIBRATE_H_
#define _CORE_TPERIVIBRATE_H_
struct VibeRecord;
#include "stage/Stage.h"
struct VibePattern;
struct VibePort;
struct TPeriVibrate;

#include "core/types.h"

extern "C" {
void periVibratePlay(int, int);
}

extern "C" {
TPeriVibrate* periVibrateCreate();
}

/**
 * @size 0x24
 */
struct VibePattern {
	u16 mValueA;     // _00
	u16 mValueB;     // _02
	u8 mSteps[0x20]; // _04
};

/**
 * @size 0x10
 */
struct VibePort {
	int mOn;      // _00
	int mPattern; // _04
	int mStep;    // _08
	int mTime;    // _0C
};

/**
 * @size 0x500
 */
class TPeriVibrate : public TObject {
public:
	TPeriVibrate(TObject* parent);
	virtual ~TPeriVibrate();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	void play(int port, int pattern);

	u8 _20[0x20];                // _20
	VibePattern mPatterns[0x20]; // _40
	VibePort mPorts[4];          // _4C0
};

/**
 * As it is in the table.
 *
 * @size 0xC
 */
struct VibeRecord {
	u32 mHeader;  // _00
	u32 mBits[2]; // _04 16 steps of 2 bits each
};

extern "C" {
extern const char* gTPeriVibrateName;
extern TPeriVibrate* sPeriVibrateObj;
extern VibeRecord sVibePatterns[];
}

#endif
