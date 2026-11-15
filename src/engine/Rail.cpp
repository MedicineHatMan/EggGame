#include "egg/ObjEgg.h"
#include "engine/Rail.h"
#include "core/Mem.h"
#include "core/VecMath.h"
#include "engine/WorldMesh.h"
#include "Ninja/ninja.h"

// The stage's rails, 0x800A4348-0x800A5B64: the rider (EggCollide), the rail geometry, the sphere search over the
// rails' quad tree, and the manager that owns one RailPath per rail. The list and array code at the end is MW
// template instances. The names are ours.

extern "C" {
WorldVector<u8>* worldByteVecDtor(WorldVector<u8>*, s16);
}

static RailMgr* sRailMgr;

typedef char EggCollide_size_check[(sizeof(EggCollide) == 0x1C) ? 1 : -1];
typedef char RailPath_size_check[(sizeof(RailPath) == 0x14) ? 1 : -1];
typedef char RailData_size_check[(sizeof(RailData) == 0x18) ? 1 : -1];
typedef char RailNode_size_check[(sizeof(RailNode) == 0x20) ? 1 : -1];
typedef char RailFile_size_check[(sizeof(RailFile) == 0x10) ? 1 : -1];
typedef char RailMgr_size_check[(sizeof(RailMgr) == 0x10) ? 1 : -1];

// 0x800A4348
EggCollide::EggCollide()
{
	mSeg  = 0;
	mDist = 0.0f;
	mPath = 0;
}

// 0x800A436C
EggCollide::~EggCollide()
{
	if (mLink.mList != 0) {
		mPath->mRiders.remove(&mLink);
	}
}

// 0x800A43CC
void EggCollide::getNormal(Vec3f* out) const
{
	RailPath* path = mPath;
	if (path != 0) {
		path->getNormal(mSeg, mDist, out);
	}
}

// 0x800A4408
f32 EggCollide::getDistance() const
{
	RailPath* path = mPath;
	if (path != 0) {
		return path->getDistance(mSeg, mDist);
	}
	return 0.0f;
}

// 0x800A4448
void EggCollide::getDir(Vec3f* out) const
{
	RailPath* path = mPath;
	if (path != 0) {
		path->getDir(mSeg, mDist, out);
	}
}

// 0x800A4484
void EggCollide::getPos(Vec3f* out) const
{
	RailPath* path = mPath;
	if (path != 0) {
		path->getPos(mSeg, mDist, out);
	}
}

// 0x800A44C0
void EggCollide::setPoint(const RailPoint* point)
{
	mSeg  = point->mSeg;
	mDist = point->mDist;
}

// 0x800A44D4
void EggCollide::setPath(RailPath* path)
{
	if (mLink.mList != 0) {
		mPath->mRiders.remove(&mLink);
	}
	if (path != 0) {
		path->mRiders.append(&mLink);
	}
	mPath = path;
}

// 0x800A4534
void EggCollide::advance(f32 dist)
{
	if (mPath != 0) {
		mSeg = mPath->findSeg(dist + getDistance(), mSeg, &mDist);
	}
}

// 0x800A4598
RailPath::RailPath()
{
}

// 0x800A45CC. The riders are left on no rail.
RailPath::~RailPath()
{
	for (RailLink* link = mRiders.mHead; link != 0; link = link->mNext) {
		link->mOwner->mPath = 0;
	}
}

// 0x800A463C
f32 RailPath::getLength()
{
	return mData->mLength;
}

// 0x800A4648. Finds the segment at a distance along the rail, searching from seg.
int RailPath::findSeg(f32 dist, int seg, f32* rest)
{
	RailData* data = mData;
	if (dist < 0.0f) {
		*rest = 0.0f;
		return 0;
	}
	if (dist >= data->mLength) {
		*rest = 0.0f;
		if (data->mFlags & 1) {
			return 0;
		}
		return data->mCount - 1;
	}

	if (dist < data->mDist[seg]) {
		while (--seg >= 0) {
			if (dist >= data->mDist[seg]) {
				break;
			}
		}
	} else if (dist >= data->mDist[seg + 1]) {
		while (++seg < data->mCount) {
			if (dist < data->mDist[seg + 1]) {
				break;
			}
		}
	}
	*rest = dist - data->mDist[seg];
	return seg;
}

