#include "ui/AeArc.h"
#include "ui/AeObjControl.h"
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "enemy/boss/TEneYellowBossBlue.h"
#include "egg/TEggCreatureTypeE.h"
#include "ui/TAeObjGo.h"
#include "egg/EggCreature.h"
#include "core/Mem.h"

#include "animal/TAniManager.h"
#include "core/TMainTask.h"

typedef char TAniManager_size_check[(sizeof(TAniManager) == 0x4C) ? 1 : -1];

TAniManager* gAniManager; // 0x802C5780
TArc* sAeGoArc;           // 0x802C5784

// 0x80152E84
void loadGoAnimalPad()
{
	gAniManager = 0;
	loadArc("pa_ani_regular.pad", &sAeGoArc);
	if (sAeGoArc != 0) {
		fxBankBind(9, sAeGoArc->mData);
	}
	aniLoadFiles();
}

// 0x80152ED4
void freeGoAnimalPad()
{
	gAniManager = 0;
	if (sAeGoArc != 0) {
		fxBankUnbind(9, sAeGoArc->mData);
		releaseResource(sAeGoArc);
		sAeGoArc = 0;
	}
	aniFreeFiles();
}

// 0x80152F24
void createGoAnimal()
{
	gAniManager = new TAniManager(gTL_01, 0);
}

// 0x80152F68
extern "C" void createAniManager()
{
	gAniManager = new TAniManager(gTL_01, 1);
}

// 0x80152FAC
extern "C" void aniManagerAdd(TEggCreatureTypeE* creature)
{
	if (gAniManager != 0) {
		gAniManager->mList->add(&creature->mLink);
	}
}

// 0x80152FE4
extern "C" void aniManagerRemove(TEggCreatureTypeE* creature)
{
	if (gAniManager != 0) {
		gAniManager->mList->remove(&creature->mLink);
	}
}

// 0x8015301C
extern "C" int aniManagerAllocId()
{
	if (gAniManager != 0) {
		return gAniManager->allocId();
	}
	return -1;
}

// 0x80153050
extern "C" void aniManagerFreeId(int id)
{
	if (gAniManager != 0) {
		gAniManager->freeId(id);
	}
}

// 0x80153084
extern "C" int aniManagerGetMode()
{
	if (gAniManager != 0) {
		return gAniManager->mMode;
	}
	return -1;
}

const char* gTAniManagerName = "TAniManager";

// 0x801530A0
TAniManager::TAniManager(TObject* parent, int mode)
    : TObject(parent)
{
	mList   = 0;
	mMode   = mode;
	mList   = new AniList;
	mNextId = 10;
	for (int i = 0; i < 8; i++) {
		mUsed[i] = 0;
	}
	_00   = (u32)gTAniManagerName;
	mSize = 0x4C;
}

// 0x80153140
TAniManager::~TAniManager()
{
	delete mList;
	mList       = 0;
	gAniManager = 0;
}

// 0x801531B8
void TAniManager::draw()
{
	switch (mMode) {
	case 0:
		limitByRemovable();
		break;
	case 1:
		limitByAge();
		break;
	}
}

// 0x801531FC
void TAniManager::limitByRemovable()
{
	AniList* list = mList;
	if (list->mCount > 2) {
		for (AniListLink* link = list->mFirst; link != 0; link = link->mNext) {
			if (link->mOwner->vfn_88() != 0) {
				link->mOwner->vfn_7C();
				return;
			}
		}
	}
}

// 0x80153278
void TAniManager::limitByAge()
{
	AniList* list = mList;
	if (list->mCount > 2) {
		for (AniListLink* link = list->mFirst; link != 0; link = link->mNext) {
			if (link->mOwner != 0) {
				link->mOwner->vfn_7C();
				return;
			}
		}
	}
}

// 0x801532D8
int TAniManager::allocId()
{
	for (int i = 0; i < 8; i++) {
		if (mUsed[i] == 0) {
			mNextId = i + 10;
			break;
		}
	}
	int slot = mNextId - 10;
	if (mUsed[slot] == 0) {
		mUsed[slot] = 1;
	}
	return slot + 10;
}

// 0x8015333C
void TAniManager::freeId(int id)
{
	if (mUsed[id - 10] != 0) {
		mUsed[id - 10] = 0;
	}
}

// 0x8015335C
AniList::AniList()
{
	mFirst = 0;
	mLast  = 0;
	mCount = 0;
}

// 0x80153370
AniList::~AniList()
{
	clear();
}

// 0x801533B8
void AniList::add(AniListLink* link)
{
	if (mFirst == 0) {
		mLast  = link;
		mFirst = link;
	} else {
		link->mPrev  = mLast;
		mLast->mNext = link;
		mLast        = link;
	}
	link->mList = this;
	mCount++;
}

// 0x801533F8
void AniList::remove(AniListLink* link)
{
	if (link->mPrev != 0) {
		link->mPrev->mNext = link->mNext;
	} else {
		mFirst = link->mNext;
	}
	if (link->mNext != 0) {
		link->mNext->mPrev = link->mPrev;
	} else {
		mLast = link->mPrev;
	}
	link->mNext = 0;
	link->mPrev = 0;
	link->mList = 0;
	mCount--;
}

// 0x80153458
void AniList::clear()
{
	AniListLink* link = mFirst;
	while (link != 0) {
		AniListLink* next = link->mNext;
		link->mList       = 0;
		link->mNext       = 0;
		link->mPrev       = 0;
		link              = next;
	}
}
