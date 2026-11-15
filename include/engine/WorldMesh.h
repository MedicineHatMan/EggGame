#ifndef _ENGINE_WORLDMESH_H_
#define _ENGINE_WORLDMESH_H_
#include "engine/MeshColli.h"
#include "engine/WorldVector.h"
struct WorldGroup;
struct WorldTreeNode;
struct QueryEntry;
struct QueryList;
struct WorldData;
struct MeshEntry;
struct MeshSphere;
struct QueryPool;
struct Rider;
struct RiderList;
struct WorldGather;
struct WorldMesh;

#include "core/types.h"

class MeshColli;
struct MeshTri;
struct Vec3f;

extern "C" {
int edgeSphereTest(f32, const Vec3f*, const Vec3f*, const Vec3f*, Vec3f*);
f32 triDownHitParam(f32, const Vec3f*, const Vec3f*, const Vec3f*);
int triAnyVertexBelow(const Vec3f*, const Vec3f*, const Vec3f*, const Vec3f*);
int triContainsXZ(const Vec3f*, const Vec3f*, const Vec3f*, const Vec3f*);
void meshDropRiders(MeshColli*);
f32 pointDistanceSq(const Vec3f*, const Vec3f*);
void queryPoolInit();
void queryPoolShutdown();
int triContainsPoint(const Vec3f*, const Vec3f*, const Vec3f*, const Vec3f*, const Vec3f*);
int triGroundBelow(Vec3f*, MeshTri*, const Vec3f*, Vec3f*, f32*);
int triLineCross(Vec3f*, MeshTri*, const Vec3f*, const Vec3f*, Vec3f*, f32*);
int triSphereCorner(f32, const Vec3f*, const Vec3f*, const Vec3f*, const Vec3f*, u16*);
int triSphereEdge(f32, const Vec3f*, const Vec3f*, const Vec3f*, const Vec3f*, u16*, Vec3f*);
}

extern "C" {
void boxAroundLine(f32*, const Vec3f*, const Vec3f*);
void meshColliSetMatrix(MeshColli*, const f32*);
void boxAroundPoint(f32*, const Vec3f*);
void boxAroundSphere(f32*, const MeshSphere*);
void* worldMeshFindGroup(WorldMesh*, int);
int worldMeshGather(WorldMesh*, MeshEntry*, const f32*);
int worldTreeQuery(WorldMesh*, WorldGather*, const f32*);
void worldTreeGather(WorldMesh*, WorldGather*, const f32*, int);
void worldLeavesToTris(WorldMesh*, WorldGather*, MeshEntry*);
QueryPool* queryPoolCtor(QueryPool*);
QueryPool* queryPoolDtor(QueryPool*, s16);
void riderListCtor(RiderList*);
RiderList* riderListDtor(RiderList*, s16);
void riderListAppend(RiderList*, Rider*);
void riderListClear(RiderList*);
void riderListRemove(RiderList*, Rider*);
void riderListUnlinkAll(RiderList*);
}

extern "C" {
void riderSetMesh(Rider*, RiderList*);
void riderDetach(Rider*);
QueryPool* queryPoolGet();
void queryListBegin(QueryPool*, QueryList*);
void queryEntryBegin(QueryList*, QueryEntry*);
void queryListAdd(QueryList*, const QueryEntry*);
void worldMeshSetData(WorldMesh*, WorldData*);
void worldMeshWalkGroup(WorldMesh*, int, void*, void*);
}

struct WorldTreeNode { // size 0x20
	u16 mIsLeaf;      // _00
	u16 mTriCount;    // _02
	u16* mTris;       // _04
	f32 mMinX;        // _08
	f32 mMaxX;        // _0C
	f32 mMinZ;        // _10
	f32 mMaxZ;        // _14
	u16 mChildren[4]; // _18
};

struct WorldGroup { // size 8
	u16 mId;    // _00
	u16 _02;
	u16 mFirst; // _04 the first of its tris
	u16 mCount; // _06
};

struct WorldData {
	Vec3f* mVerts;        // _00
	u32 _04;
	MeshTri* mTris;       // _08
	int mTriCount;        // _0C
	WorldTreeNode* mTree; // _10
	int mLeaves;          // _14
	u8 _18[0x18];
	WorldGroup* mGroups;  // _30
	int mGroupCount;      // _34
};

/** @size 0x98 */
class WorldMesh : public MeshColli {
public:
	WorldMesh();
	virtual ~WorldMesh();
	virtual int gatherSphere(MeshEntry*, const void*);
	virtual int gatherLine(MeshEntry*, const void*, const void*);
	virtual int gatherBall(MeshEntry*, const void*);
	virtual int gatherBox(MeshEntry*, const void*);
	virtual int queryDown(const MeshEntry*, const Vec3f*, u32, MeshHitFn, void*);
	virtual int queryLine(const MeshEntry*, const Vec3f*, const Vec3f*, u32, MeshHitFn, void*);
	virtual int querySphere(const MeshEntry*, const MeshSphere*, u32, void*, void*);
	virtual int querySphereEx(const MeshEntry*, const MeshSphere*, u32, void*, void*, void*, void*);
	virtual void walk(const MeshEntry*, void*, void*, int*);

	WorldData* mData;                 // _7C
	WorldVector<WorldTreeNode*> mHit; // _80
	WorldVector<u8> mSeen;            // _8C
};

struct Rider { // size 0x14 what stands on a mesh
	Rider();  // 0x80077D24
	~Rider(); // 0x80077D40

	Rider* mNext;  // _00
	Rider* mPrev;  // _04
	Rider* mSelf;  // _08
	void* mList;   // _0C
	MeshColli* mMesh; // _10
};

struct RiderList {
	Rider* mTail; // _00
	Rider* mHead; // _04
	int mCount;   // _08
};

struct QueryList {
	QueryEntry* mEntries; // _00
	void* mTris;          // _04
	u16 mCount;           // _08
	u16 mTriCount;        // _0A
};

struct QueryEntry {
	void* mMesh;  // _00
	void* mTris;  // _04
	u32 mCount;   // _08
};

struct QueryPool {
	QueryEntry* mEntries; // _00 0x40 entries
	void* mTris;    // _04
};

extern "C" {
extern QueryPool* sWorldMeshPool;
}

/** How far the ground and line probes reach */
extern const f32 gFarRayDist;

extern "C" {
void worldNodeVecCtor(WorldVector<WorldTreeNode*>* v);
void worldNodeVecAlloc(WorldVector<WorldTreeNode*>* v, int cap);
void worldNodeVecCopy(WorldVector<WorldTreeNode*>* v, WorldTreeNode** to, WorldTreeNode* const* from, int count);
void worldByteVecCtor(WorldVector<u8>* v);
void worldByteVecAlloc(WorldVector<u8>* v, int cap);
void worldByteVecCopy(WorldVector<u8>* v, u8* to, const u8* from, int count);
void worldByteVecResize(WorldVector<u8>* v, int size);
void worldByteVecReserve(WorldVector<u8>* v, int cap);
void worldNodeVecResize(WorldVector<WorldTreeNode*>* v, int size);
void worldNodeVecReserve(WorldVector<WorldTreeNode*>* v, int cap);
}

#endif
