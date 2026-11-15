#ifndef _OBJECT_YELLOW_TOBJYELLOWSLIDEDOOR_H_
#define _OBJECT_YELLOW_TOBJYELLOWSLIDEDOOR_H_
#include "Ninja/ninja.h"
#include "camera/EventCamRef.h"
#include "camera/TCamera.h"
#include "enemy/EnemyLight.h"
#include "engine/DrawTask.h"
#include "engine/MeshColli.h"
#include "object/ObjCommon.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
#include "ui/TextVec.h"
struct TObjYellowSlideDoor;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjYellowSlideDoor(TObject*, SetDesc*);
}

class TObjYellowSlideDoor : public TSetActor {
public:
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
	TObjYellowSlideDoor(TObject* parent, SetDesc* desc);
	virtual ~TObjYellowSlideDoor();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual int sendToHandle(ObjMsg* msg);
	virtual void vfn_74();

	static void setColliBox(void* colli, const Vec3f* center, f32 radius);

	void updateMatrix();
	void initColli();
	void updateColli();
	void freeColli();
	void startEventCamera();
	void endEventCamera();
	int onCameraMessage(ObjMsg* msg);
	void stateWait();
	void stateOpen();
	void stateOpenAtEye();
	void stateDone();

	NJS_OBJECT* mModelData; // _64
	NJS_TEXLIST* mTexList;  // _68
	MeshData* mColliData;   // _6C
	DrawTask* mDrawTask;    // _70
	ObjDrawState mState;    // _74
	ObjModel mModel;        // _98
	f32 mMatrix[12];        // _B8
	MeshObject* mColli;     // _E8
	u16 mSwitchId;          // _EC
	int mUseCamera;         // _F0
	Vec3f mCenter;          // _F4
	f32 mRadius;            // _100
	f32 mLift;              // _104 how far along X the door has slid
	TCamera* mFixCam;       // _108 a TCamFix
	int mHasOffset;         // _10C
	Vec3f mCamEye;          // _110
	EventCamRef mCam;       // _11C
	EnemyLight mLight;      // _124
	int mAct;               // _164 0 = wait 1 = open 2 = open with the camera at the record's eye 3 = done
};

extern "C" {
extern InitVec sDoorCamOffset;
}

#endif
