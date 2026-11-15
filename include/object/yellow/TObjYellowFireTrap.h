#ifndef _OBJECT_YELLOW_TOBJYELLOWFIRETRAP_H_
#define _OBJECT_YELLOW_TOBJYELLOWFIRETRAP_H_
#include "Ninja/ninja.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "object/ObjAction.h"
#include "object/ObjModel.h"
#include "sound/SoundHandle.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
class TParticleGenerator;
struct TObjYellowFireTrap;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjYellowFireTrap(TObject*, SetDesc*);
}

class TObjYellowFireTrap : public TSetActor {
public:
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
	TObjYellowFireTrap(TObject* parent, SetDesc* desc);
	virtual ~TObjYellowFireTrap();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual int onHit(const TObjAction* other, int, int* info);
	virtual void vfn_74();

	void makeShapes();
	void updateMatrix();
	void stateOn();
	void stateOff();

	ObjModel mModel;            // _64
	DrawTask* mDrawTask;        // _84
	CharaColli* mJetColli;      // _88 seven spheres along the flame
	f32 mMatrix[12];            // _8C
	int mEternal;               // _BC
	int mPower;                 // _C0
	int mOnTime;                // _C4
	int mOffTime;               // _C8
	int mTimer;                 // _CC
	SoundHandle mSound;         // _D0
	int mAct;                   // _FC 0 = on 1 = off 2 = always on
	TParticleGenerator* mTrail; // _100
};

extern "C" {
extern int sYellowFireTrapVisible;
}

#endif
