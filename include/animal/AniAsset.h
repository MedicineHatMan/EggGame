#ifndef _ANIASSET_H
#define _ANIASSET_H

#include "core/types.h"
#include "object/ObjCommon.h"

class TArc;
struct AniAsset;
struct AniAssetList;

/** An AniAsset's place in the list of a slot's users. @size 0x10 */
struct AniAssetLink {
	AniAssetLink()
	{
		mNext = 0;
		mPrev = 0;
		mList = 0;
	}

	AniAssetLink* mNext; // _00
	AniAssetLink* mPrev; // _04
	AniAsset* mOwner;    // _08
	AniAssetList* mList; // _0C null while it is in none
};

/** @size 0xC */
struct AniAssetList {
	AniAssetList();  // 0x801B0BC0
	~AniAssetList(); // 0x801B0BD4

	void append(AniAssetLink* link); // 0x801B0AB8
	void clear();                    // 0x801B0AF8
	void remove(AniAssetLink* link); // 0x801B0B34
	void unlinkAll();                // 0x801B0B94 leaves the count alone

	AniAssetLink* mTail; // _00
	AniAssetLink* mHead; // _04
	int mCount;          // _08
};

struct AniModelSlot;

/** The list half of AniModelSlot */
struct AniModelSlotLink {
	AniModelSlot* mNext; // _00 in the manager's list or the free list
	AniModelSlot* mPrev; // _04
};

/** The data half of AniModelSlot a second base its dtor is reached through a null-checked cast */
struct AniModelSlotData {
	~AniModelSlotData() { }

	int mKind;           // _08
	int mLoadId;         // _0C from requestDiscLoad
	void* mBlock;        // _10
	TArc* mArc;          // _14 the animal's archive with the AniModel export set once the read is done
	int mFxBank;         // _18 the effect bank
	AniAssetList mUsers; // _1C
};

/**
 * One animal's model file: an ARC read into a block of sAniBlocks, and the effect bank it brings.
 * @size 0x28
 */
struct AniModelSlot : AniModelSlotLink, AniModelSlotData {
	AniModelSlot(); // 0x801B0A28
	~AniModelSlot(); // 0x801B09D0
};

/**
 * An animal's model file, loaded in the background. aniModelRequest starts loading the file for a kind of animal,
 * aniAssetAttach hooks this up once it is there (mLink.mList is set then), and aniAssetDetach lets it go.
 * Flag 1 keeps the slot from being reused.
 *
 * @size 0x18
 */
struct AniAsset {
	AniAsset(); // 0x801B02B0

	int isKept();  // 0x801B02F8
	void keep();  // 0x801B0304
	void unkeep(); // 0x801B0314

	FlagByte mFlags;    // _00
	u8 _01[3];
	AniAssetLink mLink; // _04
	AniModelSlot* mFile; // _14
};

extern "C" {
const char* aniModelFile(int kind);              // 0x801B0108
void aniModelInit();                             // 0x801B0154
void aniModelExit();                             // 0x801B01B0
void aniModelRequest(int kind);                  // 0x801B0204
int aniAssetAttach(int kind, AniAsset* asset);   // 0x801B0238
void aniAssetDetach(AniAsset* asset);            // 0x801B027C
}

#endif
