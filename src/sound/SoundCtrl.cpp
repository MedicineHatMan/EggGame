#include "core/types.h"
#include "stage/Stage.h"
#include "sound/SoundCtrl.h"
#include "sound/SoundBank.h"
#include "io/FileReader.h"
#include "camera/TCameraCtrl.h"
#include "core/Mem.h"
#include "sound/TMusicADX.h"
#include "sound/TSoundEffect.h"
#include "sound/TSoundEffect3DCtrl.h"
#include "sound/TSoundEffectAdx.h"
#include "sound/AudioLib.h"

struct SoundVoice;
struct SoundHandle;

struct ChunkFile;

extern "C" {

// 0x800ED784
void soundInit()
{
	loadOwnedResource("SEMapRegular.bin", 'SEMP', &sSEMapFile);
	gSEMap = (SEMap*)sSEMapFile->mData;
	loadOwnedResource("BGMRegular.bin", 'MSMP', &sBGMMapFile);
	gBGMMap = (BGMMap*)sBGMMapFile->mData;

	gSoundEffect            = new TSoundEffect(0);
	gSoundEffect->mTable    = (u8*)gSEMap->mVoices;
	gSoundEffectAdx         = new TSoundEffectAdx(0);
	gSoundEffectAdx->mTable = (u8*)gSEMap->mAdxVoices;
	gMusic                  = new TMusicADX(0);
	gSound3D                = new TSoundEffect3DCtrl(0);
	soundSetMode(1);
}

// 0x800ED898
// 0 = mono 1 = stereo 2 = surround
void soundSetMode(int mode)
{
	if (mode == 0) {
		gcaxSndSetOutput(0);
		gcaxSndSetSurround(0);
		musicServer(1);
	} else if (mode == 1) {
		gcaxSndSetOutput(1);
		gcaxSndSetSurround(0);
		musicServer(0);
	} else if (mode == 2) {
		gcaxSndSetOutput(1);
		gcaxSndSetSurround(1);
		musicServer(0);
	}
	sSoundMode = mode;
	soundSetMix(sSoundMix);
}

// 0x800ED930
void soundSet3DMono(int mono)
{
	if (gSound3D != 0) {
		gSound3D->setMono(mono);
	}
}

// 0x800ED964
void soundMltLoaded(int kind, int, void* bank)
{
	if (kind == 0) {
		((SoundMltBank*)bank)->reset();
	} else if (kind == 1) {
		((SoundMltBank*)bank)->reset();
	}
}

// 0x800ED9A4
void soundMpbLoaded(int kind, int, void* bank, int slot)
{
	if (kind == 0) {
		((SoundMpbBank*)bank)->reset();
		((SoundMpbBank*)bank)->mSlot = slot;
	} else if (kind == 1) {
		((SoundMpbBank*)bank)->reset();
	}
}

// 0x800ED9F8
// The banks every stage shares
void soundLoadBanks()
{
	sAfsWork = memAlloc(0x1E4);
	FileReader::get()->readAdx("afs_se.afs", 0, sAfsWork);
	sMltBank = new SoundMltBank;
	sMpbBank = new SoundMpbBank;
	FileReader::get()->read("egg01.mlt", sMltBank, (FileReadCallback)soundMltLoaded, 0);
	FileReader::get()->read("egg02.mlt", sMltBank, (FileReadCallback)soundMltLoaded, 0);
	FileReader::get()->read("bank06.mpb", sMpbBank, (FileReadCallback)soundMpbLoaded, (void*)6);
}

// 0x800EDAD0
void soundLoadBank(int slot, const char* name)
{
	FileReader::get()->read(name, sMpbBank, (FileReadCallback)soundMpbLoaded, (void*)slot);
}

// 0x800EDB18
// Once a frame the listener follows the camera
void soundUpdate()
{
	Vec3f pos;
	Vec3f dir;

	if (gSound3D != 0) {
		camGetDrawPos(&pos);
		camGetDrawFlat(&dir);
		gSound3D->setListener(&pos, &dir);
		gSound3D->draw();
	}
	if (gSoundEffect != 0) {
		gSoundEffect->draw();
	}
	if (gSoundEffectAdx != 0) {
		gSoundEffectAdx->draw();
	}
	if (gMusic != 0) {
		gMusic->draw();
	}
}

// 0x800EDBC8
void sePauseAll()
{
	if (gSound3D != 0) {
		gSound3D->pauseAll();
	}
	if (gSoundEffect != 0) {
		gSoundEffect->pauseAll();
	}
	if (gSoundEffectAdx != 0) {
		gSoundEffectAdx->pauseAll();
	}
}

// 0x800EDC14
void seResumeAll()
{
	if (gSound3D != 0) {
		gSound3D->resumeAll();
	}
	if (gSoundEffect != 0) {
		gSoundEffect->resumeAll();
	}
	if (gSoundEffectAdx != 0) {
		gSoundEffectAdx->resumeAll();
	}
}

// 0x800EDC60
// Leaves the ADX sounds playing
void sePauseVoices()
{
	if (gSound3D != 0) {
		gSound3D->pauseAll();
	}
	if (gSoundEffect != 0) {
		gSoundEffect->pauseAll();
	}
}

// 0x800EDC9C
void seResumeVoices()
{
	if (gSound3D != 0) {
		gSound3D->resumeAll();
	}
	if (gSoundEffect != 0) {
		gSoundEffect->resumeAll();
	}
}

// 0x800EDCD8
void soundPause()
{
	sePauseAll();
	if (gMusic != 0) {
		gMusic->pause();
	}
}

// 0x800EDD08
void soundResume()
{
	seResumeAll();
	if (gMusic != 0) {
		gMusic->resume();
	}
}

// 0x800EDD38
void seSilenceAll()
{
	if (gSoundEffect != 0) {
		gSoundEffect->silenceAll();
	}
}

// 0x800EDD64
int sePlay(int id)
{
	return sePlayEx(id, 100, 0x7F, 0x40);
}

// 0x800EDD90
int sePlayFor(int id, int frames)
{
	return sePlayExFor(id, 100, 0x7F, 0x40, frames);
}

// 0x800EDDC0
int sePlayEx(int id, int priority, int volume, int pan)
{
	if (gSEMap == 0) {
		return -1;
	}
	u32 index = id - gSEMap->mBase;
	if ((int)index < 0 || index >= gSEMap->mCount) {
		return -1;
	}
	SEMapEntry* entry = &gSEMap->mTable[index];
	switch (entry->mKind) {
	case 0:
		if (gSoundEffect != 0) {
			return gSoundEffect->play(entry->mVoice, priority, 0x78, volume, pan, 0);
		}
		break;
	case 1:
		if (gSoundEffectAdx != 0) {
			return gSoundEffectAdx->play(entry->mVoice, priority, 0x78, volume, pan);
		}
		break;
	}
	return -1;
}

// 0x800EDE90
int sePlayExFor(int id, int priority, int volume, int pan, int frames)
{
	if (gSEMap == 0) {
		return -1;
	}
	u32 index = id - gSEMap->mBase;
	if ((int)index < 0 || index >= gSEMap->mCount) {
		return -1;
	}
	SEMapEntry* entry = &gSEMap->mTable[index];
	switch (entry->mKind) {
	case 0:
		if (gSoundEffect != 0) {
			return gSoundEffect->play(entry->mVoice, priority, frames, volume, pan, 0);
		}
		break;
	case 1:
		if (gSoundEffectAdx != 0) {
			return gSoundEffectAdx->play(entry->mVoice, priority, frames, volume, pan);
		}
		break;
	}
	return -1;
}

// 0x800EDF64
int sePlayOther(int id, int priority, int volume, int pan, int frames)
{
	if (gSEMap == 0) {
		return -1;
	}
	u32 index = id - gSEMap->mBase;
	if ((int)index < 0 || index >= gSEMap->mCount) {
		return -1;
	}
	SEMapEntry* entry = &gSEMap->mTable[index];
	switch (entry->mKind) {
	case 1:
		break;
	case 0:
		if (gSoundEffect != 0) {
			return gSoundEffect->playOther(entry->mVoice, priority, frames, volume, pan, 0);
		}
		break;
	}
	return -1;
}

// 0x800EE014
int sePlayOtherBank(int id, int priority, int volume, int pan, int frames, int bank)
{
	if (gSEMap == 0) {
		return -1;
	}
	u32 index = id - gSEMap->mBase;
	if ((int)index < 0 || index >= gSEMap->mCount) {
		return -1;
	}
	SEMapEntry* entry = &gSEMap->mTable[index];
	switch (entry->mKind) {
	case 1:
		break;
	case 0:
		if (gSoundEffect != 0) {
			return gSoundEffect->playOther(entry->mVoice, priority, frames, volume, pan, bank);
		}
		break;
	}
	return -1;
}

// 0x800EE0D0
void seSetVolume(int serial, int volume)
{
	if (gSoundEffect != 0) {
		gSoundEffect->setVolume(serial, volume);
	}
}

// 0x800EE10C
void seSetPan(int serial, int pan)
{
	if (gSoundEffect != 0) {
		gSoundEffect->setPan(serial, pan);
	}
}

// 0x800EE148
void seSetPitch(int serial, int pitch)
{
	if (gSoundEffect != 0) {
		gSoundEffect->setPitch(serial, pitch);
	}
}

// 0x800EE184
void seStop(int serial)
{
	if (gSoundEffect != 0) {
		gSoundEffect->stop(serial);
	}
	if (gSoundEffectAdx != 0) {
		gSoundEffectAdx->stop(serial);
	}
}

// 0x800EE1D8
void seStopAll()
{
	if (gSoundEffect != 0) {
		gSoundEffect->stopAll();
	}
	if (gSoundEffectAdx != 0) {
		gSoundEffectAdx->stopAll();
	}
}

// 0x800EE214
int seIsPlaying(int serial)
{
	if (gSoundEffect != 0) {
		return gSoundEffect->isPlaying(serial);
	}
	return 0;
}

// 0x800EE250
void bgmPlay(int id)
{
	if (gBGMMap == 0) {
		return;
	}
	int index = id - gBGMMap->mBase;
	if (index < 0 || index >= gBGMMap->mCount) {
		return;
	}
	BGMMapEntry* entry = &gBGMMap->mTable[index];
	switch (entry->mKind) {
	case 0:
		if (gMusic != 0) {
			gMusic->play(entry->mInfo->mFile);
		}
		break;
	}
}

// 0x800EE2C8
void bgmStop()
{
	if (gMusic != 0) {
		gMusic->stop();
	}
}

// 0x800EE2F4
void bgmFadeTo(f32 volume, u16 frames)
{
	if (gMusic != 0) {
		gMusic->fadeTo(frames, volume);
	}
}

// 0x800EE328
void bgmSetVolume(f32 volume)
{
	if (gMusic != 0) {
		gMusic->applyVolume(volume);
	}
}
}
