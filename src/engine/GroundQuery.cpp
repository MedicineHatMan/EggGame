#include "stage/Stage.h"
#include "object/ObjCommon.h"
#include "engine/MeshColli.h"
#include "engine/WorldMesh.h"
#include "engine/PortalVis.h"
#include "engine/GroundQuery.h"
#include "Ninja/ninja.h"
#include "core/MathUtil.h"
#include "egg/EggCreature.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "core/Mem.h"
#include "engine/MeshPool.h"
struct EggHit;
struct MeshSphere;
struct Vec3f;

struct ContactHit {
	ContactHit(); // 0x8000B320 empty

	Vec3f mPos; // _00
	f32 mDist;  // _0C
	u16 mTri;   // _10
	u16 _12;
	MeshColli* mMesh; // _14
};

struct ContactSet {
	ContactHit mHits[3]; // _00
	FlagWord mFound;     // _48
	u32 mMask;           // _4C
	f32 mNearest;        // _50
};

/** One slot of what a probe returns */
struct GroundResult {  // size 0x4C
	Vec3f mCorners[3]; // _00
	Vec3f mPos;        // _24
	Vec3f mNormal;     // _30
	f32 mDist;         // _3C
	u32 mMask;         // _40
	MeshColli* mMesh;  // _44
	u16 mTri;          // _48
};

// What the pool query callbacks add to
struct MeshGather {
	MeshList* mList;   // _00
	int mCount;        // _04
	const void* mArg0; // _08
};

// The segment query's with the second end
struct MeshGatherPair : public MeshGather {
	const void* mArg1; // _0C
};

extern "C" {
}

MeshColli* gWorldMesh;
void* gMovingMeshes;

// 0x8000AC68
// Chooses a slot by surface
ContactHit* contactPickSlot(ContactSet* set, u32 mask, f32 dist)
{
	ContactHit* hit;
	if (mask & 4) {
		hit = &set->mHits[1];
		flagWordSetBit(&set->mFound, 1);
	} else if (mask & 0x98) {
		hit = &set->mHits[2];
		flagWordSetBit(&set->mFound, 2);
	} else if (dist < set->mNearest) {
		hit = &set->mHits[0];
		flagWordSetBit(&set->mFound, 0);
		set->mNearest = dist;
	} else {
		return NULL;
	}
	return hit;
}

// 0x8000AD0C
// Copies each slot that was found out to the caller with the triangle's corners and normal
void contactCopyResults(void* result, ContactSet* set)
{
	*(u32*)((u8*)result + 0xE4) = set->mFound.mBits;
	for (int i = 0; i < 3; i++) {
		if (!((1 << i) & set->mFound.mBits)) {
			continue;
		}
		GroundResult* out = &((GroundResult*)result)[i];
		ContactHit* hit   = &set->mHits[i];
		MeshColli* mesh   = hit->mMesh;
		Vec3f* verts      = mesh->mVerts;
		MeshTri* tri      = &mesh->mTris[hit->mTri];
		if (mesh->mFlags & 1) {
			Vec3f v;
			for (int k = 0; k < 3; k++) {
				v = verts[tri->mVerts[k]];
				mtxMultVec(mesh->mMatrix, &v, &out->mCorners[k]);
			}
			out->mPos = hit->mPos;
			v.x       = tri->mNormal.x;
			v.y       = tri->mNormal.y;
			v.z       = tri->mNormal.z;
			mtxMultVecSR(mesh->mMatrix, &v, &out->mNormal);
		} else {
			out->mCorners[0] = verts[tri->mVerts[0]];
			out->mCorners[1] = verts[tri->mVerts[1]];
			out->mCorners[2] = verts[tri->mVerts[2]];
			out->mPos        = hit->mPos;
			out->mNormal     = tri->mNormal;
		}
		out->mMask = tri->mMask;
		out->mDist = hit->mDist;
		out->mMesh = hit->mMesh;
		out->mTri  = hit->mTri;
	}
}

// 0x8000AEF4
// A callback that takes one triangle hit
int contactTakeHit(MeshColli* mesh, int tri, f32 dist, const Vec3f* pos, void* arg)
{
	ContactSet* set = (ContactSet*)arg;
	ContactHit* hit = contactPickSlot(set, set->mMask & ((u32*)&mesh->mTris[tri & 0xFFFF])[2], dist);
	if (hit != NULL) {
		hit->mPos  = *pos;
		hit->mDist = dist;
		hit->mTri  = tri;
		hit->mMesh = mesh;
	}
	return 1;
}

