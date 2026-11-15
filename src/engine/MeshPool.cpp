#include "engine/MeshColli.h"
#include "engine/WorldMesh.h"
#include "engine/MeshPool.h"
#include "core/Mem.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "core/MathUtil.h"
#include "core/VecMath.h"
#include "core/SdkCalls.h"
#include "Ninja/ninja.h"

extern "C" {

MeshVector<u16>* meshU16VecDtor(MeshVector<u16>*, s16);
MeshVector<MeshObject*>* meshPtrVecDtor(MeshVector<MeshObject*>*, s16);
MeshVector<MeshTreeNode>* meshNodeVecDtor(MeshVector<MeshTreeNode>*, s16);
}

// 0x8007A160
MeshObject::MeshObject()
{
	mLink.mNext = NULL;
	mLink.mPrev = NULL;
	mLink.mList = NULL;
	mData       = NULL;
	mPool       = NULL;
	mTreeSlot   = 0xFFFF;
	mLink.mOwner = this;
	mFlags = 1;
	mState = 0;
}

// 0x8007A1D0
MeshObject::~MeshObject()
{
}

// 0x8007A228
void meshColliSetData(MeshObject* mesh, MeshData* data)
{
	mesh->mVerts = data->mVerts;
	mesh->mTris  = data->mTris;
	mesh->mData  = data;
}

// 0x8007A240
void meshSetState100(MeshObject* mesh, int on)
{
	if (on != 0) {
		mesh->mState |= 0x100;
	} else {
		mesh->mState &= ~0x100;
	}
}

// 0x8007A268
void meshSetState200(MeshObject* mesh, int on)
{
	if (on != 0) {
		mesh->mState |= 0x200;
	} else {
		mesh->mState &= ~0x200;
	}
}

// 0x8007A290
void meshSetFlag2(MeshObject* mesh, int on)
{
	if (on != 0) {
		mesh->mFlags |= 2;
	} else {
		mesh->mFlags &= ~2;
	}
}

// 0x8007A2B8
void meshClearState400(MeshObject* mesh)
{
	mesh->mState &= ~0x400;
}

// 0x8007A2C8
void meshColliRefresh(MeshObject* mesh)
{
	mesh->mState |= 0x400;
	meshDropRiders(mesh);
}

// 0x8007A2F4
void meshColliSetBox(MeshObject* mesh, const f32* box)
{
	Vec3f v;
	vecAdd(&v, (Vec3f*)(box + 3), (Vec3f*)box);
	vecScale(0.5f, &mesh->mCenter, &v);
	vecSub(&v, (Vec3f*)(box + 3), (Vec3f*)box);
	vecScale(0.5f, &mesh->mHalf, &v);
}

// 0x8007A360
int MeshObject::gatherSphere(MeshEntry* entry, const void*)
{
	if (mData == NULL) {
		return 0;
	}
	entry->mMesh  = this;
	entry->mCount = mData->mTriCount;
	return mData->mTriCount;
}

// 0x8007A390
int MeshObject::gatherLine(MeshEntry* entry, const void*, const void*)
{
	if (mData == NULL) {
		return 0;
	}
	entry->mMesh  = this;
	entry->mCount = mData->mTriCount;
	return mData->mTriCount;
}

// 0x8007A3C0
int MeshObject::gatherBall(MeshEntry* entry, const void*)
{
	if (mData == NULL) {
		return 0;
	}
	entry->mMesh  = this;
	entry->mCount = mData->mTriCount;
	return mData->mTriCount;
}

// 0x8007A3F0
int MeshObject::gatherBox(MeshEntry* entry, const void*)
{
	if (mData == NULL) {
		return 0;
	}
	entry->mMesh  = this;
	entry->mCount = mData->mTriCount;
	return mData->mTriCount;
}

