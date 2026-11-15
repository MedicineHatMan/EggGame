#ifndef _SOUNDCTRL_H
#define _SOUNDCTRL_H
struct ChunkFile;
struct SoundMltBank;
struct SoundMpbBank;
struct TMusicADX;
struct TSoundEffect;
struct TSoundEffectAdx;

#include "core/types.h"

// The sound tables that SoundCtrl.cpp loads and the one 3D controller

/**
 * One entry of SEMapRegular.bin.
 *
 * @size 0x4
 */
struct SEMapEntry {
	u16 mKind;  // _00 0 for a plain sound 1 for an ADX one
	u16 mVoice; // _02
};

/**
 * The head of SEMapRegular.bin.
 *
 * @size 0x18
 */
struct SEMap {
	u32 _00;            // _00
	int mBase;          // _04 the first id
	u32 mCount;         // _08
	SEMapEntry* mTable; // _0C
	u32 mVoices;        // _10 handed to TSoundEffect
	u32 mAdxVoices;     // _14 handed to TSoundEffectAdx
};

struct BGMInfo {
	const char* mFile; // _00
};

/** @size 0x8 */
struct BGMMapEntry {
	int mKind;      // _00 0 for a stream
	BGMInfo* mInfo; // _04
};

/** The head of BGMRegular.bin. @size 0x10 */
struct BGMMap {
	u32 _00;             // _00
	int mBase;           // _04 the first id
	int mCount;          // _08
	BGMMapEntry* mTable; // _0C
};

class TSoundEffect3DCtrl;

extern "C" {
extern TSoundEffect3DCtrl* gSound3D; // 0x802C5374
extern SEMap* gSEMap;                // 0x802C5378
}

extern "C" {
void bgmSetVolume(f32);
void bgmStop();
int seIsPlaying(int);
void sePauseVoices();
int sePlayEx(int, int, int, int);
int sePlayExFor(int, int, int, int, int);
int sePlayOther(int, int, int, int, int);
int sePlayOtherBank(int, int, int, int, int, int);
void seResumeVoices();
void seSetPan(int, int);
void seSetPitch(int, int);
void seSetVolume(int, int);
void seSilenceAll();
void seStopAll();
void soundInit();
void soundLoadBank(int, const char*);
void soundLoadBanks();
void soundPause();
void soundResume();
void soundSet3DMono(int);
void soundSetMode(int);
void soundUpdate();
}

extern "C" {
int sePlayFor(int, int);
int sePlay(int);
void bgmFadeTo(f32, u16);
void seStop(int);
}

extern "C" {
void bgmPlay(int);
}

extern "C" {
extern int sSoundMode;
extern int sSoundMix;
extern ChunkFile* sSEMapFile;
extern ChunkFile* sBGMMapFile;
extern TSoundEffect* gSoundEffect;
extern TMusicADX* gMusic;
extern TSoundEffectAdx* gSoundEffectAdx;
extern BGMMap* gBGMMap;
extern void* sAfsWork;
extern SoundMltBank* sMltBank;
extern SoundMpbBank* sMpbBank;
}

#endif
