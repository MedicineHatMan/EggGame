#ifndef _OBJECT_TOBJBRKSCAFFOLD_H_
#define _OBJECT_TOBJBRKSCAFFOLD_H_
#include "Ninja/ninja.h"
#include "Ninja/njtypes.h"
#include "engine/DrawTask.h"
#include "engine/MeshColli.h"
#include "engine/ModelDraw.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjBrkScaffold;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjBrkScaffold(TObject*, SetDesc*);
}

/**
 * A scaffold that breaks. Stage object model 53 with texture list 47, standing on collision mesh 6.
 * Vtable 0x80266E60.
 *
 * @size 0x104
 */
class TObjBrkScaffold : public TSetActor {
public:
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
	TObjBrkScaffold(TObject* parent, SetDesc* desc);
	virtual ~TObjBrkScaffold();

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);

	void stateWait();
	void stateHit();
	void shatter();

	NJS_OBJECT* mObject;   // _64
	NJS_TEXLIST* mTexList; // _68
	Vec3f mCenter;         // _6C the model's centre in the world
	f32 mRadius;           // _78 the model's bounding radius plus 10
	void* mMesh;           // _7C
	MeshObject* mMeshColli;      // _80
	DrawTask* mDrawTask;   // _84
	ModelDraw mDraw;       // _88
	ObjDrawState mState;   // _A4
	f32 mMatrix[12];       // _C8
	int mHits;             // _F8 hits left
	u16 mBlink;            // _FC frames left in stateHit
	int mAct;              // _100 0 wait 1 hit 2 shatter
};

extern "C" {
extern const char* gTObjBrkScaffoldName;
}

#endif