// 0x800A4724
void RailPath::getNormal(int seg, f32 dist, Vec3f* out)
{
	switch (mData->mKind) {
	case RAIL_FREE:
		*out = mData->mVerts->mNormal[seg];
		break;
	case RAIL_FLAT:
		out->x = 0.0f;
		out->y = 1.0f;
		out->z = 0.0f;
		break;
	}
}

// 0x800A4784
f32 RailPath::getDistance(int seg, f32 dist)
{
	return dist + mData->mDist[seg];
}

// 0x800A479C
void RailPath::getDir(const RailPoint* point, Vec3f* out)
{
	getDir(point->mSeg, point->mDist, out);
}

// 0x800A47C8
f32 RailData::getFrac(int seg, f32 dist)
{
	if (0.0f == dist) {
		return 0.0f;
	}
	return dist / (mDist[seg + 1] - mDist[seg]);
}

// 0x800A47FC
void RailData::getPosFree(int seg, f32 dist, Vec3f* out)
{
	Vec3f from;
	Vec3f to;
	from = mVerts->mPos[seg];
	to   = mVerts->mPos[seg + 1];
	vecSub(&to, &to, &from);
	vecScaleAdd(getFrac(seg, dist), out, &to, &from);
}

// 0x800A48BC
void RailData::getPosFlat(int seg, f32 dist, Vec3f* out)
{
	Vec3f from;
	Vec3f to;
	from = mVerts->mPos[seg];
	to   = mVerts->mPos[seg + 1];
	vecSub(&to, &to, &from);
	vecScaleAdd(getFrac(seg, dist), out, &to, &from);
}

// 0x800A497C. The last point uses the segment before it.
void RailData::getDirFree(int seg, Vec3f* out)
{
	Vec3f from;
	Vec3f to;
	RailVerts* verts = mVerts;
	if (seg == mCount - 1) {
		from = verts->mPos[seg - 1];
		to   = verts->mPos[seg];
	} else {
		from = verts->mPos[seg];
		to   = verts->mPos[seg + 1];
	}
	vecSub(&to, &to, &from);
	vecNormalize(out, &to);
}

// 0x800A4A68
void RailData::getDirFlat(int seg, Vec3f* out)
{
	Vec3f from;
	Vec3f to;
	RailVerts* verts = mVerts;
	if (seg == mCount - 1) {
		from = verts->mPos[seg - 1];
		to   = verts->mPos[seg];
	} else {
		from = verts->mPos[seg];
		to   = verts->mPos[seg + 1];
	}
	vecSub(&to, &to, &from);
	vecNormalize(out, &to);
}

// 0x800A4B54
void RailPath::getDir(int seg, f32 dist, Vec3f* out)
{
	switch (mData->mKind) {
	case RAIL_FREE:
		mData->getDirFree(seg, out);
		break;
	case RAIL_FLAT:
		mData->getDirFlat(seg, out);
		break;
	}
}

// 0x800A4B9C
void RailPath::getPos(int seg, f32 dist, Vec3f* out)
{
	switch (mData->mKind) {
	case RAIL_FREE:
		mData->getPosFree(seg, dist, out);
		break;
	case RAIL_FLAT:
		mData->getPosFlat(seg, dist, out);
		break;
	}
}

