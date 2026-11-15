#ifndef _TOBJREDEVENTMAGMA_H
#define _TOBJREDEVENTMAGMA_H
struct TArc;
struct SetDesc;
struct TObject;

#include "stage/SetActor.h"
#include "engine/ScrollModel.h"
#include "object/ObjModel.h"
#include "engine/MotionPlayer.h"
#include "engine/DrawTask.h"
#include "object/ObjCommon.h"
#include "camera/EventCamRef.h"

extern "C" void objHeapFree(void*, void*);

/**
 * One row of the archive list this object walks when it builds particles.
 *
 * @size 0x38
 */
struct MagmaRec {
	u32 _00[2]; // _00
	u32 mFlags; // _08 the bind callbacks edit this
	u32 mKind;  // _0C 4 and 9 are the special rows
	int mCount; // _10
	u32 _14[3]; // _14
	Vec3f mPos; // _20
	u32 _2C[3]; // _2C
};

/**
 * The magma that plays for an event. Set int 0 is the model (clamped to 0..2) and set int 3 is the switch.
 * While the switch is clear it waits. The event camera starts the motion, and message 0x92 follows.
 *
 * @size 0x168
 */
class TObjRedEventMagma : public TSetActor {
public:
	TObjRedEventMagma(TObject* parent, SetDesc* desc);
	virtual ~TObjRedEventMagma();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void*);           // _34
	virtual int sendToHandle(ObjMsg* msg);    // _4C
	virtual void setup();                     // _74

	// Names are ours
	void setAct(int act);
	void idle();
	void play();
	void settled();
	void bindOpen();
	void bindShut();
	void spawnRest();
	void spawnFour();
	void spawnNine();
	void freeFx();
	void freeFxB();

	int mCount;            // _64
	MagmaRec* mRecs;       // _68
	DrawTask* mDrawTask;   // _6C
	void* mGeo;            // _70
	void* mTex;            // _74
	ScrollModel mScroll;   // _78
	SetTransform mTrans;   // _98
	ObjModel mModel;       // _DC
	MotionPlayer mMotion;  // _FC
	u16 mModelId;          // _138 set int 0 clamped to 0..2
	u16 mEventId;          // _13A set int 3
	EventCamRef mCam;         // _13C
	int mStep;             // _144 cleared by setAct
	TObject* mFx;          // _148
	TObject* mFxB;         // _14C
	int mSound;            // _150 -1 means none
	Vec3f mCentre;         // _154
	f32 mRadius;           // _160
	int mAct;              // _164 0 waiting 1 playing 2 done
};

void loadMagmaArc();
void releaseMagmaArc();

extern "C" {
TObject* createTObjRedEventMagma(TObject*, SetDesc*);
}

extern "C" {
extern TArc* sRedEventMagmaArc;
extern void* sRedEventMagmaAssets;
}

#endif
