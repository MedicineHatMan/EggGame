#ifndef _TOBJREDMAGMASHOT_H
#define _TOBJREDMAGMASHOT_H
struct SetDesc;
struct TObject;

#include "stage/SetActor.h"
#include "object/ObjModel.h"
#include "engine/MotionPlayer.h"
#include "engine/DrawTask.h"

extern "C" void objHeapFree(void*, void*);

/**
 * A magma shot. Set int 0 is WAIT (0, a contact) or ALWAYS (1, a timer), set int 1 is the damage and set int 2
 * is the wait in frames. Set float 0 is added to the cylinder radius. It sits until the wait elapses or something
 * touches it, then rises until the offset passes a fixed height, and starts again.
 *
 * @size 0x120
 */
class TObjRedMagmaShot : public TSetActor {
public:
	TObjRedMagmaShot(TObject* parent, SetDesc* desc);
	virtual ~TObjRedMagmaShot();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void*);           // _34
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58
	virtual void setup();                     // _74

	// idle waits rise moves the shot up
	void makeShapes();
	void placeShapes();
	void updateMatrix();
	void idle();
	void rise();

	DrawTask* mDrawTask;     // _64
	ObjModel mModel;         // _68
	MotionPlayer mMotion;    // _88
	f32 mMatrix[12];         // _C4
	int _F4;                 // _F4
	int _F8;                 // _F8
	int mType;               // _FC set int 0 clamped to 0 or 1
	int mPower;              // _100 set int 1 written into the hit
	int mWait;               // _104 set int 2 frames before an ALWAYS shot rises
	f32 mRadius;             // _108 set float 0 plus a constant
	f32 mTop;                // _10C the offset y that ends the rise
	Vec3f mOffset;           // _110 added to mHome y climbs while rising
	s16 mState;              // _11C 0 idle 1 rising
	s16 mTimer;              // _11E
};

extern "C" {
TObject* createTObjRedMagmaShot(TObject*, SetDesc*);
}

#include "engine/CharaColli.h"
extern "C" {
extern const char* sRedMagmaShotTypeNames[];
extern ColliShape sRedMagmaShotShapes[];
extern int sRedMagmaShotVisible;
extern const MotionEntry sRedMagmaShotMotion;
}

#endif
