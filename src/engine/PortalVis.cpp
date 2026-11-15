#include "engine/PortalVis.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "core/Mem.h"
#include "core/SdkCalls.h"
#include "engine/WorldMesh.h"

extern "C" {
WorldVector<u8>* worldByteVecDtor(WorldVector<u8>*, s16);
}

WorldVector<u32>* portalVecFree(WorldVector<u32>* v, s16 flags);
WorldVector<Vec3f>* portalVecFree(WorldVector<Vec3f>* v, s16 flags);
static int portalOutside(Vec3f* verts, Frustum* view, Portal* portal);

// 0x8004FDDC
extern "C" void portalWorkInit()
{
	if (gPortalWork == NULL) {
		gPortalWork = new PortalWork;
	}
}

// 0x8004FE20
extern "C" void portalWorkShutdown()
{
	if (gPortalWork != NULL) {
		delete gPortalWork;
		gPortalWork = NULL;
	}
}

// 0x8004FE58
PortalWork::PortalWork()
{
	_00 = 0;
	_04 = 0;
	_08 = 0;
}

// 0x8004FE6C
PortalWork::~PortalWork()
{
}

// 0x8004FEA8
void Frustum::set(int fov, f32 aspect, f32 near, f32 far)
{
	int half = fov / 2;
	mTop     = njSin(half) / njCos(half) / near;
	mRight   = aspect * (njSin(half) / njCos(half) / near);
	mBottom  = -mTop;
	mLeft    = -mRight;
	mNear    = near;
	mFar     = far;
}

// 0x8004FF88
PortalVis::PortalVis()
{
	mData = NULL;
	portalVecInit(&mVisible);
	worldByteVecCtor(&mVisited);
	worldByteVecCtor(&mPortalFlags);
	portalVecInit(&mViewVerts);
}

// 0x8004FFDC
PortalVis::~PortalVis()
{
	portalVecFree(&mViewVerts, -1);
	worldByteVecDtor(&mPortalFlags, -1);
	worldByteVecDtor(&mVisited, -1);
	portalVecFree(&mVisible, -1);
}

// 0x80050050
void PortalVis::setVisible(u16 cell)
{
	mVisible.mData[cell >> 5] |= 1 << (cell & 31);
}

// 0x80050074
// Marks the cells seen from the camera
void PortalVis::update(Frustum* view, f32* matrix, const Vec3f* camPos)
{
	if (mData == NULL) {
		return;
	}

	portalFill(mVisible.begin(), mVisible.end(), 0);

	PortalVisData* data = mData;
	u16 cell            = findCell(*camPos);
	if (cell == 0xFFFF) {
		return;
	}

	f32 inverse[12];
	njPushMatrix(matrix);
	njInvertMatrix(NULL);
	PSMTXReorder(_gj_matrix_, inverse);
	PSMTXROMultVecArray(inverse, data->mVerts, mViewVerts.mData, data->mVertCount);
	njPopMatrixEx();

	PortalCell* here = &data->mCells[cell];
	portalFill(mPortalFlags.begin(), mPortalFlags.end(), 0);
	for (int i = 0; i < here->mOpenCount; i++) {
		mPortalFlags.mData[here->mOpen[i]] = PORTALFLAG_Open;
	}

	portalFill(mVisited.begin(), mVisited.end(), 0);

	flood(cell, data, view);
	setVisible(cell);
	for (int j = 0; j < mPortalFlags.mSize; j++) {
		if (mPortalFlags.mData[j] & PORTALFLAG_ToB) {
			setVisible(data->mPortals[j].mCellB);
		}
		if (mPortalFlags.mData[j] & PORTALFLAG_ToA) {
			setVisible(data->mPortals[j].mCellA);
		}
	}
}

// 0x80050280
// Goes through each open portal of the cell that is in the view to the cell on its other side
// The data's portal table
static inline Portal* visPortals(const PortalVisData* data)
{
	return data->mPortals;
}

