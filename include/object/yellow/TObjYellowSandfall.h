#ifndef _OBJECT_YELLOW_TOBJYELLOWSANDFALL_H_
#define _OBJECT_YELLOW_TOBJYELLOWSANDFALL_H_
#include "Ninja/ninja.h"
#include "engine/DrawTask.h"
#include "engine/dlist.h"
#include "object/ObjModel.h"
#include "sound/SoundHandle.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct SandfallFx;
struct SandfallList;
struct SandfallNode;
struct TObjYellowSandfall;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
void pauseEventSandfalls();
void startEventSandfalls();
void stopEventSandfalls();
}

extern "C" {
TObject* createTObjYellowSandfall(TObject*, SetDesc*);
}

/** One entry on the list of sand falls */
struct SandfallNode {
	SandfallNode()
	{
		mNext = 0;
		mPrev = 0;
		mList = 0;
	}

	SandfallNode* mNext;       // _00
	SandfallNode* mPrev;       // _04
	TObjYellowSandfall* mObj;  // _08
	struct SandfallList* mList; // _0C
};

/** The list every sand fall joins */
struct SandfallList {
	SandfallList();
	~SandfallList();
	void clear();
	void add(SandfallNode* node);
	void remove(SandfallNode* node);

	SandfallNode* mTail; // _00
	SandfallNode* mHead; // _04
	int mCount;          // _08
};

struct SandfallFx {
	u8 _00[4];
	u16 mFlags; // _04
	u8 _06[0x1E];
	Vec3f mPos; // _24
};

class TObjYellowSandfall : public TSetActor {
public:
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
	TObjYellowSandfall(TObject* parent, SetDesc* desc);
	virtual ~TObjYellowSandfall();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual void vfn_74();

	void updateMatrix();
	void showEffect();
	void hideEffect();
	void updateEffect();
	void scroll();
	void stateWait();
	void stateRun();
	void stateStop();
	void start();
	void stop();

	SandfallNode mNode;  // _64
	NJS_OBJECT* mModelData;    // _74
	NJS_TEXLIST* mTexList;      // _78
	DrawTask* mDrawTask;     // _7C
	ObjModel mModel;     // _80
	ObjDrawState mState;    // _A0
	f32 mMatrix[12];     // _C4
	s16 mType;           // _F4 0 = NORMAL 1 = EVENT
	f32 mScrollU;        // _F8
	f32 mScrollV;        // _FC
	SandfallFx* mEffect; // _100
	Vec3f mEffectPos;    // _104
	Vec3f mCenter;       // _110
	f32 mRadius;         // _11C
	SoundHandle mSound;  // _120
	int mAct;            // _14C
};

extern "C" {
extern const char* gTObjYellowSandfallName;
}

#endif
