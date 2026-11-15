#ifndef _OBJECT_YELLOW_TOBJYELLOWFOGTRAPCTRL_H_
#define _OBJECT_YELLOW_TOBJYELLOWFOGTRAPCTRL_H_
#include "stage/SetActor.h"
struct DrawTask;
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjYellowFogTrapCtrl;

#include "core/types.h"

class SetDesc;
class TObject;

extern "C" {
TObject* createTObjYellowFogTrapCtrl(TObject*, SetDesc*);
}

class TObjYellowFogTrapCtrl : public TSetActor {
public:
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
	TObjYellowFogTrapCtrl(TObject* parent, SetDesc* desc);
	virtual ~TObjYellowFogTrapCtrl();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual void vfn_74();

	void updateMatrix();
	void createShape();
	void updateShape();
	void freeShape();
	void applyFog();
	void stateOutside();
	void stateInside();
	void stateOff();

	DrawTask* mDrawTask; // _64
	f32 mMatrix[12];     // _68
	u16 mSwitchId;       // _98
	int mFogId;          // _9C
	f32 mWidth;          // _A0
	f32 mHeight;         // _A4
	f32 mDepth;          // _A8
	int mFirstTime;      // _AC
	int mFogOn;          // _B0
	int mClearTimer;     // _B4
	int mAct;            // _B8 0 = outside 1 = inside 2 = off
};

extern "C" {
extern const char* gTObjYellowFogTrapCtrlName;
extern void* sFogTrapCtrlObj;
}

#endif
