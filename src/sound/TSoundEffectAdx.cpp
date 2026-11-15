#include "enemy/boss/EneBoss.h"
#include "core/dolphin.h"
#include "core/Mem.h"
#include "sound/TSoundEffectAdx.h"
#include "sound/AudioLib.h"

typedef char AdxVoice_size_check[(sizeof(AdxVoice) == 0x14) ? 1 : -1];

typedef char TSoundEffectAdx_size_check[(sizeof(TSoundEffectAdx) == 0x38) ? 1 : -1];

const char* gTSoundEffectAdxName = "TSoundEffectAdx";

// 0x80142328
TSoundEffectAdx::TSoundEffectAdx(TObject* parent)
    : TObject(parent)
{
	_00    = (u32)gTSoundEffectAdxName;
	mSize  = 0x38;
	mTable = 0;
	for (int i = 0; i < 1; i++) {
		mWork       = memAlloc(0x311E4);
		mPlayers[i] = ADXT_Create(2, mWork, 0x311E4);
	}
	mVoices = (AdxVoice*)memAlloc(0x14);
	for (int i = 0; i < 1; i++) {
		mVoices[i].mSlot = i;
	}
	mPlaying = 0;
	mFree    = mVoices;
	for (int i = 0; i < 0; i++) {
		mFree[i].mPrev = 0;
		mFree[i].mNext = &mFree[i + 1];
	}
	mFree[0].mPrev = 0;
	mFree[0].mNext = 0;
}

// 0x80142418
TSoundEffectAdx::~TSoundEffectAdx()
{
	memFree(mWork);
	ADXT_Destroy(mPlayers[0]);
	memFree(mVoices);
}

// 0x80142490
int TSoundEffectAdx::play(int id, int priority, int frames, int volume, int pan)
{
	AdxVoice* voice = allocVoice(priority);
	if (voice == 0) {
		return -1;
	}
	voice->mFlags    = 0;
	voice->mPriority = priority;
	voice->mVolume   = volume;
	voice->mFrames   = frames;
	voice->mSerial   = -2;
	void* player     = mPlayers[voice->mSlot];
	ADXT_StartAfs(player, 0, mTable[id]);
	ADXT_SetOutVol(player, 0);
	ADXT_Pause(player, 0);
	return voice->mSerial;
}

// 0x8014253C
void TSoundEffectAdx::stop(int serial)
{
	AdxVoice* voice = mPlaying;
	while (voice != 0) {
		AdxVoice* next = voice->mNext;
		if ((u32)voice->mSerial == serial) {
			ADXT_Stop(mPlayers[voice->mSlot]);
			freeVoice(voice);
		}
		voice = next;
	}
}

// 0x801425AC
void TSoundEffectAdx::stopAll()
{
	AdxVoice* voice = mPlaying;
	while (voice != 0) {
		AdxVoice* next = voice->mNext;
		ADXT_Stop(mPlayers[voice->mSlot]);
		freeVoice(voice);
		voice = next;
	}
}

// 0x8014260C
void TSoundEffectAdx::pauseAll()
{
	for (AdxVoice* voice = mPlaying; voice != 0; voice = voice->mNext) {
		voice->mFlags |= 0x10;
		ADXT_Pause(mPlayers[voice->mSlot], 1);
	}
}

// 0x8014266C
void TSoundEffectAdx::resumeAll()
{
	for (AdxVoice* voice = mPlaying; voice != 0; voice = voice->mNext) {
		if (voice->mFlags & 0x10) {
			voice->mFlags &= ~0x10;
			ADXT_Pause(mPlayers[voice->mSlot], 0);
		}
	}
}

// 0x801426D4
void TSoundEffectAdx::draw()
{
	AdxVoice* voice;
	AdxVoice* next;
	void* player;
	if (mPlaying != 0) {
		int status = DVDGetDriveStatus();
		if (status == -1 || (u32)(status - 4) <= 2 || status == 0xB) {
			voice = mPlaying;
			while (voice != 0) {
				next = voice->mNext;
				player = mPlayers[voice->mSlot];
				ADXT_Stop(player);
				ADXT_ClearErrCode(player);
				freeVoice(voice);
				voice = next;
			}
		}
	}

	voice = mPlaying;
	while (voice != 0) {
		next = voice->mNext;
		if (voice->mFlags & 0x80) {
			voice->mFlags &= 0x7F;
		}
		if (voice->mFlags & 0x40) {
			voice->mFlags &= ~0x40;
		}
		player = mPlayers[voice->mSlot];
		int stat     = ADXT_GetStat(player);
		if (stat == 5 || stat == 0) {
			freeVoice(voice);
		} else if (stat == 6) {
			ADXT_Stop(player);
			ADXT_ClearErrCode(player);
			freeVoice(voice);
		} else if (voice->mFrames != 0) {
			if (--voice->mFrames == 0) {
				ADXT_Stop(player);
				freeVoice(voice);
			}
		}
		voice = next;
	}
}

// 0x8014283C
AdxVoice* TSoundEffectAdx::allocVoice(int priority)
{
	AdxVoice* voice;
	if (mFree != 0) {
		voice = mFree;
		mFree = voice->mNext;
	} else {
		AdxVoice* victim = mPlaying;
		if (victim->mPriority <= (u8)priority) {
			ADXT_Stop(mPlayers[victim->mSlot]);
			mPlaying             = victim->mNext;
			victim->mPrev->mNext = 0;
			if (victim->mNext != 0) {
				victim->mNext->mPrev = victim->mPrev;
			}
			voice = victim;
		} else {
			return 0;
		}
	}

	AdxVoice* head = mPlaying;
	AdxVoice* at   = head;
	if (head != 0) {
		while (at != 0 && at->mPriority <= (u8)priority) {
			at = at->mNext;
		}
		if (at != 0) {
			voice->mPrev = at->mPrev;
			voice->mNext = at;
			if (mPlaying == at) {
				mPlaying = voice;
			} else {
				at->mPrev->mNext = voice;
			}
			at->mPrev = voice;
		} else {
			voice->mPrev       = head->mPrev;
			voice->mNext       = 0;
			head->mPrev->mNext = voice;
			head->mPrev        = voice;
		}
	} else {
		voice->mPrev = voice;
		mPlaying     = voice;
		voice->mNext = 0;
	}
	return voice;
}

// 0x80142978
void TSoundEffectAdx::freeVoice(AdxVoice* voice)
{
	if (voice->mPrev == voice) {
		mPlaying = 0;
	} else if (mPlaying == voice) {
		mPlaying = voice->mNext;
		if (voice->mNext != 0) {
			voice->mNext->mPrev = voice->mPrev;
		}
	} else {
		voice->mPrev->mNext = voice->mNext;
		if (voice->mNext != 0) {
			voice->mNext->mPrev = voice->mPrev;
		} else {
			mPlaying->mPrev = voice->mPrev;
		}
	}
	voice->mNext = mFree;
	mFree        = voice;
}
