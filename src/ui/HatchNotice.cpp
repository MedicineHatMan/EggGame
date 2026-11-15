#include "core/types.h"
#include "stage/Stage.h"

#include "ui/HatchNotice.h"

struct HatchNoticeList {
	HatchNotice* add(int, int, const Vec3f* pos, s16 kind, f32 range);

	u8 _00[0x24];
	int mCount;          // _24
	HatchNotice* mUsed;  // _28
	HatchNotice* mFree;  // _2C
};

static HatchNoticeList* sNoticeList; // 0x802C56B0

// 0x8013DAAC
extern "C" HatchNotice* hatchNoticePost(f32 range, int a, int b, const Vec3f* pos, s16 kind)
{
	if (sNoticeList != NULL) {
		return sNoticeList->add(a, b, pos, kind, range);
	}
	return NULL;
}

// 0x8013DB00
HatchNotice* HatchNoticeList::add(int, int, const Vec3f* pos, s16 kind, f32 range)
{
	HatchNotice* node = mFree;
	if (node != NULL) {
		mFree       = node->mNext;
		node->mNext = mUsed;
		mUsed       = node;
		mCount++;
		for (int i = 0; i < 4; i++) {
		}
		node->mPos     = *pos;
		node->mRangeSq = range * range;
		node->mKind    = kind;
	}
	return node;
}
