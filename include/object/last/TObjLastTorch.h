#ifndef _OBJECT_LAST_TOBJLASTTORCH_H
#define _OBJECT_LAST_TOBJLASTTORCH_H
#include "Ninja/ninja.h"
#include "enemy/EnemyLight.h"
#include "engine/DrawTask.h"
#include "engine/ModelDraw.h"
#include "engine/TParticle.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjLastTorch;

#include "core/types.h"

class TObjLastTorch : public TSetActor {
public:
	TObjLastTorch(TObject* parent, SetDesc* desc);
	virtual ~TObjLastTorch();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void*);           // _34
	virtual void vfn_74();                    // _74 sets up the model and the mode

	void placeFlame();
	void stateUnlit();
	void stateLighting();
	void stateLit();

	NJS_OBJECT* mModel;  // _64
	DrawTask* mDraw;     // _68
	ModelDraw mMesh;     // _6C
	ObjDrawState mState; // _88
	MotionTrack mTrack;  // _AC
	f32 mMtx[12];        // _C8
	f32 mFlameMtx[12];   // _F8 the flame node's caught while the model is walked
	u16 mSwitchId;       // _128
	u16 _12A;
	int mEventId;               // _12C
	TParticleGenerator* mFlame; // _130
	Vec3f mCenter;              // _134
	f32 mRadius;                // _140
	EnemyLight mLight;          // _144
	int mMode;                  // _184 a LastTorchMode
};

extern "C" {
extern const char* gTObjLastTorchName;
}

#endif