// 0x8007A420
int MeshObject::queryDown(const MeshEntry* entry, const Vec3f* pos, u32 mask, MeshHitFn hit, void* arg)
{
	u32 i;
	u32 count;
	int found = 0;
	MeshTri* tri = mTris;
	Vec3f local;
	mtxMultVec(mInverse, pos, &local);
	if ((mState & 0x200) != 0 || (mState & 0x100) == 0) {
		Vec3f at;
		f32 dist;
		for (i = 0, count = entry->mCount; i < count; i++, tri++) {
			if ((tri->mMask & mask) && triGroundBelow(mVerts, tri, &local, &at, &dist) != 0) {
				found++;
				mtxMultVec(mMatrix, &at, &at);
				if (hit(this, i & 0xFFFF, dist, &at, arg) == 0) {
					break;
				}
			}
		}
	} else {
		Vec3f low = *pos;
		low.y -= gFarRayDist;
		mtxMultVec(mInverse, &low, &low);
		Vec3f at;
		f32 dist;
		for (i = 0; i < entry->mCount; i++, tri++) {
			if ((tri->mMask & mask) && triLineCross(mVerts, tri, &local, &low, &at, &dist) != 0) {
				found++;
				mtxMultVec(mMatrix, &at, &at);
				if (hit(this, i & 0xFFFF, dist, &at, arg) == 0) {
					break;
				}
			}
		}
	}
	return found;
}

// 0x8007A5D4
int MeshObject::queryLine(const MeshEntry* entry, const Vec3f* from, const Vec3f* to, u32 mask, MeshHitFn hit, void* arg)
{
	u32 count;
	int found = 0;
	MeshTri* tri;
	u32 i;
	Vec3f a, b, at;
	f32 dist;

	mtxMultVec(mInverse, from, &a);
	mtxMultVec(mInverse, to, &b);
	tri   = mTris;
	i     = 0;
	count = entry->mCount;
	for (; i < count; i++, tri++) {
		if ((tri->mMask & mask) && triLineCross(mVerts, tri, &a, &b, &at, &dist) != 0) {
			found++;
			mtxMultVec(mMatrix, &at, &at);
			if (hit(this, i & 0xFFFF, dist, &at, arg) == 0) {
				break;
			}
		}
	}
	return found;
}

// 0x8007A6C4
int MeshObject::querySphere(const MeshEntry* entry, const MeshSphere* sphere, u32 mask, void* hitFn, void* arg)
{
	typedef int (*Fn)(MeshObject*, int, void*, f32);
	int found = 0;
	Vec3f local, at;
	u16 which;
	mtxMultVec(mInverse, &sphere->mPos, &local);
	Vec3f* verts = mData->mVerts;
	MeshTri* tri = mData->mTris;
	for (u32 i = 0; i < mData->mTriCount; i++, tri++) {
		if (!(tri->mMask & mask)) {
			continue;
		}
		// Both products take the sphere centre so the height is always zero the original's slip
		f32 height = njInnerProduct(&tri->mNormal, &local);
		height -= njInnerProduct(&tri->mNormal, &local);
		if ((f32)__fabs(height) > sphere->mRadius) {
			continue;
		}
		gjMulAddVector(-height, &at, &local, &tri->mNormal);
		const Vec3f* a = &verts[tri->mVerts[0]];
		const Vec3f* b = &verts[tri->mVerts[1]];
		const Vec3f* c = &verts[tri->mVerts[2]];
		if (triContainsPoint(a, b, c, &at, &tri->mNormal) != 0) {
			found++;
			if (((Fn)hitFn)(this, i & 0xFFFF, arg, height) == 0) {
				return found;
			}
		} else if (triSphereCorner(sphere->mRadius, a, b, c, &local, &which) != 0) {
			found++;
			if (((Fn)hitFn)(this, i & 0xFFFF, arg, height) == 0) {
				return found;
			}
		} else if (triSphereEdge(sphere->mRadius, a, b, c, &local, &which, &at) != 0) {
			found++;
			if (((Fn)hitFn)(this, i & 0xFFFF, arg, height) == 0) {
				return found;
			}
		}
	}
	return found;
}

