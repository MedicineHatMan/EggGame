#include "enemy/boss/EneBoss.h"
#include "sound/TSoundEffect.h"
#include "sound/AudioLib.h"

typedef char SoundNode_size_check[(sizeof(SoundNode) == 0x1C) ? 1 : -1];

typedef char TSoundEffect_size_check[(sizeof(TSoundEffect) == 0x58) ? 1 : -1];

const char* gTSoundEffectName = "TSoundEffect";

// 0x80030B60
TSoundEffect::TSoundEffect(TObject* parent)
    : TObject(parent), mTable(0)
{
	mWaiting.mHead  = 0;
	mWaiting.mTail  = 0;
	mWaiting.mCount = 0;
	mPlaying.mHead  = 0;
	mPlaying.mTail  = 0;
	mPlaying.mCount = 0;
	mFrame  = 0;
	mSerial = 0;
	_00     = (u32)gTSoundEffectName;
	mSize   = 0x58;
	for (int i = 0; i < 2; i++) {
		gcaxSndOpenPort(&mPorts[i]);
		for (int channel = 0; channel < 0x10; channel++) {
			sfxMidiResetChannel(mPorts[i], (s8)channel);
		}
	}
	gcaxSndResetAll();
	mPool.init(0x20);
	SoundNode* node;
	for (int i = 0; i < 0x20; i++) {
		node = mPool.mNodes;
		node += i;
		node->mVoice.mPort = i >> 4;
		mPool.mNodes[i].mVoice.mChannel = i & 0xF;
	}
}

// 0x80030C6C
TSoundEffect::~TSoundEffect()
{
	for (int i = 0; i < 2; i++) {
		gcaxSndClosePort(mPorts[i]);
	}
}

// 0x80030CFC
int TSoundEffect::play(int id, int priority, int frames, int volume, int pan, int bank)
{
	int useBank;
	if ((u8)bank == 0) {
		useBank = mTable[id * 2];
	} else {
		useBank = bank;
	}
	int idx = id * 2;
	if (isWaiting(useBank & 0xFF, mTable[idx + 1])) {
		return -1;
	}
	SoundVoice* voice = allocVoice(priority);
	if (voice == 0) {
		return -1;
	}
	voice->mFlags    = 0xCC;
	voice->mPriority = priority;
	voice->mVolume   = volume;
	voice->mNote     = 0x3C;
	voice->mPan      = pan;
	voice->mPitch    = 0;
	voice->mFrames   = frames;
	voice->mSerial   = mSerial++;
	voice->mBank     = (u8)useBank;
	voice->mProgram  = mTable[idx + 1];
	return voice->mSerial;
}

// 0x80030DEC
int TSoundEffect::playOther(int id, int priority, int frames, int volume, int pan, int bank)
{
	int useBank;
	if ((u8)bank == 0) {
		useBank = mTable[id * 2];
	} else {
		useBank = bank;
	}
	int idx = id * 2;
	if (isWaiting(useBank & 0xFF, mTable[idx + 1])) {
		return -1;
	}
	SoundVoice* voice = allocVoice(priority);
	if (voice == 0) {
		return -1;
	}
	voice->mFlags    = 0xAC;
	voice->mPriority = priority;
	voice->mVolume   = volume;
	voice->mNote     = 0x3C;
	voice->mPan      = pan;
	voice->mPitch    = 0;
	voice->mFrames   = frames;
	voice->mSerial   = mSerial++;
	voice->mBank     = (u8)useBank;
	voice->mProgram  = mTable[idx + 1];
	return voice->mSerial;
}

// 0x80030EDC
void TSoundEffect::setPan(int serial, int pan)
{
	SoundVoice* voice = findVoice2(serial);
	if (voice != 0) {
		voice->mPan = pan;
		voice->mFlags &= ~0x40;
		voice->mFlags |= 0x20;
	}
}

// 0x80030F2C
void TSoundEffect::setPitch(int serial, int pitch)
{
	SoundVoice* voice = findVoice2(serial);
	if (voice != 0) {
		voice->mPitch = pitch;
		voice->mFlags |= 8;
	}
}

// 0x80030F70
void TSoundEffect::setVolume(int serial, int volume)
{
	SoundVoice* voice = findVoice2(serial);
	if (voice != 0) {
		voice->mVolume = volume;
		voice->mFlags |= 0x80;
	}
}

// 0x80030FB4
// It stops on the next frame
void TSoundEffect::stop(int serial)
{
	SoundVoice* voice = findVoice2(serial);
	if (voice != 0) {
		voice->mFrames = 1;
	}
}

// 0x80030FE4
void TSoundEffect::stopAll()
{
	mPool.takeBack(&mWaiting);
	SoundNode* node = mPlaying.mHead;
	while (node != 0) {
		SoundNode* next = node->mNext;
		stopVoice(&node->mVoice);
		node = next;
	}
	mPool.takeBack(&mPlaying);
}

// 0x80031048
int TSoundEffect::isPlaying(int serial) { return findVoice(serial) != 0; }

