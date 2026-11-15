#include "engine/MeshColli.h"
#include "engine/WorldVector.h"
#include "engine/WorldMesh.h"
#include "Ninja/ninja.h"
#include "core/Mem.h"
#include "object/TObjEggRing.h"
#include "core/MathUtil.h"

struct WorldGather {
	WorldTreeNode** mData; // _00
	int mCount;            // _04
};

extern "C" {

WorldVector<WorldTreeNode*>* worldNodeVecDtor(WorldVector<WorldTreeNode*>*, s16);
WorldVector<u8>* worldByteVecDtor(WorldVector<u8>*, s16);
}

// 0x80076E4C
WorldMesh::WorldMesh()
{
	worldNodeVecCtor(&mHit);
	worldByteVecCtor(&mSeen);
	mFlags = 0;
	mData  = NULL;
	mVerts = NULL;
	mTris  = NULL;
	njUnitMatrix(mMatrix);
	njUnitMatrix(mInverse);
}

// 0x80076EBC
WorldMesh::~WorldMesh()
{
	worldByteVecDtor(&mSeen, -1);
	worldNodeVecDtor(&mHit, -1);
}

// 0x80076F30
void worldMeshSetData(WorldMesh* mesh, WorldData* data)
{
	worldNodeVecResize(&mesh->mHit, data->mLeaves);
	worldByteVecResize(&mesh->mSeen, data->mTriCount);
	syMemSet(mesh->mSeen.mData, 0, mesh->mSeen.mSize);
	mesh->mData  = data;
	mesh->mVerts = data->mVerts;
	mesh->mTris  = data->mTris;
}

// 0x80076F98
void* worldMeshFindGroup(WorldMesh* mesh, int id)
{
	WorldData* data = mesh->mData;
	if (data != NULL) {
		for (u32 i = 0; i < data->mGroupCount; i++) {
			if (id == data->mGroups[i].mId) {
				return &data->mGroups[i];
			}
		}
	}
	return NULL;
}

// 0x80076FDC
void worldMeshWalkGroup(WorldMesh* mesh, int id, void* fn, void* arg)
{
	typedef void (*Fn)(WorldMesh*, MeshTri*, void*, int*);
	WorldGroup* group = (WorldGroup*)worldMeshFindGroup(mesh, id);
	if (group != NULL) {
		int more  = 1;
		int index = group->mFirst;
		int count = group->mCount;
		while (count > 0 && more != 0) {
			((Fn)fn)(mesh, &mesh->mData->mTris[index], arg, &more);
			count--;
			index++;
		}
	}
}

// 0x80077070
int WorldMesh::gatherSphere(MeshEntry* entry, const void* pos)
{
	f32 box[6];
	boxAroundPoint(box, (const Vec3f*)pos);
	return worldMeshGather(this, entry, box);
}

// 0x800770B8
int WorldMesh::gatherLine(MeshEntry* entry, const void* from, const void* to)
{
	f32 box[6];
	boxAroundLine(box, (const Vec3f*)from, (const Vec3f*)to);
	return worldMeshGather(this, entry, box);
}

// 0x80077104
int WorldMesh::gatherBall(MeshEntry* entry, const void* sphere)
{
	f32 box[6];
	boxAroundSphere(box, (const MeshSphere*)sphere);
	return worldMeshGather(this, entry, box);
}

// 0x8007714C
int WorldMesh::gatherBox(MeshEntry* entry, const void* box)
{
	return worldMeshGather(this, entry, (const f32*)box);
}

// 0x8007716C
int WorldMesh::queryDown(const MeshEntry* entry, const Vec3f* pos, u32 mask, MeshHitFn hit, void* arg)
{
	int found;
	u16* id = (u16*)entry->_04;
	int count = entry->mCount;
	found = 0;
	Vec3f at;
	f32 dist;
	for (; count > 0; count--, id++) {
		MeshTri* tri = &mTris[*id];
		if ((tri->mMask & mask) && triGroundBelow(mVerts, tri, pos, &at, &dist) != 0) {
			found++;
			if (hit(this, *id, dist, &at, arg) == 0) {
				break;
			}
		}
	}
	return found;
}

// 0x8007722C
int WorldMesh::queryLine(const MeshEntry* entry, const Vec3f* from, const Vec3f* to, u32 mask, MeshHitFn hit, void* arg)
{
	int found;
	u16* id = (u16*)entry->_04;
	int count = entry->mCount;
	found = 0;
	Vec3f at;
	f32 dist;
	for (; count > 0; count--, id++) {
		MeshTri* tri = &mTris[*id];
		if ((tri->mMask & mask) && triLineCross(mVerts, tri, from, to, &at, &dist) != 0) {
			found++;
			if (hit(this, *id, dist, &at, arg) == 0) {
				break;
			}
		}
	}
	return found;
}