// 0x8007A8C4
// A separate callback for the face the corners and the edges
int MeshObject::querySphereEx(const MeshEntry* entry, const MeshSphere* sphere, u32 mask, void* faceFn, void* cornerFn, void* edgeFn, void* arg)
{
	typedef int (*Face)(MeshObject*, int, void*, const Vec3f*, f32);
	typedef int (*Corner)(MeshObject*, int, void*, u16, f32);
	typedef int (*Edge)(MeshObject*, int, void*, u16, const Vec3f*, f32);
	int found = 0;
	Vec3f local, at;
	u16 which;
	mtxMultVec(mInverse, &sphere->mPos, &local);
	Vec3f* verts = mData->mVerts;
	MeshTri* tri = mData->mTris;
	for (u32 i = 0; i < mData->mTriCount; i++, tri++) {
		if (!(tri->mMask & mask)) {
			continue;
		}
		const Vec3f* a = &verts[tri->mVerts[0]];
		f32 height = njInnerProduct(&tri->mNormal, &local);
		height -= njInnerProduct(&tri->mNormal, a);
		if ((f32)__fabs(height) > sphere->mRadius) {
			continue;
		}
		gjMulAddVector(-height, &at, &local, &tri->mNormal);
		const Vec3f* b = &verts[tri->mVerts[1]];
		const Vec3f* c = &verts[tri->mVerts[2]];
		if (faceFn != NULL && triContainsPoint(a, b, c, &at, &tri->mNormal) != 0) {
			found++;
			mtxMultVec(mMatrix, &at, &at);
			if (((Face)faceFn)(this, i & 0xFFFF, arg, &at, height) == 0) {
				return found;
			}
		} else if (cornerFn != NULL && triSphereCorner(sphere->mRadius, a, b, c, &local, &which) != 0) {
			found++;
			if (((Corner)cornerFn)(this, i & 0xFFFF, arg, which, height) == 0) {
				return found;
			}
		} else if (edgeFn != NULL && triSphereEdge(sphere->mRadius, a, b, c, &local, &which, &at) != 0) {
			found++;
			mtxMultVec(mMatrix, &at, &at);
			if (((Edge)edgeFn)(this, i & 0xFFFF, arg, which, &at, height) == 0) {
				return found;
			}
		}
	}
	return found;
}

// 0x8007AB14
void MeshObject::walk(const MeshEntry* entry, void* fn, void* arg, int* more)
{
	typedef void (*Fn)(MeshObject*, int, void*, int*);
	u32 count = entry->mCount;
	for (u32 i = 0; i < count && *more != 0; i++) {
		((Fn)fn)(this, i & 0xFFFF, arg, more);
	}
}

// 0x8007AB88
MeshPool* meshPoolInit(MeshPool* pool, int count)
{
	meshLinkListCtor(&pool->mList);
	meshNodeVecCtor(&pool->mNodes);
	meshPtrVecCtor(&pool->mSlots);
	meshU16VecCtor(&pool->mOrder);
	meshU16VecCtor(&pool->mScratch);
	meshNodeVecResize(&pool->mNodes, count << 1);
	meshPtrVecResize(&pool->mSlots, count);
	meshU16VecResize(&pool->mOrder, count);
	meshU16VecResize(&pool->mScratch, count);
	pool->mNodeCount = 0;
	return pool;
}

// 0x8007AC14
MeshPool* meshPoolDestroy(MeshPool* pool, s16 flags)
{
	if (pool != NULL) {
		meshU16VecDtor(&pool->mScratch, -1);
		meshU16VecDtor(&pool->mOrder, -1);
		meshPtrVecDtor(&pool->mSlots, -1);
		meshNodeVecDtor(&pool->mNodes, -1);
		meshLinkListDtor(&pool->mList, -1);
		if (flags > 0) {
			delete pool;
		}
	}
	return pool;
}

// 0x8007AC94
MeshObject* meshPoolCreate(MeshPool* pool)
{
	MeshObject* mesh = new MeshObject;
	if (mesh == NULL) {
		return NULL;
	}
	mesh->mPool = pool;
	meshLinkListAppend(&pool->mList, &mesh->mLink);
	return mesh;
}

// 0x8007ACF8
void meshPoolFree(MeshPool* pool, MeshObject* mesh)
{
	if (mesh != NULL) {
		if (mesh->mTreeSlot != 0xFFFF) {
			pool->mSlots.mData[mesh->mTreeSlot] = NULL;
		}
		meshLinkListRemove(&pool->mList, &mesh->mLink);
		delete mesh;
	}
}

