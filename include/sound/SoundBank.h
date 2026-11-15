#ifndef _SOUNDBANK_H
#define _SOUNDBANK_H

#include "core/types.h"
#include "io/Resource.h"

/**
 * @size 0x38
 */
class SoundMltBank : public TResource {
public:
	SoundMltBank();
	virtual ~SoundMltBank();
	virtual int load(TFile* file);
	virtual int getState();

	void reset();

	u32 _28;       // _28
	void* mBuffer; // _2C
	int mSize;     // _30
	int mState;    // _34 0 = new 1 = reading 2 = done
};

/**
 * A .mpb bank: read 64 KB at a time and sent on to sound memory.
 * @size 0x40
 */
class SoundMpbBank : public TResource {
public:
	SoundMpbBank();
	virtual ~SoundMpbBank();
	virtual int load(TFile* file);
	virtual int getState();

	void reset();

	u32 _28;       // _28
	void* mBuffer; // _2C 0x10000 bytes
	s32 mLeft;     // _30 long the clamp's int limit is converted and copied into this value's register
	s32 mChunk;    // _34
	int mSlot;     // _38 set by soundMpbLoaded
	int mState;    // _3C 0 = new 1 = reading 2 = done
};

extern "C" void soundSetMix(int mix);

extern "C" {
extern s8 sSoundMixTable[2][3][2];
}

#endif