// 0x8000AF8C
u32 groundQueryDown(const void* box, void* a, void* b, void* c)
{
	u8 list[12];
	queryListBegin(queryPoolGet(), (QueryList*)list);
	if (queryGatherSphere((MeshList*)list, box) != 0) {
		return meshListQueryDown((MeshList*)list, (const Vec3f*)box, (u32)a, (MeshHitFn)b, c);
	}
	return 0;
}

// 0x8000B000
int meshListQueryDown(const MeshList* list, const Vec3f* pos, u32 mask, MeshHitFn fn, void* arg)
{
	int count;
	MeshEntry* data;

	data      = list->mData;
	count     = list->mCount;
	int total = 0;
	for (int i = 0; i < count; i++) {
		total += data[i].mMesh->queryDown(&data[i], pos, mask, fn, arg);
	}
	return total;
}

// 0x8000B08C
// The base meshes have no triangles
int MeshColli::queryDown(const MeshEntry*, const Vec3f*, u32, MeshHitFn, void*)
{
	return 0;
}

// 0x8000B094
u32 groundQueryLine(const void* from, const void* to, void* a, void* b, void* c)
{
	u8 list[12];
	u8 box[24];
	boxAroundLine((f32*)box, (const Vec3f*)from, (const Vec3f*)to);
	queryListBegin(queryPoolGet(), (QueryList*)list);
	if (queryGatherBox((MeshList*)list, box) != 0) {
		return meshListQueryLine((MeshList*)list, (const Vec3f*)from, (const Vec3f*)to, (u32)a, (MeshHitFn)b, c);
	}
	return 0;
}

// 0x8000B120
int meshListQueryLine(const MeshList* list, const Vec3f* from, const Vec3f* to, u32 mask, MeshHitFn fn, void* arg)
{
	int count;
	MeshEntry* data;

	data      = list->mData;
	count     = list->mCount;
	int total = 0;
	for (int i = 0; i < count; i++) {
		total += data[i].mMesh->queryLine(&data[i], from, to, mask, fn, arg);
	}
	return total;
}

// 0x8000B1B4
int MeshColli::queryLine(const MeshEntry*, const Vec3f*, const Vec3f*, u32, MeshHitFn, void*)
{
	return 0;
}

// 0x8000B1BC
u32 meshColliProbe(EggHit* out, const Vec3f* pos, u32 mask)
{
	u8 list[12];
	queryListBegin(queryPoolGet(), (QueryList*)list);
	if (queryGatherSphere((MeshList*)list, pos) != 0) {
		return meshListProbeDown(out, (MeshList*)list, pos, mask);
	}
	return 0;
}

// 0x8000B228
int meshListProbeDown(EggHit* out, const MeshList* list, const Vec3f* pos, u32 mask)
{
	int total = 0;
	ContactSet set;
	set.mNearest = gFarRayDist + 1.0f;
	set.mFound   = FlagWord(0);
	set.mMask    = mask;
	for (int i = 0; i < list->mCount; i++) {
		total += list->mData[i].mMesh->queryDown(&list->mData[i], pos, mask, contactTakeHit, &set);
	}
	contactCopyResults(out, &set);
	return total;
}

// 0x8000B320
ContactHit::ContactHit()
{
}

// 0x8000B324
u32 meshColliProbeLine(EggHit* out, const Vec3f* from, const Vec3f* to, u32 mask)
{
	u8 list[12];
	queryListBegin(queryPoolGet(), (QueryList*)list);
	if (queryGatherLine((MeshList*)list, from, to) != 0) {
		return meshListProbeLine(out, (MeshList*)list, from, to, mask);
	}
	return 0;
}

// 0x8000B39C
u32 meshListProbeLine(EggHit* out, const MeshList* list, const Vec3f* from, const Vec3f* to, u32 mask)
{
	int total = 0;
	ContactSet set;
	set.mNearest = gFarRayDist + 1.0f;
	set.mFound   = FlagWord(0);
	set.mMask    = mask;
	for (int i = 0; i < list->mCount; i++) {
		total += list->mData[i].mMesh->queryLine(&list->mData[i], from, to, mask, contactTakeHit, &set);
	}
	contactCopyResults(out, &set);
	return total;
}

