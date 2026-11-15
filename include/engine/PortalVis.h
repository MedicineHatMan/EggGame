#ifndef _PORTALVIS_H
#define _PORTALVIS_H
#include "Ninja/ninja.h"
#include "core/types.h"
struct PortalWork;

#include "stage/Stage.h"
#include "engine/WorldVector.h"

/** What the camera sees, at one unit in front of it. Made by Frustum::set. @size 0x18 */
struct Frustum {
	void set(int fov, f32 aspect, f32 near, f32 far);

	f32 mTop;   // _00
	f32 mBottom; // _04
	f32 mLeft;   // _08
	f32 mRight;  // _0C
	f32 mNear;   // _10
	f32 mFar;    // _14
};

struct PortalPlane { // size 0x10
	int isBehind(const Vec3f* pos);

	Vec3f mNormal; // _00
	f32 mDist;     // _0C
};

struct PortalNode { // size 8
	u16 mIsLeaf; // _00 bit 0
	u16 mIndex;  // _02 the cell for a leaf else the plane
	u16 mFront;  // _04 0xFFFF = none
	u16 mBack;   // _06
};

struct Portal { // size 0xC
	u8 _00;
	u8 mVertCount; // _01
	u8 _02[2];
	u16 mCellA;    // _04
	u16 mCellB;    // _06
	u16* mVerts;   // _08
};

struct PortalCell { // size 0x14
	u8 _00[2];
	u16 mPortalCount; // _02
	u16 mOpenCount;   // _04
	u8 _06[2];
	u16* mPortals;    // _08
	u16* mOpen;       // _0C the portals that can be looked through from here
	u8 _10[4];
};

struct PortalVisData {
	u16 findCell(const Vec3f* pos, u16 node) const;

	Vec3f* mVerts;       // _00
	int mVertCount;      // _04
	u8 _08[8];
	PortalNode* mNodes;  // _10
	u8 _14[4];
	PortalPlane* mPlanes; // _18
	u8 _1C[4];
	Portal* mPortals;    // _20
	u8 _24[4];
	PortalCell* mCells;  // _28
};

enum PortalFlag {
	PORTALFLAG_Open    = 1,
	PORTALFLAG_ToB     = 2, // looked through from cell A
	PORTALFLAG_ToA     = 4, // looked through from cell B
	PORTALFLAG_Outside = 8, // out of the view
};

/**
 * The stage's portal culling. TStage makes it and the scene record holds it.
 * @size 0x34
 */
struct PortalVis {
	PortalVis();
	~PortalVis();

	void setVisible(u16 cell);
	void update(Frustum* view, f32* matrix, const Vec3f* camPos);
	void flood(int cell, const PortalVisData* data, Frustum* view);
	u16 findCell(Vec3f pos) const;

	PortalVisData* mData;           // _00
	WorldVector<u32> mVisible;      // _04 a bit per cell
	WorldVector<u8> mVisited;       // _10 a byte per cell
	WorldVector<u8> mPortalFlags;   // _1C PortalFlag per portal
	WorldVector<Vec3f> mViewVerts;  // _28 mData's vertices in camera space
};

extern "C" {
void portalWorkInit();
void portalWorkShutdown();
}

/** A work area groundInit makes and groundShutdown frees. Nothing else reads it. @size 0x14C */
struct PortalWork {
	PortalWork();
	~PortalWork();

	int _00;
	int _04;
	int _08;
	u8 _0C[0x140];
};

extern "C" {
extern PortalWork* gPortalWork;
}

#include "engine/WorldVector.h"

void portalFill(WorldVecIter<u32> first, WorldVecIter<u32> last, const int& value);
void portalFill(WorldVecIter<u8> first, WorldVecIter<u8> last, const int& value);
void portalVecInit(WorldVector<u32>* v);
void portalVecInit(WorldVector<Vec3f>* v);

#endif
