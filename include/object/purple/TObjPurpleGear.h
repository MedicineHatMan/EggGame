#ifndef _TOBJPURPLEGEAR_H
#define _TOBJPURPLEGEAR_H
struct TArc;
struct SetDesc;
struct TObject;

#include "stage/SetActor.h"
#include "object/ObjModel.h"
#include "engine/DrawTask.h"
#include "Ninja/ninja.h"

extern "C" void objHeapFree(void*, void*);

/**
 * A gear that only spins. Set int 0 picks the model and is clamped to 0 or 1: SMALL is MODEL_GEAR_S,
 * BIG is MODEL_GEAR_L. Set int 1 is added to the roll each frame, and set float 0 is added to a scale
 * of 1. The roll is on X. Past 900 it deletes itself. The draw cull is 800, with no depth sort.
 *
 * @size 0x110
 */
class TObjPurpleGear : public TSetActor {
public:
	TObjPurpleGear(TObject* parent, SetDesc* desc);
	virtual ~TObjPurpleGear();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void*);           // _34
	virtual void vfn_74();                    // _74

	NJS_OBJECT* mModelData;    // _64 MODEL_GEAR_S or MODEL_GEAR_L
	NJS_TEXLIST* mTexList;      // _68
	DrawTask* mDrawTask; // _6C list 0x21
	ObjModel mModel;     // _70
	f32 mMatrix[12];     // _90 placed turned by mRot then scaled
	f32 mSpin[12];       // _C0 mMatrix rolled on X by mAngle
	int mAngle;          // _F0
	int mKind;           // _F4 0 = small 1 = large
	int mRotSpeed;       // _F8 set int 1 added to mAngle each frame
	f32 mScale;          // _FC 1 plus set float 0
	Vec3f mCenter;       // _100 the model's centre moved by mSpin
	f32 mRadius;         // _10C the mesh radius times mScale
};

extern "C" {
void loadDarkGateAssets();
void releaseDarkGateAssets();
void resetDarkGateAssets();
}

extern "C" {
TObject* createTObjPurpleGear(TObject*, SetDesc*);
}

extern "C" {
int prepareDarkGateAssets();
}

extern "C" {
extern char* sPurpleGearKindNames[2];
}

#endif