// 0x800A4BE4. Tests segments first..last-1 and then points first..last against the sphere. Counts every touch.
int RailData::hitFree(int first, int last, const Vec3f* center, RailHitFn cb, void* arg, int* more, f32 radius)
{
	Vec3f near;
	Vec3f line;
	Vec3f rel;
	Vec3f from;
	Vec3f to;
	Vec3f at;
	RailPoint point;

	int seg;
	f32 limit        = radius * radius;
	RailVerts* verts = mVerts;
	int hits         = 0;

	for (seg = first; seg < last && *more != 0; seg++) {
		from = verts->mPos[seg];
		to   = verts->mPos[seg + 1];
		vecSub(&line, &to, &from);
		vecSub(&rel, center, &from);
		f32 t = njInnerProduct(&line, &line);
		t     = njInnerProduct(&rel, &line) / t;
		if (t < 0.0f || t > 1.0f) {
			continue;
		}
		vecScaleAdd(t, &near, &line, &from);
		vecSub(&line, &near, center);
		if (njScalor2(&line) > limit) {
			continue;
		}
		hits++;
		if (cb != 0) {
			point.mSeg  = seg;
			point.mDist = t * (mDist[seg + 1] - mDist[seg]);
			cb(this, &point, &near, arg, more);
		}
	}

	if (*more == 0) {
		return hits;
	}

	for (seg = first; seg <= last && *more != 0; seg++) {
		at.set(verts->mPos[seg].x, verts->mPos[seg].y, verts->mPos[seg].z);
		vecSub(&line, center, &at);
		if (njScalor2(&line) > limit) {
			continue;
		}
		hits++;
		if (cb != 0) {
			point.mSeg  = seg;
			point.mDist = 0.0f;
			cb(this, &point, &near, arg, more);
		}
	}
	return hits;
}

// 0x800A4E64. As hitFree, but only counts the touches the callback accepts.
int RailData::hitFlat(int first, int last, const Vec3f* center, RailHitFn cb, void* arg, int* more, f32 radius)
{
	Vec3f near;
	Vec3f line;
	Vec3f rel;
	Vec3f from;
	Vec3f to;
	Vec3f at;
	RailPoint point;

	int seg;
	f32 limit        = radius * radius;
	RailVerts* verts = mVerts;
	int hits         = 0;

	for (seg = first; seg < last && *more != 0; seg++) {
		from = verts->mPos[seg];
		to   = verts->mPos[seg + 1];
		vecSub(&line, &to, &from);
		vecSub(&rel, center, &from);
		f32 t = njInnerProduct(&line, &line);
		t     = njInnerProduct(&rel, &line) / t;
		if (t < 0.0f || t > 1.0f) {
			continue;
		}
		vecScaleAdd(t, &near, &line, &from);
		vecSub(&line, &near, center);
		if (njScalor2(&line) > limit) {
			continue;
		}
		if (cb != 0) {
			point.mSeg  = seg;
			point.mDist = t * (mDist[seg + 1] - mDist[seg]);
			if (cb(this, &point, &near, arg, more) != 0) {
				hits++;
			}
		}
	}

	if (*more == 0) {
		return hits;
	}

	for (seg = first; seg <= last && *more != 0; seg++) {
		at.set(verts->mPos[seg].x, verts->mPos[seg].y, verts->mPos[seg].z);
		vecSub(&line, center, &at);
		if (njScalor2(&line) > limit) {
			continue;
		}
		if (cb != 0) {
			point.mSeg  = seg;
			point.mDist = 0.0f;
			if (cb(this, &point, &near, arg, more) != 0) {
				hits++;
			}
		}
	}
	return hits;
}

// 0x800A50F4
RailFile::RailFile()
{
	worldByteVecCtor(&mSeen);
	mSet = 0;
}

// 0x800A5130
RailFile::~RailFile()
{
	worldByteVecDtor(&mSeen, -1);
}

// 0x800A5180
void RailFile::setSet(RailSet* set)
{
	mSet = set;
}

