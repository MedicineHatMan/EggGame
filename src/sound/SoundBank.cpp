#include "sound/SoundBank.h"
#include "sound/SoundCtrl.h"
#include "core/Mem.h"
#include "sound/AudioLib.h"

s8 sSoundMixTable[2][3][2] = {
	{ { -4, 14 }, { 0, 15 }, { 0, 15 } },
	{ { -4, 14 }, { 0, 15 }, { -3, 15 } },
};

// 0x800EE5BC
void soundSetMix(int mix)
{
	sSoundMix = mix;
	bgmSetVolume((f32)(sSoundMixTable[mix][sSoundMode][0] + 100) / 100.0f);
	gcaxSndSetMasterLevel((s8)sSoundMixTable[mix][sSoundMode][1]);
}

// 0x800EE654
SoundMltBank::SoundMltBank()
{
	mBuffer = 0;
	mSize   = 0;
	mState  = 0;
}

// 0x800EE6A0
SoundMltBank::~SoundMltBank()
{
	if (mBuffer != 0) {
		memFree(mBuffer);
	}
}

// 0x800EE70C
int SoundMltBank::load(TFile* file)
{
	int handle;
	switch (mState) {
	case 0:
		mSize   = file->getSize();
		mBuffer = memAlloc(mSize);
		mState  = 1;
	case 1:
		if (file->read(mBuffer, mSize) <= 0) {
			break;
		}
		gcaxSndReverbOff();
		gcaxLoaderAlloc(&handle);
		gcaxLoaderSet(handle, mBuffer, mSize, -1, 0);
		gcaxLoaderLoad(handle);
		gcaxLoaderFree(handle);
		gcaxSndReverbOn(0, 0);
		memFree(mBuffer);
		mBuffer = 0;
		mState  = 2;
		return 1;
	case 2:
		return 1;
	}
	return 0;
}

// 0x800EE81C
int SoundMltBank::getState()
{
	switch (mState) {
	case 0:
	case 1:
		return 0;
	case 2:
		return 1;
	}
	return 2;
}

// 0x800EE850
void SoundMltBank::reset()
{
	if (mBuffer != 0) {
		memFree(mBuffer);
		mBuffer = 0;
	}
	mState = 0;
}

// 0x800EE898
SoundMpbBank::SoundMpbBank()
{
	mBuffer = 0;
	mLeft   = 0;
	mState  = 0;
	mBuffer = memAlloc(0x10000);
}

// 0x800EE8F0
SoundMpbBank::~SoundMpbBank()
{
	if (mBuffer != 0) {
		memFree(mBuffer);
	}
}

// 0x800EE95C
int SoundMpbBank::load(TFile* file)
{
	switch (mState) {
	case 0:
		mLeft  = file->getSize();
		mChunk = mLeft < 0x10000 ? mLeft : 0x10000;
		mState = 1;
	case 1:
		if (file->read(mBuffer, mChunk) > 0) {
			gcaxSndSendBank(mSlot, mBuffer, (mChunk + 0x1F) & ~0x1F);
			mLeft -= mChunk;
			if (mLeft > 0) {
				mChunk = mLeft < 0x10000 ? mLeft : 0x10000;
			} else {
				mState = 2;
			}
		}
		break;
	case 2:
		return 1;
	}
	return 0;
}

// 0x800EEA78
int SoundMpbBank::getState()
{
	switch (mState) {
	case 0:
	case 1:
		return 0;
	case 2:
		return 1;
	}
	return 2;
}

// 0x800EEAAC
void SoundMpbBank::reset()
{
	mState = 0;
}
