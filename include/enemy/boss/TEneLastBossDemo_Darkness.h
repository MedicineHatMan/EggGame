#ifndef _ENEMY_BOSS_TENELASTBOSSDEMO_DARKNESS_H
#define _ENEMY_BOSS_TENELASTBOSSDEMO_DARKNESS_H
#include "Ninja/ninja.h"
#include "stage/Stage.h"
struct TEneLastBossDemo_Darkness;

#include "core/types.h"

class TEneLastBossDemo_Darkness : public TObject {
public:
	TEneLastBossDemo_Darkness(TObject* parent);
	virtual ~TEneLastBossDemo_Darkness();

	virtual void draw();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	NJS_OBJECT mBank; // _20 a bare object for the motion to move
	void* mTable;   // _58
	f32 mTime;      // _5C
	void* mFx[2];   // _60
};

#endif
