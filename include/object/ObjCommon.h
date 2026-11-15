#ifndef _OBJCOMMON_H
#define _OBJCOMMON_H
struct ObjCommonData;
struct TArc;

#include "core/types.h"
#include "item/ItemList.h"

// Small classes that eggs and the player share

// A message between objects Eggs and the player's states take the same one
#include "stage/Stage.h"

struct ObjMsg {
	u16 mType; // _00
};

// A message carrying one vector, a push to add or a velocity to fill in
struct ObjVecMsg : public ObjMsg {
	u8 _02[2];  // _02
	Vec3f mVec; // _04
};

// A message carrying an object id, usually the sender's
struct ObjIdMsg : public ObjMsg {
	u8 _02[2]; // _02
	u16 mId;   // _04
};

// A message asking for one value back
struct ObjValueMsg : public ObjMsg {
	u8 _02[2];  // _02
	f32 mValue; // _04
};

/** Sixteen flag bits. The name comes from TObjSnowman, where its constructor is (0x800FBF5C). @size 0x2 */
struct SnowFlags {
	SnowFlags();
	void clear(); // 0x801BBABC

	u16 mBits; // _00
};

// FlagByte is in Stage.h where TGameContext holds one

/** Size 0x38 from where TEneAm01 and TChrPlayer put the next member */
struct ObjShadow {
	ObjShadow();
	~ObjShadow();

	ObjShadow* mPrev; // _00
	ObjShadow* mNext; // _04
	int mEnabled;     // _08
	Vec3f mGround;    // _0C
	Vec3f mNormal;    // _18
	int mTiltX;       // _24
	int mTiltY;       // _28
	int mAngle;       // _2C
	f32 _30;          // _30 size across
	f32 _34;          // _34 size along
};

struct FlagWord {
	FlagWord();
	FlagWord(int value);
	void operator=(const FlagWord& other);

	u32 mBits; // _00 1 = on the ground 2 = against a wall
};

/**
 * A user of an item archive. itemHolderAttach links it to the archive the manager loaded for an item id, and
 * itemHolderRelease unlinks it; the archive is dropped when its last holder goes. See TItemManage.h.
 *
 * @size 0x18
 */
struct ItemHolder {
	ItemHolder();

	int isPinned(); // bit 0 of mFlags the manager may not reuse its archive for another item

	// The player inlines these whether anything is held and the first item's id 0xFFFF for none
	bool hasItem() const { return mNode.mList != NULL ? true : false; }
	int firstId() { return mNode.mList != NULL ? mEntry->mData.mKind : 0xFFFF; }

	FlagByte mFlags;                  // _00
	u8 _01[3];                        // _01
	TRefList<ItemHolder>::Node mNode; // _04 in the archive's mHolders
	TListNode<ItemArc>* mEntry;       // _14 the archive while attached
};

struct NJS_OBJECT;
struct NJS_TEXLIST;

extern "C" {
void* findGeObjData(const char*);
void* findStgObjData(const char*);
NJS_OBJECT** getGeObjModels();
NJS_TEXLIST** getGeObjTextures();
void loadObjCommon(const char*, const char*);
void releaseObjCommon();
}

extern "C" {
struct StageModelInfo;
struct StageModelList {
	StageModelInfo** mModels; // _00
	int mCount;               // _04
};
extern StageModelList* gCommonModels;
extern TArc* sCommonArc;
extern StageModelList* gStageModels;
extern TArc* sStageObjArc;
extern TArc* gStgObjCommonFile;
extern TArc* gGeObjCommonFile;
}

#endif
