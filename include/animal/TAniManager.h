#ifndef _ANIMAL_TANIMANAGER_H_
#define _ANIMAL_TANIMANAGER_H_
#include "Ninja/ninja.h"
#include "egg/TEggCreature.h"
#include "stage/Stage.h"
struct AniList;
struct TAniManager;
struct TEggCreatureTypeE;

#include "core/types.h"

extern "C" {
int aniManagerAllocId();
void aniManagerFreeId(int);
int aniManagerGetMode();
void createAniManager();
}

extern "C" {
void aniManagerAdd(TEggCreatureTypeE*);
void aniManagerRemove(TEggCreatureTypeE*);
}

/**
 * @size 0xC
 */
struct AniList {
	AniList();
	~AniList();

	void add(AniListLink* link);
	void remove(AniListLink* link);
	void clear();

	AniListLink* mLast;  // _00
	AniListLink* mFirst; // _04
	int mCount;          // _08
};

/**
 * @size 0x4C
 */
class TAniManager : public TObject {
public:
	TAniManager(TObject* parent, int mode);
	virtual ~TAniManager();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	void limitByRemovable();
	void limitByAge();
	int allocId();
	void freeId(int id);

	AniList* mList; // _20
	int mMode;      // _24
	int mNextId;    // _28
	int mUsed[8];   // _2C for the ids 10 to 17
};

extern "C" {
extern const char* gTAniManagerName;
}

#endif