// 0x8007AD68
void meshPoolUpdate(MeshPool* pool)
{
	meshTreeRebuild(pool);
}

// 0x8007AD88
// Calls fn for each mesh whose box touches this one
void meshPoolQuery(MeshPool* pool, const f32* box, MeshPoolFn fn, void* arg)
{
	if (fn != NULL && pool->mNodeCount != 0) {
		meshTreeQuery(pool, box, fn, arg, 0);
	}
}

// 0x8007ADC0
void meshTreeQuery(MeshPool* pool, const f32* box, MeshPoolFn fn, void* arg, u32 index)
{
	MeshTreeNode* node = &pool->mNodes.mData[index & 0xFFFF];
	if (box[3] < node->mMin[0] || box[0] > node->mMax[0] || box[4] < node->mMin[1] || box[1] > node->mMax[1]
	    || box[5] < node->mMin[2] || box[2] > node->mMax[2]) {
		return;
	}
	if (node->mIsLeaf == 1) {
		MeshObject* mesh = pool->mSlots.mData[node->mSlot];
		if (mesh != NULL) {
			fn(mesh, arg);
		}
	} else {
		meshTreeQuery(pool, box, fn, arg, node->mLeft);
		meshTreeQuery(pool, box, fn, arg, node->mRight);
	}
}

// 0x8007AEBC
// The box around some meshes
void meshPoolBox(MeshPool* pool, f32* box, const u16* order, u32 count)
{
	f32 minX, maxX;
	f32 minY, maxY;
	f32 minZ, maxZ;
	minX = gFarRayDist;
	minY = gFarRayDist;
	minZ = gFarRayDist;
	maxX = -gFarRayDist;
	maxY = -gFarRayDist;
	maxZ = -gFarRayDist;
	while (count & 0xFFFF) {
		MeshObject* mesh = pool->mSlots.mData[*order];
		Vec3f lo, hi;
		vecSub(&lo, &mesh->mCenter, &mesh->mHalf);
		vecAdd(&hi, &mesh->mCenter, &mesh->mHalf);
		if (minX > lo.x) {
			minX = lo.x;
		}
		if (minY > lo.y) {
			minY = lo.y;
		}
		if (minZ > lo.z) {
			minZ = lo.z;
		}
		if (maxX < hi.x) {
			maxX = hi.x;
		}
		if (maxY < hi.y) {
			maxY = hi.y;
		}
		if (maxZ < hi.z) {
			maxZ = hi.z;
		}
		count--;
		order++;
	}
	box[0] = minX;
	box[1] = minY;
	box[2] = minZ;
	box[3] = maxX;
	box[4] = maxY;
	box[5] = maxZ;
}

// 0x8007B02C
// The longest axis of a box
int boxLongestAxis(const f32* box)
{
	int axis = 0;
	Vec3f size;
	vecSub(&size, (Vec3f*)(box + 3), (Vec3f*)box);
	f32 best = size.x;
	if (best < size.y) {
		best = size.y;
		axis = 1;
	}
	if (best < size.z) {
		axis = 2;
	}
	return axis;
}

// 0x8007B094
// Rebuilds the tree
void meshTreeRebuild(MeshPool* pool)
{
	int count = 0;
	for (MeshLink* link = pool->mList.mHead; link != NULL; link = link->mNext) {
		MeshObject* mesh = (MeshObject*)link->mOwner;
		if (mesh->mState & 0x400) {
			mesh->mTreeSlot = 0xFFFF;
		} else {
			mesh->mTreeSlot = count;
			pool->mOrder.mData[count] = count;
			pool->mSlots.mData[count] = mesh;
			count++;
		}
	}
	pool->mNodeCount = 0;
	if (count != 0) {
		f32 box[6];
		meshPoolBox(pool, box, pool->mOrder.mData, count & 0xFFFF);
		sMeshSortSlots = pool->mSlots.mData;
		pool->mNodeCount++;
		meshTreeBuild(pool, box, pool->mOrder.mData, count, 0);
	}
}

