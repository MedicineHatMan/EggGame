#ifndef _SOUNDHANDLE_H
#define _SOUNDHANDLE_H

#include "core/types.h"
#include "stage/Stage.h"

struct Sound3DList;

#pragma push
#pragma sym off
/**
 * A sound kept up to date. Whoever starts it owns this.
 *
 * @size 0x2C
 */
struct SoundHandle {
	SoundHandle()
	{
		mNext = 0;
		mPrev = 0;
		mList = 0;
	}

	SoundHandle* mNext; // _00
	SoundHandle* mPrev; // _04
	SoundHandle* mSelf; // _08
	Sound3DList* mList;   // _0C
	u8 mNear;             // _10
	u8 mFar;              // _11
	u8 mFlags;            // _12 1 restart when it ends 2 paused
	u8 mMode;             // _13
	u8 mGroup;            // _14
	u16 mId;              // _16
	f32 mRange;           // _18
	int mVoice;           // _1C
	Vec3f mPos;           // _20
};
#pragma pop

extern "C" {
int sePlay3DHeld(f32 range, SoundHandle* handle, int id, int mode, const Vec3f* pos);
void seStop3D(SoundHandle* handle);
}

#endif
