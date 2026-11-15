#include "enemy/boss/EneBoss.h"
#include "core/dolphin.h"
#include "core/Mem.h"
#include "io/NinjaFile.h"
#include "sound/TMusicADX.h"
#include "sound/AudioLib.h"

const char* gTMusicADXName = "TMusicADX";

typedef char TMusicADX_size_check[(sizeof(TMusicADX) == 0x54) ? 1 : -1];

// 0x80031B30
TMusicADX::TMusicADX(TObject* parent)
    : TObject(parent)
{
	_00     = (u32)gTMusicADXName;
	mSize   = 0x54;
	mWork   = memAlloc(0x311E4);
	mPlayer = ADXT_Create(2, mWork, 0x311E4);
	mTarget = 1.0f;
	mVolume = 1.0f;
	mFade   = 0;
	mState  = 3;
}

// 0x80031BC0
TMusicADX::~TMusicADX()
{
	memFree(mWork);
	ADXT_Destroy(mPlayer);
}

// 0x80031C30
void TMusicADX::draw()
{
	if (mState == 1) {
		int status = DVDGetDriveStatus();
		if (status == 0 || status == 8) {
			if (mName[0] != 0) {
				ADXT_StartFname(mPlayer, mName);
				mState = 0;
			}
		}
	} else if (mState == 0) {
		int status = DVDGetDriveStatus();
		if (status == -1) {
			mState = 2;
			ADXT_Stop(mPlayer);
		} else if ((u32)(status - 4) <= 2 || status == 0xB) {
			mState = 1;
			ADXT_Stop(mPlayer);
		}
	}
	if (mState == 0 && ADXT_GetStat(mPlayer) == 6) {
		ADXT_Stop(mPlayer);
		ADXT_ClearErrCode(mPlayer);
	}
	if (mFade != 0) {
		mVolume += (mTarget - mVolume) / (f32)mFade;
		applyVolume(mVolume);
		mFade--;
	}
}

// 0x80031D68
void TMusicADX::play(const char* name)
{
	resNameSet((ResName*)mName, name);
	ADXT_Pause(mPlayer, 0);
	ADXT_StartFname(mPlayer, name);
	mState = 0;
}

// 0x80031DBC
void TMusicADX::stop()
{
	mName[0] = 0;
	ADXT_Stop(mPlayer);
	mState = 3;
}

// 0x80031DFC
void TMusicADX::applyVolume(f32 volume)
{
	int out;
	if (volume <= 0.0f) {
		out = -1000;
	} else if (volume >= 1.0f) {
		out = 0;
	} else {
		out = (int)(1000.0f * volume) - 1000;
	}
	ADXT_SetOutVol(mPlayer, out);
}

// 0x80031E68
void TMusicADX::fadeTo(u16 frames, f32 volume)
{
	mFade   = frames;
	mTarget = volume;
	if (frames == 0) {
		mVolume = mTarget;
		applyVolume(mVolume);
	}
}

// 0x80031EA4
void TMusicADX::pause()
{
	ADXT_Pause(mPlayer, 1);
}

// 0x80031ECC
void TMusicADX::resume()
{
	ADXT_Pause(mPlayer, 0);
}

// 0x80031EF4
extern "C" void musicServer(int mode)
{
	ADXT_SetDecodeSteAsMonoSw();
}
