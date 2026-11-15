#include "engine/MeshColli.h"
#include "engine/WorldVector.h"
#include "engine/WorldMesh.h"
#include "Ninja/ninja.h"
#include "core/Mem.h"
#include "object/TObjEggRing.h"
#include "core/MathUtil.h"

const f32 gFarRayDist = 100000.0f;

QueryPool* sWorldMeshPool;

// 0x80077CB4
// The box around a line
void boxAroundLine(f32* box, const Vec3f* a, const Vec3f* b)
{
	if (a->x < b->x) {
		box[0] = a->x;
		box[3] = b->x;
	} else {
		box[0] = b->x;
		box[3] = a->x;
	}
	if (a->y < b->y) {
		box[1] = a->y;
		box[4] = b->y;
	} else {
		box[1] = b->y;
		box[4] = a->y;
	}
	if (a->z < b->z) {
		box[2] = a->z;
		box[5] = b->z;
	} else {
		box[2] = b->z;
		box[5] = a->z;
	}
}

// 0x80077D24
Rider::Rider()
{
	mNext = NULL;
	mPrev = NULL;
	mList = NULL;
	mSelf = this;
	mMesh = NULL;
}

// 0x80077D40
Rider::~Rider()
{
	riderDetach(this);
}

// 0x80077D88
// Moves a rider to a new mesh
void riderSetMesh(Rider* rider, RiderList* list)
{
	if (rider->mList != NULL) {
		riderListRemove((RiderList*)rider->mMesh, rider);
	}
	if (list != NULL) {
		riderListAppend(list, rider);
	}
	rider->mMesh = (MeshColli*)list;
}

// 0x80077DE4
void riderDetach(Rider* rider)
{
	if (rider->mList != NULL) {
		riderListRemove((RiderList*)rider->mMesh, rider);
		rider->mMesh = NULL;
	}
}

// 0x80077E2C
MeshColli::MeshColli()
{
	riderListCtor((RiderList*)_00);
	mFlags = 0;
}

// 0x80077E70
MeshColli::~MeshColli()
{
	for (Rider* r = ((RiderList*)_00)->mHead; r != NULL; r = r->mNext) {
		r->mSelf->mMesh = NULL;
	}
	riderListDtor((RiderList*)_00, -1);
}

// 0x80077EEC
void meshColliSetMatrix(MeshColli* mesh, const f32* matrix)
{
	mtxCopy(mesh->mMatrix, matrix);
	mtxInverse(mesh->mInverse, mesh->mMatrix);
}

// 0x80077F28
// Drops everything that stands on this mesh
void meshDropRiders(MeshColli* mesh)
{
	RiderList* list = (RiderList*)mesh->_00;
	for (Rider* r = list->mHead; r != NULL; r = r->mNext) {
		r->mSelf->mMesh = NULL;
	}
	riderListClear(list);
}

// 0x80077F68
// The ground straight below a point
int triGroundBelow(Vec3f* verts, MeshTri* tri, const Vec3f* pos, Vec3f* out, f32* dist)
{
	if (tri->mNormal.y < 1e-06f) {
		return 0;
	}
	const Vec3f* a = &verts[tri->mVerts[0]];
	const Vec3f* b = &verts[tri->mVerts[1]];
	const Vec3f* c = &verts[tri->mVerts[2]];
	if (triAnyVertexBelow(a, b, c, pos) == 0) {
		return 0;
	}
	if (triContainsXZ(a, b, c, pos) == 0) {
		return 0;
	}
	f32 t = triDownHitParam(-100000.0f, a, &tri->mNormal, pos);
	out->set(pos->x, -100000.0f * t + pos->y, pos->z);
	*dist = -100000.0f * -t;
	return 1;
}

static inline Vec3f difference(const Vec3f& a, const Vec3f& b)
{
	Vec3f r;
	gjSubVector(&r, &a, &b);
	return r;
}

// 0x80078064
// Where a line crosses a triangle's front
int triLineCross(Vec3f* verts, MeshTri* tri, const Vec3f* from, const Vec3f* to, Vec3f* out, f32* dist)
{
	Vec3f rel;
	Vec3f dir;
	dir       = difference(*to, *from);
	f32 denom = njInnerProduct(&dir, &tri->mNormal);
	if (denom >= -1e-06f) {
		return 0;
	}
	const Vec3f* a = &verts[tri->mVerts[0]];
	const Vec3f* b = &verts[tri->mVerts[1]];
	const Vec3f* c = &verts[tri->mVerts[2]];
	rel.x          = from->x - a->x;
	rel.y          = from->y - a->y;
	rel.z          = from->z - a->z;
	f32 above      = njInnerProduct(&rel, &tri->mNormal);
	if (above <= 1e-06f) {
		return 0;
	}
	f32 t = -above / denom;
	if (t > 1.0f || t < 0.0f) {
		return 0;
	}
	gjMulAddVector(t, &rel, from, &dir);
	if (triContainsPoint(a, b, c, &rel, &tri->mNormal) == 0) {
		return 0;
	}
	*out  = rel;
	*dist = t * njScalor(&dir);
	return 1;
}

