#ifndef _OBJECT_MISSION_TMSNOBJDARKGATE_H_
#define _OBJECT_MISSION_TMSNOBJDARKGATE_H_
struct TArc;
#include "camera/EventCamRef.h"
struct NJS_OBJECT;
struct DrawTask;
#include "camera/TCamera.h"
#include "engine/Fx.h"
#include "engine/ModelDraw.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct SetDesc;
struct TMsnObjDarkGate;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTMsnObjDarkGate(TObject*, SetDesc*);
}

class TMsnObjDarkGate : public TSetActor {
public:
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
	TMsnObjDarkGate(TObject* parent, SetDesc* desc);
	virtual ~TMsnObjDarkGate();

	virtual void draw();   // _0C the update
	virtual void vfn_10(); // _10
	virtual void render(); // _1C
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual int sendToHandle(ObjMsg* msg);

	int takeCamera(ObjMsg* msg);
	void waitCamera();
	void waitUser();
	void appear();
	void done();
	void appearStart();
	void appearFade();
	void appearEnd();
	void leave();
	void leaving();
	void left();

	DrawTask* mDrawTask;       // _64
	ModelDraw mDraw;       // _68
	ObjDrawState mState;   // _84
	f32 mMatrix[12];       // _A8
	Fx* mGlow;           // _D8
	u8 mHidden;            // _DC
	u16 mSwitch;           // _DE
	ObjRef mUser;          // _E0 who goes through
	EventCamRef mCamRef;   // _EC
	void* mCamera;         // _F4
	TCamPath* mEventCam;   // _F8
	Fx* mOpenFx;         // _FC
	u16 mTimer;            // _100
	int mSetCam;           // _104
	Vec3f mCamEye;         // _108
	Vec3f mCamAt;          // _114
	int mMusic;            // _120
	int mDelay;            // _124
	int mGate;             // _128
	int mAppear;           // _12C
};

struct NJS_TEXLIST;

extern "C" {
extern TArc* sDarkGateArc;
extern NJS_OBJECT* gDarkGateModel;
extern NJS_TEXLIST* gDarkGateTexList;
extern f32 gDarkGateRadius;
extern const char* gTMsnObjDarkGateName;
}

#endif
