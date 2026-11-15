#ifndef _RAIL_H
#define _RAIL_H

#include "core/types.h"
#include "stage/Stage.h"
#include "engine/WorldVector.h"

struct EggCollide;
struct RailPath;
struct RailData;

// A place on a rail the distance past the start of a segment and the segment
struct RailPoint {
	f32 mDist; // _00
	int mSeg;  // _04
};

// Called for each rail segment a search touches
typedef int (*RailHitFn)(RailData* rail, RailPoint* point, Vec3f* pos, void* arg, int* more);
// The same as the manager hands it to the caller with the rail's RailPath
typedef void (*RailPathFn)(RailPath* path, const RailPoint* point, Vec3f* pos, void* arg, int* more);

// The points of a rail
struct RailVerts {
	u32 _00;        // _00
	Vec3f* mPos;    // _04
	Vec3f* mNormal; // _08 one per point only free rails have them
};

enum RailKind {
	RAIL_FREE = 0, // has its own normals
	RAIL_FLAT = 1, // up is always +Y
};

/** @size 0x18 */
struct RailData {
	f32 getFrac(int seg, f32 dist);
	void getPosFree(int seg, f32 dist, Vec3f* out);
	void getPosFlat(int seg, f32 dist, Vec3f* out);
	void getDirFree(int seg, Vec3f* out);
	void getDirFlat(int seg, Vec3f* out);
	int hitFree(int first, int last, const Vec3f* center, RailHitFn cb, void* arg, int* more, f32 radius);
	int hitFlat(int first, int last, const Vec3f* center, RailHitFn cb, void* arg, int* more, f32 radius);
	int hit(int first, int last, const Vec3f* center, RailHitFn cb, void* arg, int* more, f32 radius);

	u8 mKind;          // _00 RailKind
	u8 mFlags;         // _01 bit 0 = loops
	u16 mCount;        // _02 points
	u32 mId;           // _04
	u32 mType;         // _08 the plain search only finds type 0
	f32 mLength;       // _0C
	RailVerts* mVerts; // _10
	f32* mDist;        // _14 the distance from the start to each point
};

// A cell of the quad tree over the rails on X and Z
struct RailNode {
	s16 mLeaf;     // _00
	u16 mCount;    // _02 entries in a leaf
	u32 mEntries;  // _04 byte offset into RailSet::mEntries
	f32 mMinX;     // _08
	f32 mMinZ;     // _0C
	f32 mMaxX;     // _10
	f32 mMaxZ;     // _14
	u16 mChild[4]; // _18
};

// A leaf entry a run of segments of one rail
struct RailEntry {
	s16 mRail;  // _00
	s16 mFirst; // _02
	s16 mLast;  // _04
};

// The rail file of a stage
struct RailSet {
	int mCount;        // _00
	RailData* mRails;  // _04
	u32 _08;           // _08
	RailNode* mNodes;  // _0C
	u32 _10;           // _10
	u8* mEntries;      // _14
};

struct RailBox {
	Vec3f mMin; // _00
	Vec3f mMax; // _0C
};

struct RailSphere {
	Vec3f mCenter;  // _00
	f32 mRadius;    // _0C
	RailHitFn mCb;  // _10
	void* mArg;     // _14
};

struct RailQuery {
	RailBox mBox;       // _00
	RailSphere mSphere; // _18
};

struct RailList;

// A rider's place in a rail's list of riders
struct RailLink {
	RailLink()
	{
		mNext = 0;
		mPrev = 0;
		mList = 0;
	}

	RailLink* mNext;    // _00
	RailLink* mPrev;    // _04
	EggCollide* mOwner; // _08
	RailList* mList;    // _0C
};

struct RailList {
	RailList();
	~RailList();
	void append(RailLink* link);
	void remove(RailLink* link);
	void clear();

	RailLink* mTail; // _00
	RailLink* mHead; // _04
	int mCount;      // _08
};

/** @size 0x14 */
struct RailPath {
	RailPath();
	~RailPath();

	f32 getLength();
	int findSeg(f32 dist, int seg, f32* rest);
	void getNormal(int seg, f32 dist, Vec3f* out);
	f32 getDistance(int seg, f32 dist);
	void getDir(const RailPoint* point, Vec3f* out);
	void getDir(int seg, f32 dist, Vec3f* out);
	void getPos(int seg, f32 dist, Vec3f* out);

	RailData* mData;    // _00
	u32 _04;            // _04
	RailList mRiders;   // _08
};

struct RailArray {
	RailArray();
	~RailArray();
	void resize(int count);
	void reserve(int count);
	void alloc(int count);
	void copy(RailPath** to, RailPath* const* from, int count);

	RailPath** mData; // _00
	int mCount;       // _04
	int mCap;         // _08
};

/** @size 0x10 */
struct RailFile {
	RailFile();
	~RailFile();
	void setSet(RailSet* set);
	int walk(const RailBox* box, int node, int* more, int hits, const RailSphere* sphere);
	int search(const Vec3f* pos, RailHitFn cb, void* arg, int* more, f32 radius);

	RailSet* mSet;          // _00
	WorldVector<u8> mSeen;  // _04
};

/** @size 0x10 */
struct RailMgr {
	RailMgr();
	~RailMgr();
	void clear();
	RailPath* find(u32 id);
	void load(RailSet* set);
	int searchOpen(f32 radius, const Vec3f* pos, RailPathFn cb, void* arg);
	int searchAll(f32 radius, const Vec3f* pos, RailPathFn cb, void* arg);

	RailFile* mFile;  // _00
	RailArray mPaths; // _04
};

// What searchOpen and searchAll pass through RailFile::search
struct RailCall {
	RailMgr* mMgr;   // _00
	RailPathFn mCb;  // _04
	void* mArg;      // _08
};

extern "C" {
void railMgrCreate();
void railMgrDestroy();
void railMgrLoad(RailSet* set);
int railSearch(f32 radius, const Vec3f* pos, RailPathFn cb, void* arg);
int railSearchAll(f32 radius, const Vec3f* pos, RailPathFn cb, void* arg);
RailPath* railFind(u32 id);
}

#endif
