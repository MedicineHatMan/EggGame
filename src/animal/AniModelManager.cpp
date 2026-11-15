#include "animal/AniAsset.h"
#include "io/Resource.h"
#include "core/BlockPool.h"
#include "Dolphin/os.h"
#include "engine/FxSystem.h"
#include "animal/TAniManager.h"
#include "io/ResManager.h"
#include "ui/AeArc.h"
#include "animal/AniModelManager.h"

struct NJS_TEXLIST;

/** The head of an animal archive's AniModel export */
struct AniModelExport {
	u8 _00[0x14];
	NJS_TEXLIST** mTexList; // _14
	u8* mGvm;               // _18
};

extern "C" {

AniModelName sAniModelNames[] = {
	{ 0, "ani_model_pen.arc" },
	{ 1, "ani_model_kiji.arc" },
	{ 2, "ani_model_hari.arc" },
	{ 3, "ani_model_saru.arc" },
	{ 4, "ani_model_kuma.arc" },
	{ 5, "ani_model_dog.arc" },
	{ 6, "ani_model_owl.arc" },
	{ 7, "ani_model_orix.arc" },
	{ 8, "ani_model_bull.arc" },
	{ 9, "ani_model_devil.arc" },
	{ 10, "ani_model_hawk.arc" },
	{ 11, "ani_model_sonic.arc" },
	{ 12, "ani_model_tails.arc" },
	{ 13, "ani_model_knuckles.arc" },
	{ 14, "ani_model_chao.arc" },
	{ 15, "ani_model_lappy.arc" },
	{ 16, "ani_model_kapu.arc" },
	{ 17, "ani_model_nights.arc" },
	{ 18, "ani_model_amigo.arc" },
	{ 19, "ani_model_rare1.arc" },
	{ 20, "ani_model_rare2.arc" },
	{ 21, "ani_model_lion.arc" },
	{ 22, "ani_model_crab.arc" },
	{ 23, "ani_model_camel.arc" },
	{ -1, 0 },
};

// 0x801B0108
const char* aniModelFile(int kind)
{
	for (int i = 0; sAniModelNames[i].mFile != 0; i++) {
		if (kind == sAniModelNames[i].mKind) {
			return sAniModelNames[i].mFile;
		}
	}
	return 0;
}

// 0x801B0154
void aniModelInit()
{
	sAniBlocks = new BlockPool(0x40000, 3);
	sAniModels = new AniModelManager;
}

// 0x801B01B0
void aniModelExit()
{
	delete sAniModels;
	delete sAniBlocks;
	sAniModels = 0;
	sAniBlocks = 0;
}

// 0x801B0204
void aniModelRequest(int kind)
{
	if (sAniModels != 0) {
		sAniModels->request(kind);
	}
}

// 0x801B0238
int aniAssetAttach(int kind, AniAsset* asset)
{
	if (sAniModels != 0) {
		return sAniModels->attach(kind, asset);
	}
	return 0;
}

// 0x801B027C
void aniAssetDetach(AniAsset* asset)
{
	if (sAniModels != 0) {
		sAniModels->detach(asset);
	}
}
}

// 0x801B02B0
AniAsset::AniAsset()
{
	mLink.mOwner = this;
	mFile        = 0;
}

// 0x801B02F8
int AniAsset::isKept()
{
	return mFlags.mBits & 1;
}

// 0x801B0304
void AniAsset::keep()
{
	mFlags.mBits |= 1;
}

// 0x801B0314
void AniAsset::unkeep()
{
	mFlags.mBits &= ~1;
}

// 0x801B0324
AniModelManager::AniModelManager()
{
	mSlots.mHead  = 0;
	mSlots.mTail  = 0;
	mSlots.mCount = 0;
	mPool.init(3);
	sAniModelManager = this;
}

// 0x801B0374
AniModelManager::~AniModelManager()
{
	for (AniModelSlot* slot = mSlots.mHead; slot != 0; slot = slot->mNext) {
		if (slot->mArc != 0) {
			releaseResource(slot->mArc);
		} else {
			cancelDiscLoad(slot->mLoadId);
		}
		sAniBlocks->release(slot->mBlock);
	}
	sAniModelManager = 0;
}