void PortalVis::flood(int cell, const PortalVisData* data, Frustum* view)
{
	if (mVisited.mData[cell] != 0) {
		return;
	}
	mVisited.mData[cell] = 1;

	WorldVecIter<u8> first = mPortalFlags.begin();
	u8* flags              = first.mPtr;
	Portal* portals        = visPortals(data);
	PortalCell* here       = &data->mCells[cell];
	for (int i = 0; i < here->mPortalCount; i++) {
		u16 index      = here->mPortals[i];
		Portal* portal = &portals[index];
		if (flags[index] == 0 || (flags[index] & PORTALFLAG_Outside)) {
			continue;
		}
		if (portalOutside(mViewVerts.mData, view, portal)) {
			flags[index] |= PORTALFLAG_Outside;
		} else if (portal->mCellA == cell) {
			if (!(flags[index] & PORTALFLAG_ToB)) {
				flags[index] |= PORTALFLAG_ToB;
				flood(portal->mCellB, data, view);
			}
		} else {
			if (!(flags[index] & PORTALFLAG_ToA)) {
				flags[index] |= PORTALFLAG_ToA;
				flood(portal->mCellA, data, view);
			}
		}
	}
}

// 0x800503AC
u16 PortalVis::findCell(Vec3f pos) const
{
	return mData->findCell(&pos, 0);
}

// 0x800503D4
// 0xFFFF when the point is in no cell
u16 PortalVisData::findCell(const Vec3f* pos, u16 node) const
{
	PortalNode* at = &mNodes[node];
	u16 cell       = 0xFFFF;
	if (at->mIsLeaf & 1) {
		return at->mIndex;
	}
	if (!mPlanes[at->mIndex].isBehind(pos)) {
		if (at->mFront != 0xFFFF) {
			cell = findCell(pos, at->mFront);
		}
	} else {
		if (at->mBack != 0xFFFF) {
			cell = findCell(pos, at->mBack);
		}
	}
	return cell;
}

// 0x80050484
int PortalPlane::isBehind(const Vec3f* pos)
{
	if (njInnerProduct(&mNormal, pos) - mDist > -0.01f) {
		return 0;
	}
	return 1;
}

// 0x800504D0
// Whether the box around the portal in camera space misses the view
static int portalOutside(Vec3f* verts, Frustum* view, Portal* portal)
{
	f32 minX, minY;
	f32 maxX, maxY;
	f32 minZ, maxZ;
	minX = minY = minZ = 100000.0f;
	maxX = maxY = maxZ = -100000.0f;
	u16* index         = portal->mVerts;
	for (int i = 0; i < portal->mVertCount; i++) {
		Vec3f* v = &verts[index[i]];
		if (v->x < minX) {
			minX = v->x;
		}
		if (v->y < minY) {
			minY = v->y;
		}
		if (v->z < minZ) {
			minZ = v->z;
		}
		if (v->x > maxX) {
			maxX = v->x;
		}
		if (v->y > maxY) {
			maxY = v->y;
		}
		if (v->z > maxZ) {
			maxZ = v->z;
		}
	}

	if (minZ > view->mFar || maxZ < view->mNear) {
		return 1;
	}
	f32 edge = maxZ * (view->mRight / view->mNear);
	if (minX > edge || maxX < -edge) {
		return 1;
	}
	edge = maxZ * (view->mTop / view->mNear);
	if (minY > edge || maxY < -edge) {
		return 1;
	}
	return 0;
}

// 0x800505E0
void portalFill(WorldVecIter<u32> first, WorldVecIter<u32> last, const int& value)
{
	u32* to = first.mPtr;
	while (to != last.mPtr) {
		*to++ = value;
	}
}

// 0x80050604
void portalFill(WorldVecIter<u8> first, WorldVecIter<u8> last, const int& value)
{
	u8* to = first.mPtr;
	while (to != last.mPtr) {
		*to++ = value;
	}
}

// 0x80050630
void portalVecInit(WorldVector<Vec3f>* v)
{
	v->mData = NULL;
	v->mSize = 0;
	v->mCap  = 0;
}

// 0x80050644
WorldVector<Vec3f>* portalVecFree(WorldVector<Vec3f>* v, s16 flags)
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

// 0x80050698
void portalVecInit(WorldVector<u32>* v)
{
	v->mData = NULL;
	v->mSize = 0;
	v->mCap  = 0;
}

// 0x800506AC
WorldVector<u32>* portalVecFree(WorldVector<u32>* v, s16 flags)
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
