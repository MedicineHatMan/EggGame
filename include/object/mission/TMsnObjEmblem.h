#ifndef _OBJECT_MISSION_TMSNOBJEMBLEM_H_
#define _OBJECT_MISSION_TMSNOBJEMBLEM_H_
struct EmblemColor;
class TParticleGenerator;
#include "Ninja/ninja.h"
#include "camera/EventCamRef.h"
#include "camera/TCamera.h"
#include "engine/DrawTask.h"
#include "engine/Fx.h"
#include "engine/Light.h"
#include "engine/MotionPlayer.h"
#include "object/ObjCommon.h"
#include "object/ObjModel.h"
#include "sound/SoundHandle.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TMsnObjEmblem;
struct SetDesc;
struct TEmblemGet;
struct TObject;
struct Vec3f;

#include "core/types.h"

struct TArc;

extern "C" {
void loadEmblemArc();
void releaseEmblemArc();
}

extern "C" {
void emblemShineBegin();
void emblemShineEnd();
void emblemShineTev(u8);
void emblemShineTexGen(u8);
}

extern "C" {
TEmblemGet* spawnEmblemGet(TObject*, const Vec3f*, int);
}

extern "C" {
TObject* createTMsnObjEmblem(TObject*, SetDesc*);
}

class TMsnObjEmblem : public TSetActor {
public:
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
	TMsnObjEmblem(TObject* parent, SetDesc* desc);
	virtual ~TMsnObjEmblem();

	virtual void draw();   // _0C the update
	virtual void vfn_10(); // _10
	virtual void render(); // _1C
	virtual int getKind() { return 3; }
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual int sendToHandle(ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0xC7:
			return takeCamera(msg);
		}
		return 0;
	}

	void setup();
	int takeCamera(ObjMsg* msg);
	void start();
	void openCamera();
	void closeCamera();
	void placeGenerators();
	void checkTaken();
	void waitCamera();
	void appear();
	void show();
	void taken();

	ObjModel mModel;          // _64
	ObjPlace mPlace;          // _84
	MotionPlayer mMotion;     // _E0
	DrawTask* mDrawTask;          // _11C
	f32 mView[12];            // _120
	int mState;               // _150
	int mKind;                // _154 0 normal 1 flag
	int mSwitch;              // _158 a flag's
	int _15C;                 // _15C
	TParticleGenerator* mGenA; // _160
	TParticleGenerator* mGenB; // _164
	ObjShadow mShadow;        // _168
	int mCameraId;            // _1A0
	TCamPath* mCamera;        // _1A4
	EventCamRef mCamRef;      // _1A8
	Fx* mFx;                // _1B0
	f32 mFxMatrix[12];        // _1B4
	SoundHandle mSound;       // _1E4
};

/**
 * @size 0x1D4
 */
class TEmblemGet : public TObject {
public:
	TEmblemGet(TObject* parent, const f32* matrix);
	virtual ~TEmblemGet();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();

	void startCamera();
	void updateEffects();

	f32 mMatrix[12];     // _20
	f32 mView[12];       // _50
	MotionPlayer mAnim;  // _80
	DrawTask* mTask;         // _BC
	ObjModel mModel;     // _C0
	ObjDrawState mState; // _E0 the model's ambient only fading from white
	Fx* mFx;           // _104
	f32 mFxMatrix[12];   // _108
	TParticleGenerator* mGenA; // _138
	TParticleGenerator* mGenB; // _13C
	TCamPath* mCamera;   // _140
	int mFading;         // _144
	u8 mFadeStep;        // _148
	int _14C;            // _14C
	u8 _150[4];          // _150
	int mSounded;        // _154
	PointLight mLight;   // _158 at the emblem's node on the player's lights
};

// The shine's colour the alpha is set when an emblem is built
struct EmblemColor {
	u8 r, g, b, a;
};

extern "C" {
extern TArc* gEmblemArc;
extern NJS_OBJECT* gEmblemModel;
extern NJS_MOTION** gEmblemMotions;
extern NJS_TEXLIST* gEmblemTexList;
extern MotionEntry sMsnEmblemMotions[];
extern EmblemColor sEmblemTint;
extern const char* gTEmblemGetName;
}

#endif
