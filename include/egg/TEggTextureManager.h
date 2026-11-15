#ifndef _TEGGTEXTUREMANAGER_H
#define _TEGGTEXTUREMANAGER_H

#include "stage/Stage.h"

extern "C" void objHeapFree(void*, void*);

/**
 * One live egg texture. Twelve of these sit in the free list.
 *
 * @size 0x18
 */
struct EggTexSlot {
	EggTexSlot* mNext; // _00
	EggTexSlot* mPrev; // _04
	s16 mIndex;        // _08 0 to 11
	s16 mId;           // _0A
	int mRefs;         // _0C
	void* mData;       // _10
	u32 _14;           // _14
};

/**
 * Free list of 12 slots. mHead is the live list.
 *
 * @size 0x130
 */
struct EggTexSlots {
	EggTexSlots();

	EggTexSlot* take();
	void drop(EggTexSlot* slot);
	EggTexSlot* pop();
	void push(EggTexSlot* slot);
	void* freeSelf(s16 flag);

	EggTexSlot* mHead;       // _00
	EggTexSlot* mTail;       // _04
	int mCount;              // _08
	EggTexSlot mSlots[12];   // _0C
	EggTexSlot* mFree;       // _12C
};

/**
 * A second live list. Sixteen nodes, each 0x10.
 *
 * @size 0x10
 */
struct EggTexHold {
	EggTexHold* mNext; // _00
	EggTexHold* mPrev; // _04
	int mKey;          // _08
	int mRefs;         // _0C
};

/**
 * @size 0x110
 */
struct EggTexHolds {
	EggTexHolds();

	void drop(EggTexHold* hold);
	void push(EggTexHold* hold);
	void* freeSelf(s16 flag);

	EggTexHold* mHead;      // _00
	EggTexHold* mTail;      // _04
	int mCount;             // _08
	EggTexHold mHolds[16];  // _0C
	EggTexHold* mFree;      // _10C
};

/**
 * One image in the loaded table. Twelve of them, pointed at by mNames.
 *
 * @size 0xC
 */
struct EggTexName {
	u32 _00;       // _00
	u32 _04;       // _04
	void* mImage;  // _08
};

/**
 * Loads egg.gpl and hands out its images. acquire bumps a refcount, release drops it.
 *
 * @size 0x314
 */
class TEggTextureManager : public TObject {
public:
	TEggTextureManager(TObject* parent);
	virtual ~TEggTextureManager();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw(); // _0C

	int acquire(int id);
	void release(int id);
	void* getImage(int id);
	EggTexHold* findHold(int key);
	EggTexSlot* findSlot(int id);

	EggTexSlots mSlots;      // _20
	EggTexHolds mHolds;      // _150
	void* mSlotBuf;          // _260 mTexBytes * 12
	u32 _264;                // _264 first word of the file header
	u32 mFileCount;          // _268
	u32 _26C;                // _26C
	u32 mFileSize;           // _270
	void* mTable;            // _274 file body each row 0x10
	u32 mTexBytes;           // _278
	EggTexName* mNames;      // _27C points at mNameData
	int mNameCount;          // _280 12
	EggTexName mNameData[12]; // _284
};

extern "C" {
void createTEggTextureManager();
}

extern "C" {
TEggTextureManager* getTEggTextureManager();
}

extern "C" {
extern TEggTextureManager* sEggTextureMgr;
extern const char* gTEggTextureManagerName;
}

#endif
