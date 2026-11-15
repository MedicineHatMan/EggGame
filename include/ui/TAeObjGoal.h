#ifndef _TAEOBJGOAL_H
#define _TAEOBJGOAL_H

#include "ui/TAeLayer.h"
struct DrawTask;

/**
 * @size 0x3C
 */
class TAeObjGoal : public TAeObject {
public:
	TAeObjGoal(TObject* parent, void* data, void* assets);
	virtual ~TAeObjGoal();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void render(); // _1C

	TAeLayer* mLayer; // _34
	DrawTask* mTask;  // _38
};

extern "C" {
extern const char* gTAeObjGoalName;
extern u32 sAeGoalArc;
extern u32 sAeGoalData;
extern u32 sAeGoalTex;
}

TAeObjGoal* createGoal();
int loadGoal();

#endif