// 0x80078218
f32 triDownHitParam(f32 scale, const Vec3f* a, const Vec3f* normal, const Vec3f* pos)
{
	f32 denom = scale * normal->y;
	Vec3f rel;
	rel.x = a->x - pos->x;
	rel.y = a->y - pos->y;
	rel.z = a->z - pos->z;
	return njInnerProduct(&rel, normal) / denom;
}

// 0x80078288
// Whether a triangle has a corner at or below a height
int triAnyVertexBelow(const Vec3f* a, const Vec3f* b, const Vec3f* c, const Vec3f* pos)
{
	f32 y = pos->y;
	return a->y <= y || b->y <= y || c->y <= y;
}

// 0x800782CC
// Whether a point is inside a triangle looking from above
#pragma bool off
int triContainsXZ(const Vec3f* a, const Vec3f* b, const Vec3f* c, const Vec3f* pos)
{
	f32 ax;
	f32 bx;
	f32 az;
	f32 bz;
	ax = a->x - pos->x;
	az = a->z - pos->z;
	bx = b->x - pos->x;
	bz = b->z - pos->z;
	if (az * bx - ax * bz < 0.0f) {
		return 0;
	}
	f32 cx = c->x - pos->x;
	f32 cz = c->z - pos->z;
	if (bz * cx - bx * cz < 0.0f) {
		return 0;
	}
	return !(cz * ax - cx * az < 0.0f);
}
#pragma bool reset

// 0x80078358
// Whether a point on a triangle's plane is inside it
#pragma bool off
int triContainsPoint(const Vec3f* a, const Vec3f* b, const Vec3f* c, const Vec3f* pos, const Vec3f* normal)
{
	Vec3f va;
	va.x = a->x;
	va.y = a->y;
	va.z = a->z;
	njSubVector(&va, pos);
	Vec3f vb;
	vb.x = b->x;
	vb.y = b->y;
	vb.z = b->z;
	njSubVector(&vb, pos);
	Vec3f vc;
	Vec3f cross;
	njOuterProduct(&va, &vb, &cross);
	if (njInnerProduct(&cross, normal) < 0.0f) {
		return 0;
	}
	vc.x = c->x;
	vc.y = c->y;
	vc.z = c->z;
	njSubVector(&vc, pos);
	njOuterProduct(&vb, &vc, &cross);
	if (njInnerProduct(&cross, normal) < 0.0f) {
		return 0;
	}
	njOuterProduct(&vc, &va, &cross);
	return !(njInnerProduct(&cross, normal) < 0.0f);
}
#pragma bool reset

// 0x8007848C
void boxAroundPoint(f32* box, const Vec3f* pos)
{
	f32 r  = 0.0f;
	box[0] = pos->x - r;
	box[1] = pos->y - r;
	box[2] = pos->z - r;
	box[3] = pos->x + r;
	box[4] = pos->y + r;
	box[5] = pos->z + r;
}

// 0x800784D0
void boxAroundSphere(f32* box, const MeshSphere* sphere)
{
	box[0] = sphere->mPos.x - sphere->mRadius;
	box[1] = sphere->mPos.y - sphere->mRadius;
	box[2] = sphere->mPos.z - sphere->mRadius;
	box[3] = sphere->mPos.x + sphere->mRadius;
	box[4] = sphere->mPos.y + sphere->mRadius;
	box[5] = sphere->mPos.z + sphere->mRadius;
}

// 0x80078514
// The squared distance between two points
f32 pointDistanceSq(const Vec3f* a, const Vec3f* b)
{
	Vec3f d;
	d.x = a->x - b->x;
	d.y = a->y - b->y;
	d.z = a->z - b->z;
	return njScalor2(&d);
}

// 0x80078568
// Whether a sphere touches a corner
int triSphereCorner(f32 radius, const Vec3f* a, const Vec3f* b, const Vec3f* c, const Vec3f* pos, u16* which)
{
	f32 sq = radius * radius;
	if (pointDistanceSq(a, pos) < sq) {
		*which = 0;
		return 1;
	}
	if (pointDistanceSq(b, pos) < sq) {
		*which = 1;
		return 1;
	}
	if (pointDistanceSq(c, pos) < sq) {
		*which = 2;
		return 1;
	}
	return 0;
}