// 0x8000B49C
u32 groundQuerySphere(const void* pos, void* a, void* b, void* c)
{
	u8 list[12];
	queryListBegin(queryPoolGet(), (QueryList*)list);
	if (queryGatherBall((MeshList*)list, pos) != 0) {
		return meshListQuerySphere((MeshList*)list, (const MeshSphere*)pos, a, b, (u32)c);
	}
	return 0;
}

// 0x8000B510
int meshListQuerySphere(const MeshList* list, const MeshSphere* sphere, void* a, void* b, u32 mask)
{
	int count;
	MeshEntry* data;

	data      = list->mData;
	count     = list->mCount;
	int total = 0;
	for (int i = 0; i < count; i++) {
		total += data[i].mMesh->querySphere(&data[i], sphere, mask, a, b);
	}
	return total;
}

// 0x8000B59C
// A mesh with no spheres to test
int MeshColli::querySphere(const MeshEntry*, const MeshSphere*, u32, void*, void*)
{
	return 0;
}

// 0x8000B5A4
extern "C" u32 meshListQuerySphereEx(const MeshList* list, const MeshSphere* sphere, void* a, void* b, void* c, void* d, u32 mask)
{
	int count;
	MeshEntry* data;

	data      = list->mData;
	count     = list->mCount;
	int total = 0;
	for (int i = 0; i < count; i++) {
		total += data[i].mMesh->querySphereEx(&data[i], sphere, mask, a, b, c, d);
	}
	return total;
}

// 0x8000B640
int MeshColli::querySphereEx(const MeshEntry*, const MeshSphere*, u32, void*, void*, void*, void*)
{
	return 0;
}

// 0x8000B648
int queryGatherWorldSphere(MeshList* list, const void* box)
{
	u8 entries[12];
	queryEntryBegin((QueryList*)list, (QueryEntry*)entries);
	int count = gWorldMesh->gatherSphere((MeshEntry*)entries, box);
	if (count != 0) {
		queryListAdd((QueryList*)list, (const QueryEntry*)entries);
	}
	return count;
}

// 0x8000B6B0
int queryGatherWorldLine(MeshList* list, const void* from, const void* to)
{
	u8 entries[12];
	queryEntryBegin((QueryList*)list, (QueryEntry*)entries);
	int count = gWorldMesh->gatherLine((MeshEntry*)entries, from, to);
	if (count != 0) {
		queryListAdd((QueryList*)list, (const QueryEntry*)entries);
	}
	return count;
}

// 0x8000B720
int queryGatherWorldBox(MeshList* list, const void* box)
{
	u8 entries[12];
	queryEntryBegin((QueryList*)list, (QueryEntry*)entries);
	int count = gWorldMesh->gatherBox((MeshEntry*)entries, box);
	if (count != 0) {
		queryListAdd((QueryList*)list, (const QueryEntry*)entries);
	}
	return count;
}

// 0x8000B788
int queryGatherWorldBall(MeshList* list, const void* box)
{
	u8 entries[12];
	queryEntryBegin((QueryList*)list, (QueryEntry*)entries);
	int count = gWorldMesh->gatherBall((MeshEntry*)entries, box);
	if (count != 0) {
		queryListAdd((QueryList*)list, (const QueryEntry*)entries);
	}
	return count;
}

// 0x8000B7F0
void queryGatherMeshSphere(MeshColli* mesh, MeshGather* gather)
{
	u8 entries[12];
	queryEntryBegin((QueryList*)gather->mList, (QueryEntry*)entries);
	int count = mesh->gatherSphere((MeshEntry*)entries, gather->mArg0);
	if (count != 0) {
		queryListAdd((QueryList*)gather->mList, (const QueryEntry*)entries);
	}
	gather->mCount += count;
}

// 0x8000B864
int MeshColli::gatherSphere(MeshEntry*, const void*)
{
	return 0;
}

// 0x8000B86C
void queryGatherMeshLine(MeshColli* mesh, MeshGatherPair* gather)
{
	u8 entries[12];
	queryEntryBegin((QueryList*)gather->mList, (QueryEntry*)entries);
	int count = mesh->gatherLine((MeshEntry*)entries, gather->mArg0, gather->mArg1);
	if (count != 0) {
		queryListAdd((QueryList*)gather->mList, (const QueryEntry*)entries);
	}
	gather->mCount += count;
}

// 0x8000B8E4
int MeshColli::gatherLine(MeshEntry*, const void*, const void*)
{
	return 0;
}

