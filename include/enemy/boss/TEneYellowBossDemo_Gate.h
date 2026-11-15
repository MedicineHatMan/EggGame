#ifndef _ENEMY_BOSS_TENEYELLOWBOSSDEMO_GATE_H_
#define _ENEMY_BOSS_TENEYELLOWBOSSDEMO_GATE_H_
#include "engine/MotionPlayer.h"
struct DrawTask;
struct NJS_OBJECT;
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TEneYellowBossDemo_Gate;

#include "core/types.h"

extern "C" {
void spawnDemoGate0();
void spawnDemoGate1();
}

extern "C" {
int eventTalk(int*);
int eventSetTalkMode(int*);
int eventPrisonSave(int*);
int eventGoldSave(int*);
}

class TEneYellowBossDemo_Gate : public TObject {
public:
	TEneYellowBossDemo_Gate(TObject* parent, int arg);
	virtual ~TEneYellowBossDemo_Gate();
	virtual void draw();   // _0C
	virtual void vfn_10(); // _10
	virtual void render(); // _1C

	void freeParts();

	void* operator new(unsigned long size) { return objHeapAlloc(gObjHeap, size); }
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	SetTransform mXform;  // _20
	int mArg;             // _64
	NJS_OBJECT* mModel;   // _68
	MotionPlayer mMotion; // _6C
	DrawTask* mDraw;      // _A8
	TObject* mPart[2];    // _AC the two sparkles
};

#endif
