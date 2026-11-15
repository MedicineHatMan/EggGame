#ifndef _OBJECT_MAP_TOBJMAPDOOR01_H_
#define _OBJECT_MAP_TOBJMAPDOOR01_H_
#include "Ninja/ninja.h"
#include "camera/EventCamRef.h"
#include "enemy/EnemyLight.h"
#include "engine/DrawTask.h"
#include "engine/MeshColli.h"
#include "engine/dlist.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
class TParticleGenerator;
struct DoorList;
struct DoorListNode;
struct DoorNode;
struct SetDesc;
struct TObjMapDoor01;
struct TObject;

#include "core/types.h"

class TCamera;

extern "C" {
void doorUnused1();
void doorUnused2();
void initDoors();
void termDoors();
}

extern "C" {
TObject* createDoorA(TObject*, SetDesc*);
TObject* createDoorB(TObject*, SetDesc*);
TObject* createDoorC(TObject*, SetDesc*);
TObject* createDoorD(TObject*, SetDesc*);
TObject* createDoorE(TObject*, SetDesc*);
TObject* createDoor02(TObject*, SetDesc*);
TObject* createDoorF(TObject*, SetDesc*);
TObject* createDoorG(TObject*, SetDesc*);
}

/** The part of a motion tree this class walks */
struct DoorNode {
	u8 _00[4];
	void* mModel; // _04
	u8 _08[0x24];
	DoorNode* mChild; // _2C
	DoorNode* mNext;  // _30
};

struct DoorListNode {
	DoorListNode()
	{
		mNext = 0;
		mPrev = 0;
		mList = 0;
	}

	DoorListNode* mNext;    // _00
	DoorListNode* mPrev;    // _04
	TObjMapDoor01* mObj;    // _08
	struct DoorList* mList; // _0C
};

/** The list every door joins */
struct DoorList {
	DoorList();
	~DoorList();
	void clear();
	void add(DoorListNode* node);
	void remove(DoorListNode* node);

	DoorListNode* mTail; // _00
	DoorListNode* mHead; // _04
	int mCount;          // _08
};

class TObjMapDoor01 : public TObjAction {
public:
	TObjMapDoor01(TObject* parent, SetDesc* desc);
	virtual ~TObjMapDoor01();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual Vec3f* getPos();
	virtual int getRotY();
	virtual int getKind();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual int sendToHandle(ObjMsg* msg);
	virtual SetLink* getLink(); // _6C

	void setup();
	int onCameraMessage(ObjMsg* msg);
	void initOpen();
	void startCamera();
	void endCamera();
	void startEffects();
	void endEffects();
	void createColli(int flag);
	void openOthers();
	void updateNodes(DoorNode* node, int index);
	int stepOpen();
	void pose();
	int stepClose();
	void shakeCamera();
	void quake();

	DoorListNode mNode;        // _30
	NJS_OBJECT* mModel;        // _40
	NJS_OBJECT* mFrameModel;   // _44 MODEL_MON01
	NJS_TEXLIST* mTexList;     // _48
	void* mFrameTexList;       // _4C
	void* mMotion;             // _50
	MeshColli* mColli;         // _54
	MeshObject* mFrameColli;   // _58
	MeshData* mColliData;      // _5C
	MeshData* mFrameColliData; // _60
	TParticleGenerator* mDust; // _64
	Vec3f mPos;                // _68
	Vec3f mSoundPos;           // _74
	SetRot mRot;               // _80
	f32 mMatrix[12];           // _8C
	SetLinkMember mLink;       // _BC
	DrawTask* mDrawTask;       // _C8
	EnemyLight mLight;         // _CC
	TCamera* mCamera;          // _10C
	int mAct;                  // _110 0 = shut 1 = about to open 2 = opening 3 = open 4 = closing
	int mTimer;                // _114
	EventCamRef mCam;          // _118
	int mHasCamera;            // _120
	int _124;                  // _124
	int mDoorId;               // _128
	f32 mFrame;                // _12C
	f32 mFrameStep;            // _130
	int mSetCamPos;            // _134
	Vec3f mCamPos;             // _138
	Vec3f mCamTarget;          // _144
	int mLightSet;             // _150
	int mColliNode;            // _154 the node the door's collision follows
	int mDustId;               // _158
	int mBurstId;              // _15C
	int mSoundId;              // _160
	Vec3f mBoxLow;             // _164
	Vec3f mBoxHigh;            // _170
	f32 mRadius;               // _17C
};

extern "C" {
extern DoorList* sDoorListObj;
}

#endif
