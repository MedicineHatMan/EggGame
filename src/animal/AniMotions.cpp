#include "animal/AniMount.h"
#include "io/Resource.h"

extern "C" {
}

// What the AniModel export holds as AniMotions reads it
struct AniMotionFile {
	u32 _00;
	void* mMotions; // _04
};

// 0x801C0AEC
AniMotions::AniMotions()
    : mState(0)
    , mPlayer(NULL)
    , mTable(NULL)
{
}

// 0x801C0B0C
void AniMotions::set(MotionPlayer* player, const MotionEntry* table)
{
	mPlayer = player;
	mTable  = table;
}

// 0x801C0B18
void AniMotions::update(AniAsset* asset)
{
	switch (mState) {
	case 0:
		if (asset->mLink.mList != NULL && asset->mFile->mArc != NULL) {
			load(asset);
			mState = 1;
		}
		break;
	case 1:
		if (asset->mLink.mList == NULL) {
			mState = 2;
		}
		break;
	}
}

// 0x801C0BA0
void* AniMotions::find(const AniAsset* asset)
{
	return findArcExport(asset->mFile->mArc, "AniModel");
}

// 0x801C0BD0
void AniMotions::load(AniAsset* asset)
{
	AniMotionFile* file = (AniMotionFile*)find(asset);
	if (mPlayer != NULL) {
		mPlayer->init(mTable, file->mMotions);
	}
}
