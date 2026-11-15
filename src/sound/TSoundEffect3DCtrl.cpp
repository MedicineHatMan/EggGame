#include "enemy/boss/EneBoss.h"
#include "core/VecMath.h"
#include "object/ObjAction.h"
#include "sound/SoundCtrl.h"
#include "core/MathUtil.h"
#include "sound/TSoundEffect3DCtrl.h"
#include "player/Player.h"

struct Sound3DList;

typedef char TSoundEffect3DCtrl_size_check[(sizeof(TSoundEffect3DCtrl) == 0x48) ? 1 : -1];

const char* gTSoundEffect3DCtrlName = "TSoundEffect3DCtrl";

// 0x80122C98
TSoundEffect3DCtrl::TSoundEffect3DCtrl(TObject* parent)
    : TObject(parent)
    , mMono(0)
{
	_00    = (u32)gTSoundEffect3DCtrlName;
	mSize  = 0x48;
	mPos.x = 0.0f;
	mPos.y = 0.0f;
	mPos.z = 0.0f;
	mDir   = gAxisZ;
}

// 0x80122D24
TSoundEffect3DCtrl::~TSoundEffect3DCtrl()
{
}

// 0x80122D90
void TSoundEffect3DCtrl::setMono(int mono)
{
	mMono = mono;
}

// 0x80122D98
void TSoundEffect3DCtrl::setListener(const Vec3f* pos, const Vec3f* dir)
{
	mPos = *pos;
	mDir = *dir;
}

// 0x80122DCC
void TSoundEffect3DCtrl::draw()
{
	for (SoundHandle* node = mList.mHead; node != 0; node = node->mNext) {
		SoundHandle* handle = node->mSelf;
		if (handle->mFlags & 2) {
			continue;
		}
		u8 volume, pan;
		calc(handle->mRange, &handle->mPos, handle->mNear, handle->mFar, &volume, 0.0f, &pan);
		if ((handle->mFlags & 1) && seIsPlaying(handle->mVoice) == 0) {
			handle->mVoice = sePlayOther(handle->mId, handle->mMode, volume, pan, 0);
		}
		seSetVolume(handle->mVoice, volume);
		seSetPan(handle->mVoice, pan);
	}
}

static inline f32 sound3DSqrt(f32 x)
{
	vf32 y;
	if (x > 0.0f) {
		f64 guess = __frsqrte(x);
		guess     = 0.5 * guess * (3.0 - guess * guess * x);
		guess     = 0.5 * guess * (3.0 - guess * guess * x);
		guess     = 0.5 * guess * (3.0 - guess * guess * x);
		y         = (f32)(x * guess);
		return y;
	}
	return x;
}

// 0x80122E8C
void TSoundEffect3DCtrl::calc(f32 range, const Vec3f* pos, int near, int far, u8* volume, f32, u8* pan)
{
	if (mMono == 0) {
		f32 dist = vecDistance(&mPos, pos) - 10.0f;
		if (dist <= 0.0f) {
			*volume = near;
		} else if (dist < range) {
			f32 part = dist / range;
			if (part < 0.0f) {
				part = 0.0f;
			}
			f32 left = sound3DSqrt(1.0f - part);
			*volume  = (u8)far + ((int)(left * (f32)((u8)near - (u8)far)) & 0xFF);
		} else {
			*volume = 0;
		}
		*pan = (angleDiff(angleToXZ(&mPos, pos), vecAngleXZ(&mDir)) + 0x4000) >> 9 & 0x7F;
	} else {
		*volume = near;
		*pan    = 0x20;
	}
}

// 0x80123014
int TSoundEffect3DCtrl::play(SoundHandle* handle, int id, int group, int mode, const Vec3f* pos, int near, int far, u16 extra, f32 range,
                             f32 unused)
{
	if (handle != 0 && handle->mList != 0) {
		return handle->mVoice;
	}
	u8 volume, pan;
	calc(range, pos, near, far, &volume, unused, &pan);
	int voice;
	if (handle != 0) {
		voice = sePlayOtherBank(id, mode, volume, pan, 0, group);
	} else {
		voice = sePlayOtherBank(id, mode, volume, pan, extra, group);
	}
	if (handle != 0) {
		mList.append(handle);
		handle->mSelf  = handle;
		handle->mNear  = near;
		handle->mFar   = far;
		handle->mFlags = 0;
		handle->mMode  = mode;
		handle->mGroup = group;
		handle->mId    = id;
		handle->mRange = range;
		handle->mPos   = *pos;
		handle->mVoice = voice;
	}
	return voice;
}

// 0x8012314C
void TSoundEffect3DCtrl::stop(SoundHandle* handle)
{
	if (handle->mList != 0) {
		seStop(handle->mVoice);
		mList.remove(handle);
	}
}

// 0x80123198
void TSoundEffect3DCtrl::pauseAll()
{
	for (SoundHandle* node = mList.mHead; node != 0; node = node->mNext) {
		node->mSelf->mFlags |= 2;
	}
}

// 0x801231C0
void TSoundEffect3DCtrl::resumeAll()
{
	for (SoundHandle* node = mList.mHead; node != 0; node = node->mNext) {
		node->mSelf->mFlags &= ~2;
	}
}

// 0x801231E8
Sound3DList::Sound3DList()
{
	mHead  = 0;
	mTail  = 0;
	mCount = 0;
}

// 0x801231FC
Sound3DList::~Sound3DList()
{
	clear();
}

// 0x80123244
void Sound3DList::append(SoundHandle* handle)
{
	if (mHead == 0) {
		mTail = handle;
		mHead = handle;
	} else {
		handle->mPrev = mTail;
		mTail->mNext  = handle;
		mTail         = handle;
	}
	handle->mList = this;
	mCount++;
}

// 0x80123284
void Sound3DList::remove(SoundHandle* handle)
{
	if (handle->mPrev != 0) {
		handle->mPrev->mNext = handle->mNext;
	} else {
		mHead = handle->mNext;
	}
	if (handle->mNext != 0) {
		handle->mNext->mPrev = handle->mPrev;
	} else {
		mTail = handle->mPrev;
	}
	handle->mNext = 0;
	handle->mPrev = 0;
	handle->mList = 0;
	mCount--;
}

// 0x801232E4
void Sound3DList::clear()
{
	SoundHandle* node = mHead;
	while (node != 0) {
		SoundHandle* next = node->mNext;
		node->mList       = 0;
		node->mNext       = 0;
		node->mPrev       = 0;
		node              = next;
	}
}
