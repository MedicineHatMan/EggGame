#ifndef _TOBJSKULL_H
#define _TOBJSKULL_H
struct SetDesc;
struct TObject;
struct DrawTask;

#include "stage/SetActor.h"
#include "object/ObjModel.h"
#include "engine/MotionPlayer.h"

class TObjSkull;

struct SkullNode {
	SkullNode* mNext;        // _00
	SkullNode* mPrev;        // _04
	TObjSkull* mObj;         // _08
	struct SkullList* mList; // _0C

	SkullNode()
	    : mNext(0)
	    , mPrev(0)
	    , mList(0)
	{
	}
};

/** The list every skull joins */
struct SkullList {
	SkullList();
	~SkullList();
	void clear();
	void add(SkullNode* node);
	void remove(SkullNode* node);

	SkullNode* mTail; // _00
	SkullNode* mHead; // _04
	int mCount;       // _08
};

class TObjSkull : public TSetActor {
public:
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
	TObjSkull(TObject* parent, SetDesc* desc);
	virtual ~TObjSkull();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual void vfn_74();

	void stateIdle();
	void stateLight();
	void stateOpening();
	void stateOpen();

	int lightEyes(u16 id);
	int openJaw();
	int isJawOpen();

	SkullNode mNode;      // _64
	MotionPlayer mMotion; // _74
	ObjModel mModel;      // _B0
	ObjPlace mPlace;      // _D0
	DrawTask* mDrawTask;  // _12C
	int mAct;             // _130 0 = idle 1 = lighting the eyes 2 = opening 3 = open
	int mMotionId;        // _134
	u16 mSkullId;         // _138
	int mLitId;           // _13C what lightEyes was last asked for
	f32 mScale;           // _140
	int mTimer;           // _144
};

extern "C" {
void initSkulls();
void termSkulls();
}

extern "C" {
TObjSkull* findSkull(u16);
TObject* createTObjSkull(TObject*, SetDesc*);
}

extern "C" {
extern const char* gTObjSkullName;
extern SkullList* sSkullListObj;
}

#endif
