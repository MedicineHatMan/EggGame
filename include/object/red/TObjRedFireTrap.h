#ifndef _TOBJREDFIRETRAP_H
#define _TOBJREDFIRETRAP_H
#include "core/types.h"
struct FireList;
struct FireNode;
struct SetDesc;
struct TObject;
struct CharaColli;

#include "stage/SetActor.h"
#include "object/ObjModel.h"
#include "engine/MotionPlayer.h"
#include "sound/SoundHandle.h"
#include "engine/DrawTask.h"
class TParticleGenerator;

extern "C" {
void objHeapFree(void*, void*);
extern void* gObjHeap;
}

/**
 * A jet of fire that plays a motion as it lights and goes out.
 * The record's first int is the eternal flag: set, and the trap stays on.
 * Power is the second int plus 25, on time the third plus 120, off time the fourth plus 180.
 *
 * @size 0x144
 */
class TObjRedFireTrap : public TSetActor {
public:
	TObjRedFireTrap(TObject* parent, SetDesc* desc);
	virtual ~TObjRedFireTrap();

	virtual void draw();                                            // _0C
	virtual void vfn_10();                                          // _10
	virtual void render();                                          // _1C
	virtual void initFromDesc(SetDesc* desc);                       // _30
	virtual void debugPrint(void*);                                 // _34
	virtual int onHit(const TObjAction* other, int arg, int* info); // _58
	virtual void vfn_74();                                          // _74

	void makeShapes();
	void updateMatrix();
	void stateIgnite(); // mAct 0
	void stateOn();     // mAct 1
	void stateOff();    // mAct 2

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	ObjModel mModel;            // _64
	DrawTask* mDraw;            // _84
	MotionPlayer mMotion;       // _88
	CharaColli* mJetColli;      // _C4 seven spheres along the flame
	f32 mMatrix[12];            // _C8
	int mAct;                   // _F8 0 ignite 1 on 2 off 3 eternal
	int mEternal;               // _FC
	int mPower;                 // _100 dealt to a kind-0 hit
	int _104;                   // _104 nothing in this class reads it
	int mOnTime;                // _108
	int mOffTime;               // _10C
	int mTimer;                 // _110
	TParticleGenerator* mTrail; // _114
	SoundHandle mSound;         // _118
};

extern "C" {
void fireTrapSetExit();
void fireTrapSetInit();
}

extern "C" {
TObject* createTObjRedFireTrap(TObject*, SetDesc*);
}

// A node of the fire list
struct FireNode {
	FireNode* mNext; // _00
	FireNode* mPrev; // _04
	u32 _08;         // _08
	u32 mOwner;      // _0C
};

/**
 * A list the system table creates at start-up and frees at the end. Nothing else uses it in this build.
 * @size 0xC
 */
struct FireList {
	FireList();
	~FireList();
	void clear();

	FireNode* mTail; // _00
	FireNode* mHead; // _04
	u32 mCount;      // _08
};

extern "C" {
extern int sRedFireTrapVisible;
extern FireList* gFireList;
}

#endif
