#ifndef _SOUND_TSOUNDEFFECTADX_H
#define _SOUND_TSOUNDEFFECTADX_H
#include "Ninja/ninja.h"
#include "stage/Stage.h"
struct AdxVoice;
struct TSoundEffectAdx;

#include "core/types.h"

/**
 * @size 0x14
 */
struct AdxVoice {
	AdxVoice* mPrev; // _00
	AdxVoice* mNext; // _04
	u8 mFlags;       // _08 0x80 and 0x40 only cleared 0x10 paused
	u8 mPriority;    // _09
	u8 mSlot;        // _0A
	u8 mVolume;      // _0B
	u16 mFrames;     // _0C left to play 0 for no limit
	int mSerial;     // _10
};

/**
 * @size 0x38
 */
class TSoundEffectAdx : public TObject {
public:
	TSoundEffectAdx(TObject* parent);
	virtual ~TSoundEffectAdx();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	int play(int id, int priority, int frames, int volume, int pan);
	void stop(int serial);
	void stopAll();
	void pauseAll();
	void resumeAll();
	AdxVoice* allocVoice(int priority);
	void freeVoice(AdxVoice* voice);

	u8* mTable;        // _20 the file number of each sound
	void* mPlayers[1]; // _24
	void* mWork;       // _28
	AdxVoice* mPlaying; // _2C
	AdxVoice* mFree;   // _30
	AdxVoice* mVoices; // _34
};

extern "C" {
extern const char* gTSoundEffectAdxName;
}

#endif
