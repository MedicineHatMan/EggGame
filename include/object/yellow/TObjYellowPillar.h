#ifndef _OBJECT_YELLOW_TOBJYELLOWPILLAR_H_
#define _OBJECT_YELLOW_TOBJYELLOWPILLAR_H_
#include "Ninja/ninja.h"
struct DrawTask;
#include "enemy/EnemyLight.h"
#include "engine/CharaColli.h"
#include "engine/MeshColli.h"
#include "object/ObjAction.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjYellowPillar;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjYellowPillar(TObject*, SetDesc*);
}

class TObjYellowPillar : public TSetActor {
public:
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
	TObjYellowPillar(TObject* parent, SetDesc* desc);
	virtual ~TObjYellowPillar();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual int onHit(const TObjAction* other, int, int* info);
	virtual void vfn_74();

	static void setColliBox(void* colli, const Vec3f* center, f32 radius);

	void updateMatrix();
	void createShapes();
	void updateShapes();
	void freeShapes();
	void initColli();
	void updateColli();
	void freeColli();
	void disableShapes();
	void enableShapes();
	void breakApart(const Vec3f* hit);
	void stateWait();
	void stateFall();
	void stateLanded();

	NJS_OBJECT* mModelData; // _64
	NJS_TEXLIST* mTexList;  // _68
	MeshData* mColliData;   // _6C
	DrawTask* mDrawTask;    // _70
	ObjModel mModel;        // _74
	ObjDrawState mState;    // _94
	f32 mMatrix[12];        // _B8
	CharaColli* mTerritory; // _E8 the cylinder that starts the fall
	MeshObject* mColli;     // _EC
	f32 mTerritoryRad;      // _F0
	int mPower;             // _F4
	Vec3f mCenter;          // _F8
	f32 mRadius;            // _104
	EnemyLight mLight;      // _108
	int mAct;               // _148 0 = wait 1 = fall 2 = landed
};

extern "C" {
extern const char* gTObjYellowPillarName;
}

#endif
