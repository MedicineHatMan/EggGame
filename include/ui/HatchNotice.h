#ifndef _UI_HATCHNOTICE_H_
#define _UI_HATCHNOTICE_H_
#include "stage/Stage.h"
struct HatchNotice;
struct Vec3f;

#include "core/types.h"

extern "C" {
HatchNotice* hatchNoticePost(f32, int, int, const Vec3f*, s16);
}

/**
 * @size 0x18
 */
struct HatchNotice {
	HatchNotice* mNext; // _00
	s16 mKind;          // _04
	f32 mRangeSq;       // _08
	Vec3f mPos;         // _0C
};

#endif
