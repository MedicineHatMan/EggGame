#ifndef _OBJECT_YELLOW_TOBJYELLOWSTATUE_H_
#define _OBJECT_YELLOW_TOBJYELLOWSTATUE_H_
#include "Ninja/ninja.h"
struct DrawTask;
#include "camera/TCamera.h"
#include "engine/MotionPlayer.h"
#include "engine/dlist.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct StatueList;
struct StatueNode;
struct TObjYellowStatue;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjYellowStatue(TObject*, SetDesc*);
}

/** One entry on the list of statues */
struct StatueNode {
	StatueNode* mNext;       // _00
	StatueNode* mPrev;       // _04
	TObjYellowStatue* mObj;  // _08
	struct StatueList* mList; // _0C

	StatueNode() : mNext(0), mPrev(0), mList(0) { }
};

/** The list every statue joins */
struct StatueList {
	StatueList();
	~StatueList();
	void clear();
	void add(StatueNode* node);
	void remove(StatueNode* node);

	StatueNode* mTail; // _00
	StatueNode* mHead; // _04
	int mCount;        // _08
};

class TObjYellowStatue : public TSetActor {
public:
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
	TObjYellowStatue(TObject* parent, SetDesc* desc);
	virtual ~TObjYellowStatue();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual void vfn_74();

	void updateMatrix();
	int othersDone();
	void dimOthers();
	void stateTurn();
	void stateWait();
	void stateMotion();
	void stateEvent();
	void stateCheck();
	void stateDone();

	StatueNode mNode;     // _64
	NJS_OBJECT* mModelData;     // _74
	NJS_TEXLIST* mTexList;       // _78
	DrawTask* mDrawTask;      // _7C
	ObjModel mModel;      // _80
	MotionPlayer mMotion; // _A0
	f32 mMatrix[12];      // _DC
	u16 mAngle;           // _10C
	s16 mMirror;          // _10E the first int 0 = type A otherwise type B
	u16 mSwitchTurn;      // _110
	u16 mSwitchOff;       // _112
	u16 mCheckId;         // _114
	u16 mTimer;           // _116
	TCamPath* mEvent;     // _118
	Vec3f mCenter;        // _11C
	f32 mRadius;          // _128
	int mAct;             // _12C
};

#endif
