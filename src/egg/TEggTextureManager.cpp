#include "egg/TEggTextureManager.h"
#include "core/PlacementNew.h"
#include "io/ResManager.h"
#include "Dolphin/OS/OSCache.h"
#include "io/AramFile.h"
#include "core/Mem.h"
#include "core/TMainTask.h"
#include "Ninja/ninja.h"
struct TFile;

typedef char TEggTextureManager_size_check[(sizeof(TEggTextureManager) == 0x314) ? 1 : -1];
typedef char EggTexSlots_size_check[(sizeof(EggTexSlots) == 0x130) ? 1 : -1];
typedef char EggTexHolds_size_check[(sizeof(EggTexHolds) == 0x110) ? 1 : -1];
typedef char EggTexSlot_size_check[(sizeof(EggTexSlot) == 0x18) ? 1 : -1];
typedef char EggTexHold_size_check[(sizeof(EggTexHold) == 0x10) ? 1 : -1];

// 0x801BA378
extern "C" void createTEggTextureManager()
{
	sEggTextureMgr = new TEggTextureManager(gTL_02);
}

// 0x801BA3B8
extern "C" TEggTextureManager* getTEggTextureManager()
{
	return sEggTextureMgr;
}

const char* gTEggTextureManagerName = "TEggTextureManager";

/** One row of the texture file table */
struct EggTexRow { // size 0x10
	u32 _00;
	u32 _04;
	u32 mOffset; // _08
	u32 mSize;   // _0C
};

// 0x801BA3C0
TEggTextureManager::TEggTextureManager(TObject* parent)
    : TObject(parent)
{
	void* size;
	int pos;
	u8 stream[0x2A0] __attribute__((aligned(32)));
	TFile* file;
	u32 i;
	int n;
	GjTexMem* tex;
	GjTexHeader* name;
	int id;

	_00   = (u32)gTEggTextureManagerName;
	mSize = 0x314;
	pos   = getResPackSize("egg.gpl", &size);
	file  = new (stream) TAramFile(pos, (u32)size);
	fileReadAll(file, (void*)&_264, 0x10);
	mTable = memAlloc(mFileSize - 0x10);
	fileReadAll(file, mTable, mFileSize - 0x10);
	for (i = 0; i < mFileCount; i++) {
		((EggTexRow*)mTable)[i].mOffset += pos;
	}
	mTexBytes = gjTexCalcSize(0x80, 0x80, 0x10E);
	mSlotBuf  = memAlloc(mTexBytes * 0xC);
	for (i = 0; i < 12; i++) {
		mSlots.mSlots[i].mIndex = (s16)i;
		mSlots.mSlots[i].mData  = (u8*)mSlotBuf + i * mTexBytes;
	}
	mNames     = mNameData;
	mNameCount = 12;
	for (n = 0; n < 12; n++) {
		id = gjTexFindFree(&tex, &name);
		if (id < 0) {
			return;
		}
		tex->hdr = name;
		gjTexSurfaceSetup(&tex->hdr->info, 0x80, 0x80, 0x10E);
		name->info.image = mSlots.mSlots[n].mData;
		name->texaddr    = (void*)0x101B;
		tex->count       = tex->hdr->count;
		gjTexListSetSlot((NJS_TEXLIST*)&mNames, n, id);
	}
}

// 0x801BA5C8
TEggTextureManager::~TEggTextureManager()
{
	njReleaseTexture((NJS_TEXLIST*)&mNames);
	memFree(mTable);
	memFree(mSlotBuf);
	if (sEggTextureMgr == this) {
		sEggTextureMgr = 0;
	}
	mHolds.freeSelf(-1);
	mSlots.freeSelf(-1);
}

// 0x801BA66C
void TEggTextureManager::draw() { }

// 0x801BA670
int TEggTextureManager::acquire(int id)
{
	EggTexSlot* slot;

	if (id < 0 || (u32)id >= mFileCount) {
		return 0;
	}
	slot = findSlot(id);
	if (slot != 0) {
		slot->mRefs++;
		return 1;
	}
	slot        = mSlots.take();
	slot->mId   = id;
	slot->mRefs = 1;
	readResPackRange(slot->mData, ((EggTexRow*)mTable)[id].mOffset, (void*)((EggTexRow*)mTable)[id].mSize);
	EggTexRow* rows = (EggTexRow*)mTable;
	DCFlushRange(slot->mData, rows[id].mSize);
	return 1;
}

