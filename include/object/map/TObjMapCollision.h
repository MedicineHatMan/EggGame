#ifndef _OBJECT_MAP_TOBJMAPCOLLISION_H_
#define _OBJECT_MAP_TOBJMAPCOLLISION_H_
#include "engine/MeshColli.h"
#include "object/ObjAction.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjMapCollision;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjMapCollision(TObject*, SetDesc*);
}

class TObjMapCollision : public TObjAction {
public:
	TObjMapCollision(TObject* parent, const SetDesc* desc);
	virtual ~TObjMapCollision();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual int getKind();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual SetLink* getLink();

	void buildMesh();
	int buildVerts();
	int buildTris();
	void setBox();

	MeshObject* mActor; // _30
	Vec3f* mVerts;      // _34
	MeshTri* mTris;     // _38
	MeshData mData;     // _3C _04 is the vertex count
	f32 mMtx[12];  // _4C
	Vec3f mPos;    // _7C
	SetRot mRot;   // _88
	int mSwitch;   // _94
	s16 mModel;    // _98 0 or 1
	u8 _9A[2];
	Vec3f mScale;  // _9C
	SetLinkMember mLink; // _A8
};

#endif
