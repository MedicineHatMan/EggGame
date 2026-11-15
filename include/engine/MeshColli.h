#ifndef _MESHCOLLI_H
#define _MESHCOLLI_H

#include "core/types.h"
#include "stage/Stage.h"

class MeshColli;

/** @size 0x28 */
struct MeshTri {
	u16 mVerts[3]; // _00
	u16 _06;
	u32 mMask;     // _08 the surface bits a query tests
	Vec3f mNormal; // _0C
	Vec3f mCenter; // _18 of the bounding sphere
	f32 mRadius;   // _24
};

/** What meshColliSetData points at as stored in the file */
struct MeshData {
	Vec3f* mVerts; // _00
	u32 _04;
	MeshTri* mTris; // _08
	int mTriCount;  // _0C
};

/** One mesh in a query */
struct MeshEntry {
	MeshColli* mMesh; // _00
	u32 _04;
	u32 mCount; // _08
};

/** Point and radius for the sphere queries */
struct MeshSphere {
	Vec3f mPos;  // _00
	f32 mRadius; // _0C
};

typedef int (*MeshHitFn)(MeshColli* mesh, int tri, f32 dist, const Vec3f* pos, void* arg);

/** Base of the meshes with the transforms */
class MeshColli {
public:
	u8 _00[0xC];
	u32 mFlags;       // _0C Bit 0 = has a matrix bit 1 = checks facing bit 2 = the matrix has scale
	Vec3f* mVerts;    // _10
	MeshTri* mTris;   // _14
	f32 mMatrix[12];  // _18
	f32 mInverse[12]; // _48

	// The data comes first so the compiler puts the vtable after it at _78
	MeshColli();
	virtual ~MeshColli();                                                                            // _08
	virtual int gatherSphere(MeshEntry*, const void*);                                               // _0C
	virtual int gatherLine(MeshEntry*, const void*, const void*);                                    // _10
	virtual int gatherBall(MeshEntry*, const void*);                                                 // _14
	virtual int gatherBox(MeshEntry*, const void*);                                                  // _18
	virtual int queryDown(const MeshEntry*, const Vec3f*, u32, MeshHitFn, void*);                    // _1C
	virtual int queryLine(const MeshEntry*, const Vec3f*, const Vec3f*, u32, MeshHitFn, void*);      // _20
	virtual int querySphere(const MeshEntry*, const MeshSphere*, u32, void*, void*);                 // _24
	virtual int querySphereEx(const MeshEntry*, const MeshSphere*, u32, void*, void*, void*, void*); // _28
	virtual void walk(const MeshEntry*, void*, void*, int*);                                         // _2C
};

struct MeshLink {
	MeshLink* mNext; // _00
	MeshLink* mPrev; // _04
	void* mOwner;    // _08
	void* mList;     // _0C
};

/** A mesh in the pool. @size 0xB0 */
class MeshObject : public MeshColli {
public:
	MeshObject();
	virtual ~MeshObject();
	virtual int gatherSphere(MeshEntry*, const void*);
	virtual int gatherLine(MeshEntry*, const void*, const void*);
	virtual int gatherBall(MeshEntry*, const void*);
	virtual int gatherBox(MeshEntry*, const void*);
	virtual int queryDown(const MeshEntry*, const Vec3f*, u32, MeshHitFn, void*);
	virtual int queryLine(const MeshEntry*, const Vec3f*, const Vec3f*, u32, MeshHitFn, void*);
	virtual int querySphere(const MeshEntry*, const MeshSphere*, u32, void*, void*);
	virtual int querySphereEx(const MeshEntry*, const MeshSphere*, u32, void*, void*, void*, void*);
	virtual void walk(const MeshEntry*, void*, void*, int*);

	u16 mState;      // _7C 0x100 = long drop test 0x200 = short test 0x400 = out of the tree
	u16 mTreeSlot;   // _7E 0xFFFF if not in the tree
	MeshLink mLink;  // _80
	MeshData* mData; // _90
	void* mPool;     // _94
	Vec3f mCenter;   // _98
	Vec3f mHalf;     // _A4
};

#endif