// 0x801B0408
// When the kind is already in its users are kept again
void AniModelManager::request(int kind)
{
	AniModelSlot* slot = find(kind);
	if (slot == 0) {
		void* block;
		const char* path = aniModelFile(kind);
		if (path == 0) {
			OSReport("AniModelManage: filename not found\n");
			return;
		}
		block = sAniBlocks->alloc();
		if (block == 0) {
			OSReport("AniModelManage: cannot alloc block\n");
			return;
		}
		slot = allocSlot(kind);
		if (slot == 0) {
			OSReport("AniModelManage: cannot alloc sysinfo\n");
			sAniBlocks->release(block);
			return;
		}
		int bank = aniManagerAllocId();
		mSlots.append(slot);
		slot->mKind   = kind;
		slot->mBlock  = block;
		slot->mArc    = 0;
		slot->mFxBank = bank;
		slot->mLoadId = requestDiscLoad(path, block, onLoaded);
	} else {
		for (AniAssetLink* link = slot->mUsers.mHead; link != 0; link = link->mNext) {
			link->mOwner->keep();
		}
	}
}

// 0x801B0518
int AniModelManager::attach(int kind, AniAsset* asset)
{
	AniModelSlot* slot = find(kind);
	if (slot == 0) {
		return 0;
	}
	asset->mFile = slot;
	slot->mUsers.append(&asset->mLink);
	for (AniAssetLink* link = slot->mUsers.mHead; link != 0; link = link->mNext) {
		link->mOwner->unkeep();
	}
	return 1;
}

// 0x801B0584
void AniModelManager::detach(AniAsset* asset)
{
	AniModelSlot* slot = asset->mFile;
	AniAssetLink* kept = slot->mUsers.mHead;
	while (kept != 0) {
		if (kept->mOwner->isKept() != 0) {
			break;
		}
		kept = kept->mNext;
	}
	slot->mUsers.remove(&asset->mLink);
	if (slot->mUsers.mCount == 0 && kept == 0) {
		if (slot->mArc != 0) {
			AniModelExport* model = (AniModelExport*)findArcExport(slot->mArc, "AniModel");
			if (*model->mTexList != 0) {
				releaseTextures(*model->mTexList);
			}
			fxRemoveBank(slot->mFxBank);
			void* anim = findArcExport(slot->mArc, "polyanim");
			if (anim != 0) {
				fxBankUnbind(slot->mFxBank & 0xFF, (unsigned char*)anim);
			}
			aniManagerFreeId(slot->mFxBank);
			releaseResource(slot->mArc);
		} else {
			cancelDiscLoad(slot->mLoadId);
		}
		mSlots.remove(asset->mFile);
		mPool.give(asset->mFile);
		sAniBlocks->release(slot->mBlock);
		asset->mFile = 0;
	}
}

// 0x801B069C
AniModelSlot* AniModelManager::find(int kind)
{
	for (AniModelSlot* slot = mSlots.mHead; slot != 0; slot = slot->mNext) {
		if (kind == slot->mKind) {
			return slot;
		}
	}
	return 0;
}

// 0x801B06C4
// A free slot or else the first one that none of its users keeps
AniModelSlot* AniModelManager::allocSlot(int kind)
{
	AniModelSlot* slot = mPool.take();
	if (slot == 0) {
		for (slot = mSlots.mHead; slot != 0; slot = slot->mNext) {
			AniAssetLink* kept = slot->mUsers.mHead;
			while (kept != 0) {
				if (kept->mOwner->isKept() != 0) {
					break;
				}
				kept = kept->mNext;
			}
			if (kept == 0) {
				slot->mUsers.clear();
				if (slot->mArc != 0) {
					releaseResource(slot->mArc);
				}
				sAniBlocks->release(slot->mBlock);
				return slot;
			}
		}
		return 0;
	}
	return slot;
}