// 0x80031074
SoundVoice* TSoundEffect::allocVoice(int priority)
{
	SoundNode* node = mPool.alloc();
	if (node == 0) {
		node = mPlaying.mHead;
		if (node->mVoice.mPriority > (u8)priority) {
			return 0;
		}
		stopVoice(&node->mVoice);
		mPlaying.remove(node);
	}
	mWaiting.append(node);
	return &node->mVoice;
}

// 0x800310F4
SoundVoice* TSoundEffect::findVoice(int serial)
{
	for (SoundNode* node = mWaiting.mHead; node != 0; node = node->mNext) {
		if ((u32)serial == node->mVoice.mSerial) {
			return &node->mVoice;
		}
	}
	for (SoundNode* node = mPlaying.mHead; node != 0; node = node->mNext) {
		if ((u32)serial == node->mVoice.mSerial) {
			return &node->mVoice;
		}
	}
	return 0;
}

// 0x8003114C
SoundVoice* TSoundEffect::findVoice2(int serial)
{
	for (SoundNode* node = mWaiting.mHead; node != 0; node = node->mNext) {
		if ((u32)serial == node->mVoice.mSerial) {
			return &node->mVoice;
		}
	}
	for (SoundNode* node = mPlaying.mHead; node != 0; node = node->mNext) {
		if ((u32)serial == node->mVoice.mSerial) {
			return &node->mVoice;
		}
	}
	return 0;
}

// 0x800311A4
void TSoundEffect::draw()
{
	if ((mFrame++ & 3) == 0) {
		while (mWaiting.mCount != 0) {
			SoundNode* node = mWaiting.mHead;
			mWaiting.remove(node);
			SoundNode* at = mPlaying.mHead;
			while (at != 0 && at->mVoice.mPriority <= node->mVoice.mPriority) {
				at = at->mNext;
			}
			mPlaying.insertBefore(at, node);
		}
	}

	SoundNode* next;
	SoundVoice* voice;
	SoundNode* node = mPlaying.mHead;
	while (node != 0) {
		voice = &node->mVoice;
		next = node->mNext;
		if (!(voice->mFlags & 0x10)) {
			if (voice->mFlags & 0x80) {
				voice->mFlags &= 0x7F;
				sfxMidiVolume(mPorts[voice->mPort], voice->mChannel, voice->mVolume);
			}
			if (voice->mFlags & 0x40) {
				voice->mFlags &= ~0x40;
				sfxMidiPan(mPorts[voice->mPort], voice->mChannel, voice->mPan);
			}
			if (voice->mFlags & 0x20) {
				voice->mFlags &= ~0x20;
				sfxMidiPanB(mPorts[voice->mPort], voice->mChannel, voice->mPan);
			}
			if (voice->mFlags & 8) {
				voice->mFlags &= ~8;
				sfxMidiPitch(mPorts[voice->mPort], voice->mChannel, voice->mPitch + 0x2000);
			}
			if (voice->mFlags & 4) {
				voice->mFlags &= ~4;
				sfxMidiProgram(mPorts[voice->mPort], voice->mChannel, (s8)voice->mBank, (s8)voice->mProgram);
				sfxMidiNoteOn(mPorts[voice->mPort], voice->mChannel, voice->mNote, 0x7F);
			}
			if (voice->mFrames != 0) {
				if (--voice->mFrames == 0) {
					stopVoice(voice);
					mPlaying.remove(node);
					mPool.free(node);
				}
			}
		}
		node = next;
	}
}

// 0x800313D0
void TSoundEffect::pauseAll()
{
	for (SoundNode* node = mWaiting.mHead; node != 0; node = node->mNext) {
		node->mVoice.mFlags |= 0x10;
	}
	for (SoundNode* node = mPlaying.mHead; node != 0; node = node->mNext) {
		node->mVoice.mFlags |= 0x10;
		sfxMidiVolume(mPorts[node->mVoice.mPort], node->mVoice.mChannel, 0);
	}
}

// 0x80031458
void TSoundEffect::resumeAll()
{
	for (SoundNode* node = mWaiting.mHead; node != 0; node = node->mNext) {
		node->mVoice.mFlags &= ~0x10;
	}
	for (SoundNode* node = mPlaying.mHead; node != 0; node = node->mNext) {
		if (node->mVoice.mFlags & 0x10) {
			node->mVoice.mFlags &= ~0x10;
			sfxMidiVolume(mPorts[node->mVoice.mPort], node->mVoice.mChannel, node->mVoice.mVolume);
		}
	}
}

// 0x800314E8
void TSoundEffect::silenceAll()
{
	for (SoundNode* node = mPlaying.mHead; node != 0; node = node->mNext) {
		sfxMidiVolume(mPorts[node->mVoice.mPort], node->mVoice.mChannel, 0);
	}
}

// 0x80031544
int TSoundEffect::isWaiting(int bank, int program)
{
	for (SoundNode* node = mWaiting.mHead; node != 0; node = node->mNext) {
		if (bank == node->mVoice.mBank && program == node->mVoice.mProgram) {
			return 1;
		}
	}
	return 0;
}

// 0x80031580
void TSoundEffect::stopVoice(SoundVoice* voice)
{
	sfxMidiNoteOff(mPorts[voice->mPort], voice->mChannel, voice->mNote, 0);
}

