#ifndef _ANIMAL_ANIMODELMANAGER_H_
#define _ANIMAL_ANIMODELMANAGER_H_
struct BlockPool;
struct AniModelName;
#include "Ninja/ninja.h"
struct AniModelManager;
struct AniModelSlot;
struct AniModelSlotList;
struct AniModelSlotPool;

#include "core/types.h"

struct AniAsset;

extern "C" {
int aniAssetAttach(int, AniAsset*);
void aniAssetDetach(AniAsset*);
void aniModelExit();
const char* aniModelFile(int);
void aniModelInit();
void aniModelRequest(int);
}

/** @size 0xC */
struct AniModelSlotList {
	void append(AniModelSlot* slot); // 0x801B0838
	void remove(AniModelSlot* slot); // 0x801B0874

	AniModelSlot* mHead; // _00
	AniModelSlot* mTail; // _04
	int mCount;          // _08
};

/** @size 0xC */
struct AniModelSlotPool {
	AniModelSlotPool();  // 0x801B08D0
	~AniModelSlotPool(); // 0x801B0A5C

	AniModelSlot* take();           // 0x801B08E4
	void give(AniModelSlot* slot);  // 0x801B090C
	void init(int count);           // 0x801B0924

	AniModelSlot* mSlots; // _00
	AniModelSlot* mFree;  // _04
	int mCount;           // _08
};

/** @size 0x18 */
struct AniModelManager {
	AniModelManager();  // 0x801B0324
	~AniModelManager(); // 0x801B0374

	void request(int kind);                  // 0x801B0408
	int attach(int kind, AniAsset* asset);   // 0x801B0518
	void detach(AniAsset* asset);            // 0x801B0584
	AniModelSlot* find(int kind);            // 0x801B069C
	AniModelSlot* allocSlot(int kind);       // 0x801B06C4
	static void onLoaded(int size, int id);  // 0x801B0768

	AniModelSlotPool mPool;  // _00
	AniModelSlotList mSlots; // _0C
};

/** One kind and its file */
struct AniModelName {
	int mKind;          // _00
	const char* mFile;  // _04 null at the end
};

extern "C" {
extern AniModelName sAniModelNames[];
extern BlockPool* sAniBlocks;
extern AniModelManager* sAniModels;
extern AniModelManager* sAniModelManager;
}

#endif