// 0x800A5188. Walks the quad tree from one node, testing the rails in each leaf the box reaches.
int RailFile::walk(const RailBox* box, int index, int* more, int hits, const RailSphere* sphere)
{
	RailNode* node = &mSet->mNodes[index];
	if (box->mMax.x < node->mMinX || box->mMin.x > node->mMaxX || box->mMax.z < node->mMinZ || box->mMin.z > node->mMaxZ) {
		return hits;
	}

	if (node->mLeaf == 0) {
		for (int i = 0; i < 4 && *more != 0; i++) {
			hits = walk(box, node->mChild[i], more, hits, sphere);
		}
	} else {
		RailLeaf* leaf   = (RailLeaf*)(mSet->mEntries + (node->mEntries & ~1));
		RailEntry* entry = leaf->mEntries;
		for (int i = 0; i < node->mCount && *more != 0; i++) {
			RailEntry* e = entry++;
			hits += mSet->mRails[e->mRail].hit(e->mFirst, e->mLast, &sphere->mCenter, sphere->mCb, sphere->mArg, more, sphere->mRadius);
		}
	}
	return hits;
}

// 0x800A52EC
int RailFile::search(const Vec3f* pos, RailHitFn cb, void* arg, int* more, f32 radius)
{
	if (mSet == 0) {
		return 0;
	}

	RailSphere sphere;
	RailBox box;
	sphere.mCb       = cb;
	sphere.mArg      = arg;
	box.mMin.x       = pos->x - radius;
	box.mMin.y       = pos->y - radius;
	box.mMin.z       = pos->z - radius;
	box.mMax.x       = pos->x + radius;
	box.mMax.y       = pos->y + radius;
	box.mMax.z       = pos->z + radius;
	sphere.mCenter.x = pos->x;
	sphere.mCenter.y = pos->y;
	sphere.mCenter.z = pos->z;
	sphere.mRadius   = radius;
	return walk(&box, 0, more, 0, &sphere);
}

// 0x800A538C
int RailData::hit(int first, int last, const Vec3f* center, RailHitFn cb, void* arg, int* more, f32 radius)
{
	switch (mKind) {
	case RAIL_FREE:
		return hitFree(first, last, center, cb, arg, more, radius);
	case RAIL_FLAT:
		return hitFlat(first, last, center, cb, arg, more, radius);
	}
	return 0;
}

// 0x800A53D8
RailMgr::RailMgr()
{
	mFile = new RailFile;
}

// 0x800A5428
RailMgr::~RailMgr()
{
	clear();
	delete mFile;
}

// 0x800A5488
void RailMgr::clear()
{
	for (RailPath** it = mPaths.mData; it != mPaths.mData + mPaths.mCount; it++) {
		delete *it;
	}
}

// 0x800A54E0
RailPath* RailMgr::find(u32 id)
{
	RailSet* set = mFile->mSet;
	if (set != 0) {
		for (int i = 0; i < set->mCount; i++) {
			if (id == set->mRails[i].mId) {
				return mPaths.mData[i];
			}
		}
	}
	return 0;
}

// 0x800A5540
void RailMgr::load(RailSet* set)
{
	clear();
	mFile->setSet(set);
	mPaths.resize(set->mCount);
	for (int i = 0; i < set->mCount; i++) {
		mPaths.mData[i]        = new RailPath;
		mPaths.mData[i]->mData = &set->mRails[i];
	}
}

static int railHitOpen(RailData* rail, RailPoint* point, Vec3f* pos, void* arg, int* more);
static int railHitAll(RailData* rail, RailPoint* point, Vec3f* pos, void* arg, int* more);

// 0x800A55E4. Only finds rails of type 0.
int RailMgr::searchOpen(f32 radius, const Vec3f* pos, RailPathFn cb, void* arg)
{
	RailCall call;
	int more;
	more      = 1;
	call.mMgr = this;
	call.mCb  = cb;
	call.mArg = arg;
	return mFile->search(pos, railHitOpen, &call, &more, radius);
}

// 0x800A5630
static int railHitOpen(RailData* rail, RailPoint* point, Vec3f* pos, void* arg, int* more)
{
	if (rail->mType != 0) {
		return 0;
	}
	RailCall* call = (RailCall*)arg;
	if (call->mCb != 0) {
		RailMgr* mgr = call->mMgr;
		call->mCb(mgr->mPaths.mData[rail - mgr->mFile->mSet->mRails], point, pos, call->mArg, more);
	}
	return 1;
}