// 0x8007B16C
u32 meshTreeBuild(MeshPool* pool, const f32* box, u16* order, u32 count, u32 index)
{
	MeshTreeNode* node = &pool->mNodes.mData[index & 0xFFFF];
	node->mMin[0] = box[0];
	node->mMin[1] = box[1];
	node->mMin[2] = box[2];
	node->mMax[0] = box[3];
	node->mMax[1] = box[4];
	node->mMax[2] = box[5];
	if (count == 1) {
		node->mIsLeaf = 1;
		node->mSlot   = *order;
		return index;
	}
	node->mIsLeaf = 0;
	u32 right;
	u16* rightOrder;
	u32 left;
	int axis = boxLongestAxis(box);
	if (axis == 0) {
		left = meshTreeSplitX(pool, box, order, count);
	} else if (axis == 1) {
		left = meshTreeSplitY(pool, box, order, count);
	} else if (axis == 2) {
		left = meshTreeSplitZ(pool, box, order, count);
	}
	f32 leftBox[6];
	f32 rightBox[6];
	meshPoolBox(pool, leftBox, order, left & 0xFFFF);
	right = count - left;
	rightOrder = order + left;
	meshPoolBox(pool, rightBox, rightOrder, right & 0xFFFF);
	node->mLeft  = pool->mNodeCount++;
	node->mRight = pool->mNodeCount++;
	meshTreeBuild(pool, leftBox, order, left, node->mLeft);
	meshTreeBuild(pool, rightBox, rightOrder, right, node->mRight);
	return index;
}

// 0x8007B300
int meshSortCmpX(const u16* a, const u16* b)
{
	int x = (int)sMeshSortSlots[*a]->mCenter.x;
	int y = (int)sMeshSortSlots[*b]->mCenter.x;
	if ((u16)x < (u16)y) {
		return -1;
	}
	return (u16)x > (u16)y;
}

// 0x8007B368
int meshSortCmpY(const u16* a, const u16* b)
{
	int x = (int)sMeshSortSlots[*a]->mCenter.y;
	int y = (int)sMeshSortSlots[*b]->mCenter.y;
	if ((u16)x < (u16)y) {
		return -1;
	}
	return (u16)x > (u16)y;
}

// 0x8007B3D0
int meshSortCmpZ(const u16* a, const u16* b)
{
	int x = (int)sMeshSortSlots[*a]->mCenter.z;
	int y = (int)sMeshSortSlots[*b]->mCenter.z;
	if ((u16)x < (u16)y) {
		return -1;
	}
	return (u16)x > (u16)y;
}

// 0x8007B438
// Splits along X Returns how many went left
int meshTreeSplitX(MeshPool* pool, const f32* box, u16* order, u32 count)
{
	if (count == 2) {
		return 1;
	}
	f32 mid = 0.5f * (box[0] + box[3]);
	u32 left = 0;
	u16 id;
	u32 bytes = count << 1;
	int n = count;
	u16* front = pool->mScratch.mData;
	u16* tail = (u16*)((u8*)front + bytes);
	u16* src = order;
	for (int i = 0; i < n; i++) {
		id = *src;
		if (pool->mSlots.mData[id]->mCenter.x < mid) {
			*front++ = id;
			left++;
		} else {
			*--tail = id;
		}
		src++;
	}
	if (left > ((count + 1) & ~1) * 3 / 5 || left < bytes / 5) {
		qsort(order, count, 2, (void*)meshSortCmpX);
		return count >> 1;
	}
	syMemCopy2(order, pool->mScratch.mData, n);
	return left;
}

// 0x8007B53C
// Splits along Y
int meshTreeSplitY(MeshPool* pool, const f32* box, u16* order, u32 count)
{
	if (count == 2) {
		return 1;
	}
	f32 mid = 0.5f * (box[1] + box[4]);
	u32 left = 0;
	u16 id;
	u32 bytes = count << 1;
	int n = count;
	u16* front = pool->mScratch.mData;
	u16* tail = (u16*)((u8*)front + bytes);
	u16* src = order;
	for (int i = 0; i < n; i++) {
		id = *src;
		if (pool->mSlots.mData[id]->mCenter.y < mid) {
			*front++ = id;
			left++;
		} else {
			*--tail = id;
		}
		src++;
	}
	if (left > ((count + 1) & ~1) * 3 / 5 || left < bytes / 5) {
		qsort(order, count, 2, (void*)meshSortCmpY);
		return count >> 1;
	}
	syMemCopy2(order, pool->mScratch.mData, n);
	return left;
}