// 0x801B0768
void AniModelManager::onLoaded(int size, int id)
{
	AniModelSlot* slot = sAniModelManager->mSlots.mHead;
	while (slot != 0) {
		if ((u32)id == slot->mLoadId) {
			break;
		}
		slot = slot->mNext;
	}
	if (slot != 0) {
		TArc* arc = new TArc(slot->mBlock);
		AniModelExport* model = (AniModelExport*)findArcExport(arc, "AniModel");
		if (*model->mTexList != 0) {
			numberTextures(*model->mTexList);
			bindGvmTextures(model->mGvm, *model->mTexList);
		}
		void* anim = findArcExport(arc, "polyanim");
		if (anim != 0) {
			fxBankBind(slot->mFxBank & 0xFF, (unsigned char*)anim);
		}
		slot->mArc = arc;
	}
}

// 0x801B0838
void AniModelSlotList::append(AniModelSlot* slot)
{
	if (mHead == 0) {
		mTail = slot;
		mHead = slot;
	} else {
		slot->mPrev  = mTail;
		mTail->mNext = slot;
		mTail        = slot;
	}
	mCount++;
}

// 0x801B0874
void AniModelSlotList::remove(AniModelSlot* slot)
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
	slot->mNext = 0;
	slot->mPrev = 0;
	mCount--;
}

// 0x801B08D0
AniModelSlotPool::AniModelSlotPool()
{
	mFree  = 0;
	mSlots = 0;
	mCount = 0;
}

// 0x801B08E4
AniModelSlot* AniModelSlotPool::take()
{
	AniModelSlot* slot = mFree;
	if (slot != 0) {
		mFree       = slot->mNext;
		slot->mNext = 0;
		slot->mPrev = 0;
	}
	return slot;
}

// 0x801B090C
void AniModelSlotPool::give(AniModelSlot* slot)
{
	if (slot == 0) {
		return;
	}
	slot->mNext = mFree;
	mFree       = slot;
}

// 0x801B0924
void AniModelSlotPool::init(int count)
{
	mSlots = new AniModelSlot[count];
	for (int i = 0; i < count - 1; i++) {
		mSlots[i].mNext = &mSlots[i + 1];
	}
	mSlots[count - 1].mNext = 0;
	mFree  = mSlots;
	mCount = count;
}

// 0x801B09D0
AniModelSlot::~AniModelSlot() { }

// 0x801B0A28
AniModelSlot::AniModelSlot() { }

// 0x801B0A5C
AniModelSlotPool::~AniModelSlotPool()
{
	if (mSlots != 0) {
		delete[] mSlots;
	}
}

// 0x801B0AB8
void AniAssetList::append(AniAssetLink* link)
{
	if (mHead == 0) {
		mTail = link;
		mHead = link;
	} else {
		link->mPrev  = mTail;
		mTail->mNext = link;
		mTail        = link;
	}
	link->mList = this;
	mCount++;
}

// 0x801B0AF8
void AniAssetList::clear()
{
	unlinkAll();
	mHead  = 0;
	mTail  = 0;
	mCount = 0;
}

// 0x801B0B34
void AniAssetList::remove(AniAssetLink* link)
{
	if (link->mPrev != 0) {
		link->mPrev->mNext = link->mNext;
	} else {
		mHead = link->mNext;
	}
	if (link->mNext != 0) {
		link->mNext->mPrev = link->mPrev;
	} else {
		mTail = link->mPrev;
	}
	link->mNext = 0;
	link->mPrev = 0;
	link->mList = 0;
	mCount--;
}

// 0x801B0B94
void AniAssetList::unlinkAll()
{
	AniAssetLink* link = mHead;
	while (link != 0) {
		AniAssetLink* next = link->mNext;
		link->mList = 0;
		link->mNext = 0;
		link->mPrev = 0;
		link        = next;
	}
}

// 0x801B0BC0
AniAssetList::AniAssetList()
{
	mHead  = 0;
	mTail  = 0;
	mCount = 0;
}

// 0x801B0BD4
AniAssetList::~AniAssetList()
{
	unlinkAll();
}