// 0x800772F4
int WorldMesh::querySphere(const MeshEntry* entry, const MeshSphere* sphere, u32 mask, void* hitFn, void* arg)
{
	typedef int (*Fn)(WorldMesh*, int, void*, f32);
	int found;
	u16* id = (u16*)entry->_04;
	Vec3f* verts = mVerts;
	int count = entry->mCount;
	found = 0;
	Vec3f at;
	u16 which;
	for (; count > 0; count--, id++) {
		MeshTri* tri = &mTris[*id];
		if (!(tri->mMask & mask)) {
			continue;
		}
		const Vec3f* a = &verts[tri->mVerts[0]];
		f32 height = njInnerProduct(&tri->mNormal, &sphere->mPos);
		height -= njInnerProduct(&tri->mNormal, a);
		if ((f32)__fabs(height) > sphere->mRadius) {
			continue;
		}
		gjMulAddVector(-height, &at, &sphere->mPos, &tri->mNormal);
		const Vec3f* b = &verts[tri->mVerts[1]];
		const Vec3f* c = &verts[tri->mVerts[2]];
		if (triContainsPoint(a, b, c, &at, &tri->mNormal) != 0) {
			found++;
			if (((Fn)hitFn)(this, *id, arg, height) == 0) {
				return found;
			}
		} else if (triSphereCorner(sphere->mRadius, a, b, c, &sphere->mPos, &which) != 0) {
			found++;
			if (((Fn)hitFn)(this, *id, arg, height) == 0) {
				return found;
			}
		} else if (triSphereEdge(sphere->mRadius, a, b, c, &sphere->mPos, &which, &at) != 0) {
			found++;
			if (((Fn)hitFn)(this, *id, arg, height) == 0) {
				return found;
			}
		}
	}
	return found;
}

// 0x800774E8
// A separate callback for the face the corners and the edges
int WorldMesh::querySphereEx(const MeshEntry* entry, const MeshSphere* sphere, u32 mask, void* faceFn, void* cornerFn, void* edgeFn, void* arg)
{
	typedef int (*Face)(WorldMesh*, int, void*, const Vec3f*, f32);
	typedef int (*Corner)(WorldMesh*, int, void*, int, f32);
	typedef int (*Edge)(WorldMesh*, int, void*, int, const Vec3f*, f32);
	int found;
	u16* id = (u16*)entry->_04;
	Vec3f* verts = mVerts;
	int count = entry->mCount;
	found = 0;
	Vec3f at;
	u16 which;
	for (; count > 0; count--, id++) {
		MeshTri* tri = &mTris[*id];
		if (!(tri->mMask & mask)) {
			continue;
		}
		const Vec3f* a = &verts[tri->mVerts[0]];
		f32 height = njInnerProduct(&tri->mNormal, &sphere->mPos);
		height -= njInnerProduct(&tri->mNormal, a);
		if ((f32)__fabs(height) > sphere->mRadius) {
			continue;
		}
		gjMulAddVector(-height, &at, &sphere->mPos, &tri->mNormal);
		const Vec3f* b = &verts[tri->mVerts[1]];
		const Vec3f* c = &verts[tri->mVerts[2]];
		if (faceFn != NULL && triContainsPoint(a, b, c, &at, &tri->mNormal) != 0) {
			found++;
			if (((Face)faceFn)(this, *id, arg, &at, height) == 0) {
				return found;
			}
		} else if (cornerFn != NULL && triSphereCorner(sphere->mRadius, a, b, c, &sphere->mPos, &which) != 0) {
			found++;
			if (((Corner)cornerFn)(this, *id, arg, which, height) == 0) {
				return found;
			}
		} else if (edgeFn != NULL && triSphereEdge(sphere->mRadius, a, b, c, &sphere->mPos, &which, &at) != 0) {
			found++;
			if (((Edge)edgeFn)(this, *id, arg, which, &at, height) == 0) {
				return found;
			}
		}
	}
	return found;
}

// 0x8007770C
int worldMeshGather(WorldMesh* mesh, MeshEntry* entry, const f32* box)
{
	WorldGather gather;
	gather.mData  = mesh->mHit.mData;
	gather.mCount = 0;
	if ((u32)worldTreeQuery(mesh, &gather, box) == 0) {
		return 0;
	}
	worldLeavesToTris(mesh, &gather, entry);
	return entry->mCount;
}

// 0x80077774
int worldTreeQuery(WorldMesh* mesh, WorldGather* gather, const f32* box)
{
	if (mesh->mData != NULL) {
		worldTreeGather(mesh, gather, box, 0);
	}
	return gather->mCount;
}