// 0x801BA720
void TEggTextureManager::release(int id)
{
	EggTexHold* hold;
	EggTexSlot* slot;

	hold = findHold(id);
	if (hold != 0) {
		if (--hold->mRefs == 0) {
			mHolds.drop(hold);
		}
	} else {
		slot = findSlot(id);
		if (slot != 0) {
			if (--slot->mRefs == 0) {
				mSlots.drop(slot);
			}
		}
	}
}

// 0x801BA7A8
void* TEggTextureManager::getImage(int id)
{
	EggTexSlot* slot;

	slot = findSlot(id);
	if (slot != 0) {
		return mNames[slot->mIndex].mImage;
	}
	return 0;
}

// 0x801BA7F8
EggTexHold* TEggTextureManager::findHold(int key)
{
	EggTexHold* hold;

	hold = mHolds.mHead;
	while (0 != hold) {
		if (key == hold->mKey) {
			return hold;
		}
		hold = hold->mNext;
	}
	return 0;
}

// 0x801BA820
EggTexSlot* TEggTextureManager::findSlot(int id)
{
	EggTexSlot* slot;

	slot = mSlots.mHead;
	while (slot != 0) {
		if (id == slot->mId) {
			return slot;
		}
		slot = slot->mNext;
	}
	return 0;
}

// 0x801BA848
EggTexSlots::EggTexSlots()
{
	int i;

	mHead  = 0;
	mTail  = 0;
	mCount = 0;
	for (i = 0; i < 11; i++) {
		mSlots[i].mNext = &mSlots[i + 1];
	}
	mSlots[11].mNext = 0;
	mFree            = &mSlots[0];
}

// 0x801BA89C
void* EggTexSlots::freeSelf(s16 flag)
{
	if (this != 0) {
		if (flag > 0) {
			delete this;
		}
	}
	return this;
}

// 0x801BA8D8
EggTexSlot* EggTexSlots::take()
{
	EggTexSlot* slot;

	slot = pop();
	if (slot == 0) {
		return 0;
	}
	if (mHead == 0) {
		mTail = slot;
		mHead = slot;
	} else {
		slot->mPrev  = mTail;
		mTail->mNext = slot;
		mTail        = slot;
	}
	mCount += 1;
	return slot;
}

// 0x801BA94C
void EggTexSlots::drop(EggTexSlot* slot)
{
	if (slot->mPrev != 0) {
		slot->mPrev->mNext = slot->mNext;
	} else {
		mHead = slot->mNext;
	}
	if (slot->mNext != 0) {
		slot->mNext->mPrev = slot->mPrev;
	} else {
		mTail = slot->mPrev;
	}
	push(slot);
	mCount -= 1;
}

// 0x801BA9C8
EggTexSlot* EggTexSlots::pop()
{
	EggTexSlot* slot;

	slot = mFree;
	if (slot != 0) {
		mFree       = slot->mNext;
		slot->mNext = 0;
		slot->mPrev = 0;
	}
	return slot;
}

// 0x801BA9F0
void EggTexSlots::push(EggTexSlot* slot)
{
	if (slot == 0) {
		return;
	}
	slot->mNext = mFree;
	mFree       = slot;
}

// 0x801BAA08
EggTexHolds::EggTexHolds()
{
	int i;

	mHead  = 0;
	mTail  = 0;
	mCount = 0;
	for (i = 0; i < 15; i++) {
		mHolds[i].mNext = &mHolds[i + 1];
	}
	mHolds[15].mNext = 0;
	mFree            = &mHolds[0];
}

// 0x801BAA5C
void* EggTexHolds::freeSelf(s16 flag)
{
	if (this != 0) {
		if (flag > 0) {
			delete this;
		}
	}
	return this;
}

// 0x801BAA98
void EggTexHolds::drop(EggTexHold* hold)
{
	if (hold->mPrev != 0) {
		hold->mPrev->mNext = hold->mNext;
	} else {
		mHead = hold->mNext;
	}
	if (hold->mNext != 0) {
		hold->mNext->mPrev = hold->mPrev;
	} else {
		mTail = hold->mPrev;
	}
	push(hold);
	mCount -= 1;
}

// 0x801BAB14
void EggTexHolds::push(EggTexHold* hold)
{
	if (hold == 0) {
		return;
	}
	hold->mNext = mFree;
	mFree       = hold;
}