// 0x8007B640
// Splits along Z
int meshTreeSplitZ(MeshPool* pool, const f32* box, u16* order, u32 count)
{
	if (count == 2) {
		return 1;
	}
	f32 mid = 0.5f * (box[2] + box[5]);
	u32 left = 0;
	u16 id;
	u32 bytes = count << 1;
	int n = count;
	u16* front = pool->mScratch.mData;
	u16* tail = (u16*)((u8*)front + bytes);
	u16* src = order;
	for (int i = 0; i < n; i++) {
		id = *src;
		if (pool->mSlots.mData[id]->mCenter.z < mid) {
			*front++ = id;
			left++;
		} else {
			*--tail = id;
		}
		src++;
	}
	if (left > ((count + 1) & ~1) * 3 / 5 || left < bytes / 5) {
		qsort(order, count, 2, (void*)meshSortCmpZ);
		return count >> 1;
	}
	syMemCopy2(order, pool->mScratch.mData, n);
	return left;
}

// 0x8007B744
void meshU16VecCtor(MeshVector<u16>* v)
{
	v->mData = NULL;
	v->mSize = 0;
	v->mCap  = 0;
}

// 0x8007B758
MeshVector<u16>* meshU16VecDtor(MeshVector<u16>* v, s16 flags)
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

// 0x8007B7AC
void meshU16VecClear(MeshVector<u16>* v)
{
	v->mSize = 0;
}

// 0x8007B7B8
void meshU16VecAlloc(MeshVector<u16>* v, int cap)
{
	v->mData = new u16[cap];
	v->mCap  = cap;
}

// 0x8007B7F4
void meshU16VecCopy(MeshVector<u16>* v, u16* to, const u16* from, int count)
{
	for (int i = 0; i < count; i++) {
		to[i] = from[i];
	}
}

// 0x8007B818
void meshU16VecResize(MeshVector<u16>* v, int size)
{
	if (size != v->mSize) {
		meshU16VecReserve(v, size);
		v->mSize = size;
	}
}

// 0x8007B858
void meshU16VecReserve(MeshVector<u16>* v, int cap)
{
	if (cap > v->mCap) {
		u16* old = v->mData;
		meshU16VecAlloc(v, cap);
		if (old != NULL) {
			meshU16VecCopy(v, v->mData, old, v->mSize);
			delete[] old;
		}
	}
}

// 0x8007B8B8
void meshU16VecPush(MeshVector<u16>* v, const u16* value)
{
	if (v->mSize + 1 > v->mCap) {
		meshU16VecReserve(v, v->mSize + v->mSize + 2);
	}
	v->mData[v->mSize] = *value;
	v->mSize++;
}

// 0x8007B924
void meshPtrVecCtor(MeshVector<MeshObject*>* v)
{
	v->mData = NULL;
	v->mSize = 0;
	v->mCap  = 0;
}

// 0x8007B938
MeshVector<MeshObject*>* meshPtrVecDtor(MeshVector<MeshObject*>* v, s16 flags)
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

// 0x8007B98C
void meshNodeVecCtor(MeshVector<MeshTreeNode>* v)
{
	v->mData = NULL;
	v->mSize = 0;
	v->mCap  = 0;
}

// 0x8007B9A0
MeshVector<MeshTreeNode>* meshNodeVecDtor(MeshVector<MeshTreeNode>* v, s16 flags)
{
	if (v != NULL) {
		if (v->mData != NULL) {
			if (v->mData != NULL) {
				::operator delete[]((u8*)v->mData - 0x10);
			}
		}
		if (flags > 0) {
			delete v;
		}
	}
	return v;
}

// 0x8007B9FC
void meshNodeVecResize(MeshVector<MeshTreeNode>* v, int size)
{
	if (size != v->mSize) {
		meshNodeVecReserve(v, size);
		v->mSize = size;
	}
}

// 0x8007BA3C
void meshPtrVecResize(MeshVector<MeshObject*>* v, int size)
{
	if (size != v->mSize) {
		meshPtrVecReserve(v, size);
		v->mSize = size;
	}
}

