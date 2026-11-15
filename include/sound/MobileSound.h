#ifndef _MOBILESOUND_H
#define _MOBILESOUND_H

#include "core/types.h"
#include "stage/Stage.h"
#include "object/ObjCommon.h"
#include "sound/SoundHandle.h"

/**
 * @size 0x38
 */
struct MobileSound {
	MobileSound();  // 0x80174EBC
	~MobileSound(); // 0x80174F14
	void start();
	void stop();
	void setPos(const Vec3f* pos);
	void setSe(int se);
	void update();

	FlagByte mFlags0;    // _00 unused
	FlagByte mFlags;     // _01 1 = playing 2 = changed since the last update
	int mSe;             // _04 sound id 0x7B by default
	f32 mLevel;          // _08 the speed Pans the sound from -0x2000 at 10 to 0x1FFF at 100
	SoundHandle mHandle; // _0C mHandle.mPos is the position
};

#endif