// 0x800A56A4
int RailMgr::searchAll(f32 radius, const Vec3f* pos, RailPathFn cb, void* arg)
{
	RailCall call;
	int more;
	more      = 1;
	call.mMgr = this;
	call.mCb  = cb;
	call.mArg = arg;
	return mFile->search(pos, railHitAll, &call, &more, radius);
}

// 0x800A56F0
static int railHitAll(RailData* rail, RailPoint* point, Vec3f* pos, void* arg, int* more)
{
	RailCall* call = (RailCall*)arg;
	if (call->mCb != 0) {
		RailMgr* mgr = call->mMgr;
		call->mCb(mgr->mPaths.mData[rail - mgr->mFile->mSet->mRails], point, pos, call->mArg, more);
	}
	return 1;
}

// 0x800A5750
void railMgrCreate()
{
	if (sRailMgr == 0) {
		sRailMgr = new RailMgr;
	}
}

// 0x800A5794
void railMgrDestroy()
{
	if (sRailMgr != 0) {
		delete sRailMgr;
		sRailMgr = 0;
	}
}

// 0x800A57CC
void railMgrLoad(RailSet* set)
{
	if (sRailMgr != 0) {
		sRailMgr->load(set);
	}
}

// 0x800A5800
int railSearch(f32 radius, const Vec3f* pos, RailPathFn cb, void* arg)
{
	if (sRailMgr != 0) {
		return sRailMgr->searchOpen(radius, pos, cb, arg);
	}
	return 0;
}

// 0x800A584C
int railSearchAll(f32 radius, const Vec3f* pos, RailPathFn cb, void* arg)
{
	if (sRailMgr != 0) {
		return sRailMgr->searchAll(radius, pos, cb, arg);
	}
	return 0;
}

// 0x800A5898
RailPath* railFind(u32 id)
{
	if (sRailMgr != 0) {
		return sRailMgr->find(id);
	}
	return 0;
}

// 0x800A58D4
RailList::RailList()
{
	mHead  = 0;
	mTail  = 0;
	mCount = 0;
}

// 0x800A58E8
RailList::~RailList()
{
	clear();
}

// 0x800A5930
void RailList::append(RailLink* link)
{
	if (mHead == 0) {
		mTail = link;
		mHead = link;
	} else {
		link->mPrev  = mTail;
		mTail->mNext = link;
		mTail        = link;
	}
	link->mList = this;
	mCount++;
}

// 0x800A5970
void RailList::remove(RailLink* link)
{
	if (link->mPrev != 0) {
		link->mPrev->mNext = link->mNext;
	} else {
		mHead = link->mNext;
	}
	if (link->mNext != 0) {
		link->mNext->mPrev = link->mPrev;
	} else {
		mTail = link->mPrev;
	}
	link->mNext = 0;
	link->mPrev = 0;
	link->mList = 0;
	mCount--;
}

// 0x800A59D0
void RailList::clear()
{
	RailLink* link = mHead;
	while (link != 0) {
		RailLink* next = link->mNext;
		link->mList    = 0;
		link->mNext    = 0;
		link->mPrev    = 0;
		link           = next;
	}
}

// 0x800A59FC
RailArray::RailArray()
{
	mData  = 0;
	mCount = 0;
	mCap   = 0;
}

// 0x800A5A10
RailArray::~RailArray()
{
	if (mData != 0) {
		delete[] mData;
	}
}

// 0x800A5A64
void RailArray::resize(int count)
{
	if (count != mCount) {
		reserve(count);
		mCount = count;
	}
}

// 0x800A5AA4
void RailArray::reserve(int count)
{
	if (count > mCap) {
		RailPath** old = mData;
		alloc(count);
		if (old != 0) {
			copy(mData, old, mCount);
			delete[] old;
		}
	}
}

// 0x800A5B04
void RailArray::alloc(int count)
{
	mData = new RailPath*[count];
	mCap  = count;
}

// 0x800A5B40
void RailArray::copy(RailPath** to, RailPath* const* from, int count)
{
	for (int i = 0; i < count; i++) {
		to[i] = from[i];
	}
}
