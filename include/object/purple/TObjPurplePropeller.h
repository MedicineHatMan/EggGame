#ifndef _TOBJPURPLEPROPELLER_H
#define _TOBJPURPLEPROPELLER_H
struct SetDesc;
struct TObject;

#include "stage/SetActor.h"
#include "object/ObjModel.h"
#include "sound/SoundHandle.h"
#include "engine/DrawTask.h"
#include "Ninja/ninja.h"
#include "io/Resource.h"
class TParticleGenerator;

extern "C" void objHeapFree(void*, void*);

/**
 * A propeller. Set int 0 is the spin step, set ints 2 and 3 are the first and last switch, and set float 0 is added
 * to a scale of 1. It yaws until every switch in that range is set, then the snow it was blowing stops.
 *
 * @size 0x148
 */
class TObjPurplePropeller : public TSetActor {
public:
	TObjPurplePropeller(TObject* parent, SetDesc* desc);
	virtual ~TObjPurplePropeller();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual void vfn_74();

	int switchesSet();
	void spin();
	void stopSnow();
	void stopped();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	void* mGeo;                 // _64 MODEL_PROPELLER
	void* mTex;                 // _68 TLS_MODEL_PROPELLER
	DrawTask* mDrawTask;        // _6C
	ObjModel mModel;            // _70
	f32 mMatrix[12];            // _90 placed scaled then pitched
	f32 mSpin[12];              // _C0 mMatrix yawed by mAngle
	f32 mScale;                 // _F0 1 plus set float 0
	int mAngle;                 // _F4
	int mRotSpeed;              // _F8 set int 0
	TParticleGenerator* mSnow0; // _FC particle 0xF5
	TParticleGenerator* mSnow1; // _100 particle 0xF6
	u16 mFrom;                  // _104 set int 2
	u16 mTo;                    // _106 set int 3
	SoundHandle mSound;         // _108
	Vec3f mCentre;              // _134
	f32 mRadius;                // _140
	int mAct;                   // _144 0 spinning 1 stopping the snow 2 stopped
};

extern "C" {
TObject* createTObjPurplePropeller(TObject*, SetDesc*);
}

#endif