// 0x8000B8EC
void queryGatherMeshBall(MeshColli* mesh, MeshGather* gather)
{
	u8 entries[12];
	queryEntryBegin((QueryList*)gather->mList, (QueryEntry*)entries);
	int count = mesh->gatherBall((MeshEntry*)entries, gather->mArg0);
	if (count != 0) {
		queryListAdd((QueryList*)gather->mList, (const QueryEntry*)entries);
	}
	gather->mCount += count;
}

// 0x8000B960
int MeshColli::gatherBall(MeshEntry*, const void*)
{
	return 0;
}

// 0x8000B968
void queryGatherMeshBox(MeshColli* mesh, MeshGather* gather)
{
	u8 entries[12];
	queryEntryBegin((QueryList*)gather->mList, (QueryEntry*)entries);
	int count = mesh->gatherBox((MeshEntry*)entries, gather->mArg0);
	if (count != 0) {
		queryListAdd((QueryList*)gather->mList, (const QueryEntry*)entries);
	}
	gather->mCount += count;
}

// 0x8000B9DC
int MeshColli::gatherBox(MeshEntry*, const void*)
{
	return 0;
}

// 0x8000B9E4
void queryBegin(void* list)
{
	queryListBegin(queryPoolGet(), (QueryList*)list);
}

static inline int queryGatherMovingSphere(MeshList* list, const void* pos)
{
	MeshGather gather;
	u8 box[24];
	gather.mList  = list;
	gather.mCount = 0;
	gather.mArg0  = pos;
	boxAroundPoint((f32*)box, (const Vec3f*)pos);
	meshPoolQuery((MeshPool*)gMovingMeshes, (const f32*)box, (MeshPoolFn)queryGatherMeshSphere, &gather);
	return gather.mCount;
}

// 0x8000BA18
u32 queryGatherSphere(MeshList* list, const void* pos)
{
	int count = 0;
	count += queryGatherWorldSphere(list, pos);
	count += queryGatherMovingSphere(list, pos);
	return count;
}

static inline int queryGatherMovingLine(MeshList* list, const void* from, const void* to)
{
	MeshGatherPair gather;
	u8 box[24];
	gather.mList  = list;
	gather.mCount = 0;
	gather.mArg0  = from;
	gather.mArg1  = to;
	boxAroundLine((f32*)box, (const Vec3f*)from, (const Vec3f*)to);
	meshPoolQuery((MeshPool*)gMovingMeshes, (const f32*)box, (MeshPoolFn)queryGatherMeshLine, &gather);
	return gather.mCount;
}

// 0x8000BA8C
u32 queryGatherLine(MeshList* list, const void* from, const void* to)
{
	int count = 0;
	count += queryGatherWorldLine(list, from, to);
	count += queryGatherMovingLine(list, from, to);
	return count;
}

static inline int queryGatherMovingBall(MeshList* list, const void* pos)
{
	MeshGather gather;
	u8 box[24];
	gather.mList  = list;
	gather.mCount = 0;
	gather.mArg0  = pos;
	boxAroundSphere((f32*)box, (const MeshSphere*)pos);
	meshPoolQuery((MeshPool*)gMovingMeshes, (const f32*)box, (MeshPoolFn)queryGatherMeshBall, &gather);
	return gather.mCount;
}

// 0x8000BB0C
u32 queryGatherBall(MeshList* list, const void* pos)
{
	int count = 0;
	count += queryGatherWorldBall(list, pos);
	count += queryGatherMovingBall(list, pos);
	return count;
}

static inline int queryGatherMovingBox(MeshList* list, const void* box)
{
	MeshGather gather;
	gather.mList  = list;
	gather.mCount = 0;
	gather.mArg0  = box;
	meshPoolQuery((MeshPool*)gMovingMeshes, (const f32*)box, (MeshPoolFn)queryGatherMeshBox, &gather);
	return gather.mCount;
}

// 0x8000BB80
u32 queryGatherBox(MeshList* list, const void* box)
{
	int count = 0;
	count += queryGatherWorldBox(list, box);
	count += queryGatherMovingBox(list, box);
	return count;
}

// 0x8000BBE8
void groundInit()
{
	queryPoolInit();
	portalWorkInit();
	gWorldMesh     = new WorldMesh;
	MeshPool* pool = new MeshPool;
	if (pool != NULL) {
		pool = meshPoolInit(pool, 200);
	}
	gMovingMeshes = pool;
}