// 0x800315C4
extern "C" void sfxMidiResetChannel(u32 port, int channel)
{
	u8 msg[8];
	gcaxSndMakeMsg(port, msg, (channel | 0xB0) & 0xFF, 0x79, 0);
	gcaxSndSendMsg(port, msg, 0);
}

// 0x80031618
extern "C" void sfxMidiPanB(u32 port, int channel, int pan)
{
	u8 msg[8];
	gcaxSndMakeMsg(port, msg, (channel | 0xB0) & 0xFF, 0x3E, (u8)pan);
	gcaxSndSendMsg(port, msg, 0);
}

// 0x8003166C
extern "C" void sfxMidiNoteOn(u32 port, int channel, s16 note, s8 velocity)
{
	u8 msg[8];
	gcaxSndMakeMsg(port, msg, (channel | 0x90) & 0xFF, note, velocity);
	gcaxSndSendMsg(port, msg, 0);
}

// 0x800316C8
extern "C" void sfxMidiNoteOff(u32 port, int channel, s16 note, s8 velocity)
{
	u8 msg[8];
	gcaxSndMakeMsg(port, msg, (channel | 0x80) & 0xFF, note, velocity);
	gcaxSndSendMsg(port, msg, 0);
}

// 0x80031724
extern "C" void sfxMidiPan(u32 port, int channel, int pan)
{
	u8 msg[8];
	gcaxSndMakeMsg(port, msg, (channel | 0xB0) & 0xFF, 0xA, (u8)pan);
	gcaxSndSendMsg(port, msg, 0);
}

// 0x80031778
extern "C" void sfxMidiPitch(u32 port, u8 channel, s16 pitch)
{
	u8 msg[8];
	gcaxSndMakeMsg(port, msg, (u8)(channel | 0xE0), pitch & 0x7F, pitch >> 7);
	gcaxSndSendMsg(port, msg, 0);
}

// 0x800317D0
extern "C" void sfxMidiProgram(u32 port, int channel, s8 bank, s8 program)
{
	u8 msg[8];
	gcaxSndMakeMsg(port, msg, ((s8)channel | 0xB0) & 0xFF, 0x20, bank);
	gcaxSndSendMsg(port, msg, 0);
	gcaxSndMakeMsg(port, msg, ((s8)channel | 0xC0) & 0xFF, program);
	gcaxSndSendMsg(port, msg, 0);
}

// 0x80031858
extern "C" void sfxMidiVolume(u32 port, int channel, int volume)
{
	u8 msg[8];
	gcaxSndMakeMsg(port, msg, (channel | 0xB0) & 0xFF, 7, (u8)volume);
	gcaxSndSendMsg(port, msg, 0);
}

// 0x800318AC
void SoundList::insertBefore(SoundNode* at, SoundNode* node)
{
	if (at == 0) {
		append(node);
		return;
	}
	node->mPrev = at->mPrev;
	node->mNext = at;
	if (at == mHead) {
		mHead = node;
	} else {
		at->mPrev->mNext = node;
	}
	at->mPrev = node;
	mCount++;
}

// 0x80031914
void SoundList::append(SoundNode* node)
{
	if (mHead == 0) {
		mTail = node;
		mHead = node;
	} else {
		node->mPrev  = mTail;
		mTail->mNext = node;
		mTail        = node;
	}
	mCount++;
}

// 0x80031950
void SoundList::remove(SoundNode* node)
{
	if (node->mPrev != 0) {
		node->mPrev->mNext = node->mNext;
	} else {
		mHead = node->mNext;
	}
	if (node->mNext != 0) {
		node->mNext->mPrev = node->mPrev;
	} else {
		mTail = node->mPrev;
	}
	node->mNext = 0;
	node->mPrev = 0;
	mCount--;
}

// 0x800319AC
SoundPool::SoundPool()
{
	mFree  = 0;
	mNodes = 0;
	mCount = 0;
}

// 0x800319C0
SoundPool::~SoundPool()
{
	if (mNodes != 0) {
		delete[] mNodes;
	}
}

// 0x80031A1C
SoundNode* SoundPool::alloc()
{
	SoundNode* node = mFree;
	if (node != 0) {
		mFree       = node->mNext;
		node->mNext = 0;
		node->mPrev = 0;
	}
	return node;
}

// 0x80031A44
void SoundPool::free(SoundNode* node)
{
	if (node != 0) {
		node->mNext = mFree;
		mFree       = node;
	}
}

// 0x80031A5C
void SoundPool::takeBack(SoundList* list)
{
	if (list->mTail != 0) {
		list->mTail->mNext = mFree;
		mFree              = list->mHead;
	}
	list->mTail  = 0;
	list->mHead  = 0;
	list->mCount = 0;
}

// 0x80031A8C
void SoundPool::init(int count)
{
	mNodes = new SoundNode[count];
	for (int i = 0; i < count - 1; i++) {
		mNodes[i].mNext = &mNodes[i + 1];
	}
	mNodes[count - 1].mNext = 0;
	mFree  = mNodes;
	mCount = count;
}