// 0x800777B4
// Only X and Z since the tree is a quadtree
void worldTreeGather(WorldMesh* mesh, WorldGather* gather, const f32* box, int index)
{
	WorldTreeNode* node = &mesh->mData->mTree[index];
	if (box[3] < node->mMinX || box[0] > node->mMaxX || box[5] < node->mMinZ || box[2] > node->mMaxZ) {
		return;
	}
	if (node->mIsLeaf == 0) {
		worldTreeGather(mesh, gather, box, node->mChildren[0]);
		worldTreeGather(mesh, gather, box, node->mChildren[1]);
		worldTreeGather(mesh, gather, box, node->mChildren[2]);
		worldTreeGather(mesh, gather, box, node->mChildren[3]);
	} else {
		gather->mData[gather->mCount++] = node;
	}
}

// 0x800778A4
// Keeps each triangle in the leaves once then clears the marks
void worldLeavesToTris(WorldMesh* mesh, WorldGather* gather, MeshEntry* entry)
{
	u8* seen;
	WorldTreeNode** leaf;
	u32 i;
	int k;
	int count;
	u16* out;
	count = 0;
	seen = mesh->mSeen.mData;
	leaf = gather->mData;
	out = (u16*)entry->_04;
	for (i = 0; i < gather->mCount; i++, leaf++) {
		for (k = 0; k < (*leaf)->mTriCount; k++) {
			u16 tri = (*leaf)->mTris[k];
			if (seen[tri] == 0) {
				out[count] = tri;
				count++;
			}
			seen[tri]++;
		}
	}
	entry->mMesh  = mesh;
	entry->mCount = count;
	for (int i = 0; i < count; i++) {
		mesh->mSeen.mData[out[i]] = 0;
	}
}

// 0x80077968
void WorldMesh::walk(const MeshEntry* entry, void* fn, void* arg, int* more)
{
	typedef void (*Fn)(WorldMesh*, int, void*, int*);
	u32 count;
	u16* id;

	id    = (u16*)entry->_04;
	count = entry->mCount;
	for (u32 i = 0; i < count && *more != 0; i++, id++) {
		((Fn)fn)(this, *id, arg, more);
	}
}

// 0x800779E4
void worldNodeVecCtor(WorldVector<WorldTreeNode*>* v)
{
	v->mData = NULL;
	v->mSize = 0;
	v->mCap  = 0;
}

// 0x800779F8
WorldVector<WorldTreeNode*>* worldNodeVecDtor(WorldVector<WorldTreeNode*>* v, s16 flags)
{
	if (v != NULL) {
		if (v->mData != NULL) {
			delete[] v->mData;
		}
		if (flags > 0) {
			delete v;
		}
	}
	return v;
}

// 0x80077A4C
void worldNodeVecAlloc(WorldVector<WorldTreeNode*>* v, int cap)
{
	v->mData = new WorldTreeNode*[cap];
	v->mCap  = cap;
}

// 0x80077A88
void worldNodeVecCopy(WorldVector<WorldTreeNode*>* v, WorldTreeNode** to, WorldTreeNode* const* from, int count)
{
	for (int i = 0; i < count; i++) {
		to[i] = from[i];
	}
}

// 0x80077AAC
void worldByteVecCtor(WorldVector<u8>* v)
{
	v->mData = NULL;
	v->mSize = 0;
	v->mCap  = 0;
}

// 0x80077AC0
WorldVector<u8>* worldByteVecDtor(WorldVector<u8>* v, s16 flags)
{
	if (v != NULL) {
		if (v->mData != NULL) {
			delete[] v->mData;
		}
		if (flags > 0) {
			delete v;
		}
	}
	return v;
}

// 0x80077B14
void worldByteVecAlloc(WorldVector<u8>* v, int cap)
{
	v->mData = new u8[cap];
	v->mCap  = cap;
}

// 0x80077B50
void worldByteVecCopy(WorldVector<u8>* v, u8* to, const u8* from, int count)
{
	for (int i = 0; i < count; i++) {
		*to++ = *from++;
	}
}

// 0x80077B74
void worldByteVecResize(WorldVector<u8>* v, int size)
{
	if (size != v->mSize) {
		worldByteVecReserve(v, size);
		v->mSize = size;
	}
}

// 0x80077BB4
void worldByteVecReserve(WorldVector<u8>* v, int cap)
{
	if (cap > v->mCap) {
		u8* old = v->mData;
		worldByteVecAlloc(v, cap);
		if (old != NULL) {
			worldByteVecCopy(v, v->mData, old, v->mSize);
			delete[] old;
		}
	}
}

// 0x80077C14
void worldNodeVecResize(WorldVector<WorldTreeNode*>* v, int size)
{
	if (size != v->mSize) {
		worldNodeVecReserve(v, size);
		v->mSize = size;
	}
}

// 0x80077C54
void worldNodeVecReserve(WorldVector<WorldTreeNode*>* v, int cap)
{
	if (cap > v->mCap) {
		WorldTreeNode** old = v->mData;
		worldNodeVecAlloc(v, cap);
		if (old != NULL) {
			worldNodeVecCopy(v, v->mData, old, v->mSize);
			delete[] old;
		}
	}
}
