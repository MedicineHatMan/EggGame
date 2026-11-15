#ifndef _MENUMODELARC_H
#define _MENUMODELARC_H

#include "core/types.h"
#include "io/Resource.h"

struct MenuModelSet {
	int mCount;       // _00
	void* _04;        // _04
	void** mModels;   // _08
	void** mMotions;  // _0C length is a u32 at +0x04
	void** mTexLists; // _10
	void** mGvms;     // _14
};

extern TArc* gMenuModelArc;         // 0x802C5890
extern MenuModelSet* gMenuModelSet; // 0x802C5894

void loadLandModelArc();
void loadBattleModelArc();
void freeLandModelArc();
void freeBattleModelArc();

#endif
