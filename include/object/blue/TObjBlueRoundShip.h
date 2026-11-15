#ifndef _TOBJBLUEROUNDSHIP_H
#define _TOBJBLUEROUNDSHIP_H
struct SetDesc;
struct TObject;

#include "engine/DrawTask.h"
#include "engine/MotionPlayer.h"
#include "egg/ObjEgg.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"

extern "C" void objHeapFree(void*, void*);

/**
 * A round ship. Set int 0 picks the kind: 0 stays put ("FIX"), 1 rides a path ("PATH") and
 * hangs three particles on model nodes 1, 2 and 3.
 *
 * @size 0x128
 */
class TObjBlueRoundShip : public TSetActor {
public:
	TObjBlueRoundShip(TObject* parent, SetDesc* desc);
	virtual ~TObjBlueRoundShip();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void*);           // _34
	virtual void vfn_74();                    // _74

	void setupFixed();
	void setupPath();
	void ridePath();

	DrawTask* mDraw;      // _64
	ObjModel mModel;      // _68
	MotionPlayer mMotion; // _88
	EggCollide mRail;     // _C4
	int _E0;              // _E0 only stored as 0
	f32 mMatrix[12];      // _E4
	int mAimYaw;          // _114
	void* mPart[3];       // _118
	int mType;            // _124 0 fixed 1 on a path
};

extern "C" void objHeapFree(void*, void*);

extern "C" {
TObject* createTObjBlueRoundShip(TObject*, SetDesc*);
}

extern "C" {
extern const char* gTObjBlueRoundShipName;
extern char* sRoundShipTypeNames[2];
extern int sRoundShipVisible;
}

#endif