// 0x8007861C
// Whether a sphere touches an edge
#pragma bool off
int edgeSphereTest(f32 radius, const Vec3f* pos, const Vec3f* a, const Vec3f* b, Vec3f* out)
{
	Vec3f edge;
	edge.x = b->x;
	edge.y = b->y;
	edge.z = b->z;
	njSubVector(&edge, a);
	Vec3f rel;
	rel.x = pos->x;
	rel.y = pos->y;
	rel.z = pos->z;
	njSubVector(&rel, a);
	f32 len = njInnerProduct(&edge, &edge);
	f32 t   = njInnerProduct(&rel, &edge) / len;
	if (t < 0.0f || t > 1.0f) {
		return 0;
	}
	gjMulAddVector(t, &rel, a, &edge);
	*out = rel;
	njSubVector(&rel, pos);
	return !(njScalor2(&rel) > radius * radius);
}
#pragma bool reset

// 0x80078748
int triSphereEdge(f32 radius, const Vec3f* a, const Vec3f* b, const Vec3f* c, const Vec3f* pos, u16* which, Vec3f* out)
{
	if (edgeSphereTest(radius, pos, a, b, out) != 0) {
		*which = 0;
		return 1;
	}
	if (edgeSphereTest(radius, pos, b, c, out) != 0) {
		*which = 1;
		return 1;
	}
	if (edgeSphereTest(radius, pos, c, a, out) != 0) {
		*which = 2;
		return 1;
	}
	return 0;
}

// 0x80078820
QueryPool* queryPoolGet()
{
	return sWorldMeshPool;
}

// 0x80078828
void queryPoolInit()
{
	if (sWorldMeshPool == NULL) {
		QueryPool* pool = new QueryPool;
		if (pool != NULL) {
			pool = queryPoolCtor(pool);
		}
		sWorldMeshPool = pool;
	}
}

// 0x8007886C
void queryPoolShutdown()
{
	if (sWorldMeshPool != NULL) {
		queryPoolDtor(sWorldMeshPool, 1);
		sWorldMeshPool = NULL;
	}
}

// 0x800788A4
QueryPool* queryPoolCtor(QueryPool* pool)
{
	pool->mEntries = (QueryEntry*)memAlloc(0x300);
	pool->mTris    = memAlloc(0x800);
	return pool;
}

// 0x800788E8
QueryPool* queryPoolDtor(QueryPool* pool, s16 flags)
{
	if (pool != NULL) {
		memFree(pool->mEntries);
		memFree(pool->mTris);
		if (flags > 0) {
			delete pool;
		}
	}
	return pool;
}

// 0x8007893C
void queryListBegin(QueryPool* pool, QueryList* list)
{
	list->mEntries  = pool->mEntries;
	list->mCount    = 0;
	list->mTriCount = 0;
	list->mTris     = pool->mTris;
}

// 0x8007895C
void queryEntryBegin(QueryList* list, QueryEntry* entry)
{
	entry->mMesh  = NULL;
	entry->mTris  = (u16*)list->mTris + list->mTriCount;
	entry->mCount = 0;
}

// 0x80078980
void queryListAdd(QueryList* list, const QueryEntry* entry)
{
	list->mEntries[list->mCount++] = *entry;
	list->mTriCount += entry->mCount;
}

// 0x800789C0
void riderListCtor(RiderList* list)
{
	list->mHead  = NULL;
	list->mTail  = NULL;
	list->mCount = 0;
}

// 0x800789D4
RiderList* riderListDtor(RiderList* list, s16 flags)
{
	if (list != NULL) {
		riderListUnlinkAll(list);
		if (flags > 0) {
			delete list;
		}
	}
	return list;
}

// 0x80078A1C
void riderListAppend(RiderList* list, Rider* rider)
{
	if (list->mHead == NULL) {
		list->mTail = rider;
		list->mHead = rider;
	} else {
		rider->mPrev       = list->mTail;
		list->mTail->mNext = rider;
		list->mTail        = rider;
	}
	rider->mList = list;
	list->mCount++;
}

// 0x80078A5C
void riderListClear(RiderList* list)
{
	riderListUnlinkAll(list);
	list->mHead  = NULL;
	list->mTail  = NULL;
	list->mCount = 0;
}

// 0x80078A98
void riderListRemove(RiderList* list, Rider* rider)
{
	if (rider->mPrev != NULL) {
		rider->mPrev->mNext = rider->mNext;
	} else {
		list->mHead = rider->mNext;
	}
	if (rider->mNext != NULL) {
		rider->mNext->mPrev = rider->mPrev;
	} else {
		list->mTail = rider->mPrev;
	}
	rider->mNext = NULL;
	rider->mPrev = NULL;
	rider->mList = NULL;
	list->mCount--;
}

// 0x80078AF8
void riderListUnlinkAll(RiderList* list)
{
	Rider* r = list->mHead;
	while (r != NULL) {
		Rider* next = r->mNext;
		r->mList    = NULL;
		r->mNext    = NULL;
		r->mPrev    = NULL;
		r           = next;
	}
}
