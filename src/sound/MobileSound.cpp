#include "sound/MobileSound.h"
#include "animal/AniMount.h"
#include "sound/SoundCtrl.h"

MobileSound::MobileSound()
    : mSe(0x7B)
    , mLevel(0.0f)
{
}

MobileSound::~MobileSound()
{
	seStop3D(&mHandle);
}

// 0x80174F60
void MobileSound::start()
{
	if (*(volatile u8*)&mFlags.mBits & 1) {
		return;
	}
	mFlags.mBits |= 1;
	mFlags.mBits |= 2;
}

// 0x80174F88
void MobileSound::stop()
{
	if (!(*(volatile u8*)&mFlags.mBits & 1)) {
		return;
	}
	mFlags.mBits &= ~1;
	mFlags.mBits |= 2;
}

void MobileSound::setPos(const Vec3f* pos)
{
	mHandle.mPos = *pos;
}

void MobileSound::setSe(int se)
{
	if (se == mSe) {
		return;
	}
	mSe = se;
	mFlags.mBits |= 2;
}

// 0x80174FEC
static inline int soundLevelPan(f32 level)
{
	if (level <= 10.0f) {
		return -0x2000;
	} else if (level >= 200.0f) {
		return 0x1FFF;
	}
	return (s16)((int)(16384.0f * ((level - 10.0f) / 90.0f)) - 0x2000);
}

void MobileSound::update()
{
	if (*(volatile u8*)&mFlags.mBits & 2) {
		*(volatile u8*)&mFlags.mBits &= ~2;
		if (*(volatile u8*)&mFlags.mBits & 1) {
			Vec3f pos;
			pos.x = mHandle.mPos.x;
			pos.y = mHandle.mPos.y;
			pos.z = mHandle.mPos.z;
			sePlay3DHeld(400.0f, &mHandle, mSe, 200, (const Vec3f*)&pos);
		} else {
			seStop3D(&mHandle);
		}
	}
	if (mFlags.mBits & 1) {
		f32 level = mLevel;
		seSetPitch(mHandle.mVoice, soundLevelPan(level));
		if (level < 10.0f) {
			mHandle.mNear = 0;
		} else {
			mHandle.mNear = 0x7F;
		}
	}
}