// 0x8000BC48
void groundShutdown()
{
	meshPoolDestroy((MeshPool*)gMovingMeshes, 1);
	delete gWorldMesh;
	queryPoolShutdown();
	portalWorkShutdown();
}

// 0x8000BC98
void groundSetWorldData(void* data)
{
	worldMeshSetData((WorldMesh*)gWorldMesh, (WorldData*)data);
}

// 0x8000BCC0
// The corners of a triangle in world space
void meshTriCorners(MeshColli* mesh, int tri, Vec3f* a, Vec3f* b, Vec3f* c)
{
	MeshTri* t = &mesh->mTris[tri & 0xFFFF];
	if (mesh->mFlags & 1) {
		*a = mesh->mVerts[t->mVerts[0]];
		*b = mesh->mVerts[t->mVerts[1]];
		*c = mesh->mVerts[t->mVerts[2]];
		mtxMultVec(mesh->mMatrix, a, a);
		mtxMultVec(mesh->mMatrix, b, b);
		mtxMultVec(mesh->mMatrix, c, c);
	} else {
		*a = mesh->mVerts[t->mVerts[0]];
		*b = mesh->mVerts[t->mVerts[1]];
		*c = mesh->mVerts[t->mVerts[2]];
	}
}

// 0x8000BE2C
// A triangle's normal in world space
void meshTriNormal(MeshColli* mesh, int tri, Vec3f* out)
{
	u32 flags = mesh->mFlags;
	if (flags & 1) {
		*out = mesh->mTris[tri & 0xFFFF].mNormal;
		if (flags & 4) {
			mtxMultVecSR(mesh->mMatrix, out, out);
			if (njUnitVector(out) < gUnitVectorMin) {
				*out = gVecZero;
			}
		} else {
			mtxMultVecSR(mesh->mMatrix, out, out);
		}
	} else {
		*out = mesh->mTris[tri & 0xFFFF].mNormal;
	}
}

// 0x8000BF10
// One edge of a triangle in world space
void meshTriEdge(MeshColli* mesh, u16 tri, u16 edge, Vec3f* a, Vec3f* b)
{
	MeshTri* t = &mesh->mTris[tri & 0xFFFF];
	if (mesh->mFlags & 1) {
		*a = mesh->mVerts[t->mVerts[edge]];
		*b = mesh->mVerts[t->mVerts[(edge + 1) % 3]];
		mtxMultVec(mesh->mMatrix, a, a);
		mtxMultVec(mesh->mMatrix, b, b);
	} else {
		*a = mesh->mVerts[t->mVerts[edge]];
		*b = mesh->mVerts[t->mVerts[(edge + 1) % 3]];
	}
}

// 0x8000C05C
void meshListWalk(MeshList* list, void* a, void* b)
{
	int i    = 0;
	int more = 1;
	while (i < list->mCount && more != 0) {
		list->mData[i].mMesh->walk(&list->mData[i], a, b, &more);
		i++;
	}
}

// 0x8000C0E8
// Nothing to walk
void MeshColli::walk(const MeshEntry*, void*, void*, int*)
{
}

// 0x8000C0EC
void groundWalkGroup(void* a, void* b, void* c)
{
	worldMeshWalkGroup((WorldMesh*)gWorldMesh, (int)a, b, c);
}

// 0x8000C124
void* meshColliCreate()
{
	return meshPoolCreate((MeshPool*)gMovingMeshes);
}

// 0x8000C148
void meshColliFree(void* mesh)
{
	meshPoolFree((MeshPool*)gMovingMeshes, (MeshObject*)mesh);
}

// 0x8000C170
void groundUpdateMoving()
{
	meshPoolUpdate((MeshPool*)gMovingMeshes);
}

// 0x8000C194
FlagWord::FlagWord()
{
	mBits = 0;
}

// 0x8000C1A0
FlagWord::FlagWord(int value)
{
	mBits = value;
}

// 0x8000C1A8
void FlagWord::operator=(const FlagWord& other)
{
	mBits = other.mBits;
}

// 0x8000C1B4
void flagWordSetBit(FlagWord* word, int bit)
{
	word->mBits |= 1 << bit;
}

// A global with nothing to build
struct GroundQueryStatic {
	GroundQueryStatic() { }
};
static GroundQueryStatic sGroundQueryStatic;