// 0x8007BA7C
void meshNodeVecReserve(MeshVector<MeshTreeNode>* v, int cap)
{
	if (cap > v->mCap) {
		MeshTreeNode* old = v->mData;
		meshNodeVecAlloc(v, cap);
		if (old != NULL) {
			meshNodeVecCopy(v, v->mData, old, v->mSize);
			if (old != NULL) {
				::operator delete[]((u8*)old - 0x10);
			}
		}
	}
}

// 0x8007BAE4
void meshPtrVecReserve(MeshVector<MeshObject*>* v, int cap)
{
	if (cap > v->mCap) {
		MeshObject** old = v->mData;
		meshPtrVecAlloc(v, cap);
		if (old != NULL) {
			meshPtrVecCopy(v, v->mData, old, v->mSize);
			delete[] old;
		}
	}
}

// 0x8007BB44
void meshPtrVecAlloc(MeshVector<MeshObject*>* v, int cap)
{
	v->mData = new MeshObject*[cap];
	v->mCap  = cap;
}

// 0x8007BB80
void meshPtrVecCopy(MeshVector<MeshObject*>* v, MeshObject** to, MeshObject* const* from, int count)
{
	for (int i = 0; i < count; i++) {
		to[i] = from[i];
	}
}

// 0x8007BBA4
void meshNodeVecCopy(MeshVector<MeshTreeNode>* v, MeshTreeNode* to, const MeshTreeNode* from, int count)
{
	for (int i = 0; i < count; i++) {
		to[i].mIsLeaf = from[i].mIsLeaf;
		to[i].mSlot   = from[i].mSlot;
		to[i].mLeft   = from[i].mLeft;
		to[i].mRight  = from[i].mRight;
		to[i].mMin[0] = from[i].mMin[0];
		to[i].mMin[1] = from[i].mMin[1];
		to[i].mMin[2] = from[i].mMin[2];
		to[i].mMax[0] = from[i].mMax[0];
		to[i].mMax[1] = from[i].mMax[1];
		to[i].mMax[2] = from[i].mMax[2];
	}
}

// 0x8007BC18
void meshNodeVecAlloc(MeshVector<MeshTreeNode>* v, int cap)
{
	v->mData = (MeshTreeNode*)__construct_new_array(::operator new[]((cap << 5) + 0x10), (void*)meshTreeNodeCtor, NULL, 0x20, cap);
	v->mCap  = cap;
}

// 0x8007BC70
void meshTreeNodeCtor()
{
}

// 0x8007BC74
void meshLinkListCtor(MeshList2* list)
{
	list->mHead  = NULL;
	list->mTail  = NULL;
	list->mCount = 0;
}

// 0x8007BC88
MeshList2* meshLinkListDtor(MeshList2* list, s16 flags)
{
	if (list != NULL) {
		meshLinkListUnlinkAll(list);
		if (flags > 0) {
			delete list;
		}
	}
	return list;
}

// 0x8007BCD0
void meshLinkListAppend(MeshList2* list, MeshLink* link)
{
	if (list->mHead == NULL) {
		list->mTail = link;
		list->mHead = link;
	} else {
		link->mPrev = list->mTail;
		list->mTail->mNext = link;
		list->mTail = link;
	}
	link->mList = list;
	list->mCount++;
}

// 0x8007BD10
void meshLinkListRemove(MeshList2* list, MeshLink* link)
{
	if (link->mPrev != NULL) {
		link->mPrev->mNext = link->mNext;
	} else {
		list->mHead = link->mNext;
	}
	if (link->mNext != NULL) {
		link->mNext->mPrev = link->mPrev;
	} else {
		list->mTail = link->mPrev;
	}
	link->mNext = NULL;
	link->mPrev = NULL;
	link->mList = NULL;
	list->mCount--;
}

// 0x8007BD70
void meshLinkListUnlinkAll(MeshList2* list)
{
	MeshLink* link = list->mHead;
	while (link != NULL) {
		MeshLink* next = link->mNext;
		link->mList = NULL;
		link->mNext = NULL;
		link->mPrev = NULL;
		link = next;
	}
}
