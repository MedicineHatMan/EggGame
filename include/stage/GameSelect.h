#ifndef _GAMESELECT_H
#define _GAMESELECT_H

#include "stage/Stage.h"

struct GameSelectItem {
	int mScene;     // _00 the scene to go to
	char mName[0x20]; // _04
};

/**
 * "SELECT GAME MODE": a list of up to 32 scenes. A picks one (state 0), B goes back to the title (state 1).
 * @size 0x1C
 */
class TModeGameSelect : public TMode {
public:
	TModeGameSelect();
	static TModeGameSelect* create(TModeAllocator* allocator);

	virtual ~TModeGameSelect();
	virtual int init();
	virtual int shutdown();
	virtual void update();

	void addItem(int scene, const char* name);

	int mCursor;            // _0C
	GameSelectItem* mItems; // _10 32 of them
	int mCount;             // _14
	int mFrame;             // _18 blinks the cursor
};

class TAeObjComing;

/**
 * Plays the "Coming" banner from ae_coming.arc until it ends or A is pressed. Scene 12, which nothing goes to.
 * @size 0x10
 */
class TModeComing : public TMode {
public:
	TModeComing();
	static TModeComing* create(TModeAllocator* allocator);

	virtual ~TModeComing();
	virtual int init();
	virtual int shutdown();
	virtual void update();

	TAeObjComing* mComing; // _0C
};

#include "Dolphin/GX/GXTypes.h"
extern "C" {
extern GXColor sGameSelectClearColor;
}

#endif
