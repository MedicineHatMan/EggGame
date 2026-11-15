#ifndef _SOUND_TSOUNDEFFECT_H_
#define _SOUND_TSOUNDEFFECT_H_
#include "Ninja/ninja.h"
#include "engine/dlist.h"
#include "stage/Stage.h"
struct SoundList;
struct SoundNode;
struct SoundPool;
struct SoundVoice;
struct TSoundEffect;

#include "core/types.h"

extern "C" {
void sfxMidiNoteOff(u32, int, s16, s8);
void sfxMidiNoteOn(u32, int, s16, s8);
void sfxMidiPan(u32, int, int);
void sfxMidiPanB(u32, int, int);
void sfxMidiPitch(u32, u8, s16);
void sfxMidiProgram(u32, int, s8, s8);
void sfxMidiResetChannel(u32, int);
void sfxMidiVolume(u32, int, int);
}

/**
 * What a voice is doing.
 *
 * @size 0x14
 */
struct SoundVoice {
	u8 mFlags;    // _00 0x80 volume 0x40 pan 0x20 other pan 0x10 paused 8 pitch 4 start
	u8 mPriority; // _01
	s8 mPort;     // _02
	u8 mChannel;  // _03
	u8 mVolume;   // _04
	u8 mNote;     // _05
	u8 mPan;      // _06
	s16 mPitch;   // _08
	u16 mFrames;  // _0A left to play 0 for no limit
	int mSerial;  // _0C
	u16 mBank;    // _10
	u16 mProgram; // _12
};

/**
 * @size 0x1C
 */
struct SoundNode {
	SoundNode& operator=(const SoundNode& other);

	SoundNode* mNext;  // _00
	SoundNode* mPrev;  // _04
	SoundVoice mVoice; // _08
};

/**
 * @size 0xC
 */
struct SoundList {
	void insertBefore(SoundNode* at, SoundNode* node);
	void append(SoundNode* node);
	void remove(SoundNode* node);

	SoundNode* mHead; // _00
	SoundNode* mTail; // _04
	int mCount;       // _08
};

/**
 * @size 0xC
 */
struct SoundPool {
	SoundPool();
	~SoundPool();

	SoundNode* alloc();
	void free(SoundNode* node);
	void takeBack(SoundList* list);
	void init(int count);

	SoundNode* mNodes; // _00
	SoundNode* mFree;  // _04
	int mCount;        // _08
};

/**
 * @size 0x58
 */
class TSoundEffect : public TObject {
public:
	TSoundEffect(TObject* parent);
	virtual ~TSoundEffect();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	int play(int id, int priority, int frames, int volume, int pan, int bank);
	int playOther(int id, int priority, int frames, int volume, int pan, int bank);
	void setPan(int serial, int pan);
	void setPitch(int serial, int pitch);
	void setVolume(int serial, int volume);
	void stop(int serial);
	void stopAll();
	int isPlaying(int serial);
	SoundVoice* allocVoice(int priority);
	SoundVoice* findVoice(int serial);
	SoundVoice* findVoice2(int serial);
	void pauseAll();
	void resumeAll();
	void silenceAll();
	int isWaiting(int bank, int program);
	void stopVoice(SoundVoice* voice);

	u32 mPorts[2];      // _20
	u8* mTable;         // _28
	SoundPool mPool;    // _2C
	SoundList mWaiting; // _38
	SoundList mPlaying; // _44
	int mFrame;         // _50
	int mSerial;        // _54
};

extern "C" {
extern const char* gTSoundEffectName;
}

#endif
