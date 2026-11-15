#ifndef _OBJECT_LAST_TOBJLASTSEALEDCREST_H_
#define _OBJECT_LAST_TOBJLASTSEALEDCREST_H_
#include "engine/DrawTask.h"
#include "engine/Light.h"
#include "engine/ModelDraw.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct SetDesc;
struct TObjLastSealedCrest;
struct TObject;

#include "core/types.h"

extern "C" {
void initTObjLastSealedCrest();
void shutdownTObjLastSealedCrest();
}

extern "C" {
TObject* createTObjLastSealedCrest(TObject*, SetDesc*);
}

class TObjLastSealedCrest : public TSetActor {
public:
	TObjLastSealedCrest(TObject* parent, SetDesc* desc);
	virtual ~TObjLastSealedCrest();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_10(); // _10
	virtual void render(); // _1C
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);

	void arm(); // 0x8019A570
	void breakOpen();
	void stateSealed();
	void stateOpening();
	void stateOpen();

	DrawTask* mDraw;     // _64
	ModelDraw mMesh;     // _68
	ObjDrawState mState; // _84
	f32 mMtx[12];        // _A8
	u16 mSwitchId;       // _D8
	u16 _DA;
	int mEvent;   // _DC
	int mDarkMap; // _E0 the id darkCtrlAddMap gave
	u16 mTimer;   // _E4 frames from opening to the event
	u16 _E6;
	void* mFx;        // _E8
	int mLinked;      // _EC
	PointLight mGlow; // _F0
	u16 mPulse;       // _16C the glow's angle
	u16 _16E;
	int mMode; // _170 0 sealed 1 opening 2 open
};

#endif
