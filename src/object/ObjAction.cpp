#define findObjById findObjById_u16
#include "object/ObjAction.h"
#undef findObjById
#include "core/dolphin.h"
#include "sdk/stl/math.h"
#include "io/NinjaFile.h"
#include "core/PlacementNew.h"
#include "stl/math.h"
#include "Ninja/ninja.h"
#include "core/SdkCalls.h"

// 0x8000F968
BitFlags::BitFlags()
{
	mBits = 0;
}

// 0x8000F974
void BitFlags::orWith(const BitFlags& other)
{
	mBits |= other.mBits;
}

// 0x8000F988
void BitFlags::operator=(const BitFlags& other)
{
	mBits = other.mBits;
}

// 0x8000F994
BitFlags* BitFlags::setBit(int bit)
{
	mBits |= 1 << bit;
	return this;
}

// 0x8000F9AC
void BitFlags::clear()
{
	mBits = 0;
}

// 0x8000F9B8
int BitFlags::testBit(int bit)
{
	return mBits & (1 << bit);
}

// ---- Hit record -----------------------------------------------------------------------------------------

// 0x8000F9CC
HitInfo::HitInfo()
{
	resetHitInfo(this);
}

// 0x8000FA08
extern "C" void resetHitInfo(HitInfo* hit)
{
	hit->mDamage = 0;
	hit->mFlags.clear();
	hit->mPos.z = hit->mPos.y = hit->mPos.x = 0.0f;
	hit->_14 = 0;
}

// ---- The id table ---------------------------------------------------------------------------------------

// 0x80010004
ObjBucket::ObjBucket()
{
	mNext = this;
	mPrev = this;
}

// 0x80010010
extern "C" void unlinkObjLink(void*, ObjLink* link)
{
	link->mPrev->mNext = link->mNext;
	link->mNext->mPrev = link->mPrev;
}

// 0x8001002C
// Goes at the tail walking follows mNext from the head
extern "C" void appendObjLink(ObjLink* head, ObjLink* link)
{
	link->mPrev = head->mPrev;
	link->mNext = head;
	head->mPrev->mNext = link;
	head->mPrev = link;
}

// 0x80010048
extern "C" int hashObjId(u16 id)
{
	return id;
}

// The link on the table's list of all entries with the entry it is in
struct ObjAllLink : public ObjLink {
	ObjIdEntry* mEntry; // _08
};

struct ObjIdEntry { // size 0x24
	ObjIdEntry();
	~ObjIdEntry();

	ObjLink mBucketLink; // _00 While free mNext is the free list
	TObjAction* mObj;    // _08
	ObjRefList mRefs;    // _0C
	ObjAllLink mAllLink; // _18
};

// 0x80010230
ObjIdEntry::ObjIdEntry()
{
}

// 0x800101E0
ObjIdEntry::~ObjIdEntry()
{
}

class TObjIdTable { // size 0x14
public:
	TObjIdTable();
	~TObjIdTable();

	ObjIdEntry* add(u16 id, int* isNew);
	void remove(ObjIdEntry* entry);
	ObjIdEntry* find(u16 id);
	ObjIdEntry* findInBucket(ObjBucket* bucket, u16 id);
	ObjIdIter iterate();

	ObjBucket* mBuckets;  // _00
	ObjBucket* mAll;      // _04
	ObjIdEntry* mFree;    // _08
	ObjIdEntry* mEntries; // _0C
	int mCount;           // _10
};

// 0x800100D4
TObjIdTable::TObjIdTable()
{
	mBuckets = new ObjBucket[113];
	mAll = new ObjBucket;
	mEntries = new ObjIdEntry[1024];

	for (int i = 0; i < 1023; i++) {
		mEntries[i].mObj = 0;
		mEntries[i].mRefs.mOwner = &mEntries[i];
		mEntries[i].mAllLink.mEntry = &mEntries[i];
		mEntries[i].mBucketLink.mNext = (ObjLink*)&mEntries[i + 1];
	}
	mEntries[1023].mBucketLink.mNext = 0;
	mFree = mEntries;
}

// 0x80010264
TObjIdTable::~TObjIdTable()
{
	delete[] mEntries;
	delete mAll;
	delete[] mBuckets;
}

// 0x800102D4
// Returns the existing entry when the id is already in and null when none are free
#pragma opt_common_subs off
ObjIdEntry* TObjIdTable::add(u16 id, int* isNew)
{
	if (id == 0xFFFF) {
		return 0;
	}
	int h;
	ObjIdEntry* entry;
	ObjIdEntry* found;
	h = hashObjId(id) % 113;
	found = findInBucket(&mBuckets[h], id);
	if (found != 0) {
		if (isNew != 0) {
			*isNew = 0;
		}
		return found;
	}
	entry = mFree;
	if (entry != 0) {
		mFree = (ObjIdEntry*)entry->mBucketLink.mNext;
		appendObjLink(&mBuckets[h], &entry->mBucketLink);
		if (isNew != 0) {
			*isNew = 1;
		}
		appendObjLink(mAll, &entry->mAllLink);
		mCount++;
	}
	return entry;
}
#pragma opt_common_subs reset

// 0x800103B4
void TObjIdTable::remove(ObjIdEntry* entry)
{
	entry->mBucketLink.mPrev->mNext = entry->mBucketLink.mNext;
	entry->mBucketLink.mNext->mPrev = entry->mBucketLink.mPrev;
	entry->mBucketLink.mNext = (ObjLink*)mFree;
	mFree = entry;
	unlinkObjLink(mAll, &entry->mAllLink);
	mCount--;
	entry->mRefs.clear();
}

// 0x80010424
ObjIdEntry* TObjIdTable::find(u16 id)
{
	if (id == 0xFFFF) {
		return 0;
	}
	int h = hashObjId(id) % 113;
	return findInBucket(&mBuckets[h], id);
}

// 0x80010494
ObjIdEntry* TObjIdTable::findInBucket(ObjBucket* bucket, u16 id)
{
	for (ObjLink* link = bucket->mNext; link != bucket; link = link->mNext) {
		if (id == ((ObjIdEntry*)link)->mObj->mObjId) {
			return (ObjIdEntry*)link;
		}
	}
	return 0;
}

// 0x80010050
extern "C" void createObjIdTable()
{
	if (gObjTable == 0) {
		gObjTable = new TObjIdTable;
	}
}

// 0x80010094
extern "C" void destroyObjIdTable()
{
	if (gObjTable != 0) {
		delete (TObjIdTable*)gObjTable;
		gObjTable = 0;
	}
}

// 0x800100CC
extern "C" TObjIdTable* getObjIdTable()
{
	return (TObjIdTable*)gObjTable;
}

// 0x8000FFCC
extern "C" ObjIdIter objIdIter()
{
	return getObjIdTable()->iterate();
}

// 0x8000FA58
// The ranges start at 0 0x1000 which wraps at 0x2000 0x2000 0x4000 and 0x8000
extern "C" void initObjIds()
{
	createObjIdTable();
	gNextObjId0 = 0;
	gNextObjId1 = 0x1000;
	gNextObjId2 = 0x2000;
	gNextObjId3 = 0x4000;
	gNextObjId4 = 0x8000;
	gRangeCheck = 0;
}

// 0x8000FAA8
extern "C" void shutdownObjIds()
{
	destroyObjIdTable();
}

// 0x8000FAC8
extern "C" TObjAction* findObjById(int id)
{
	if (id == 0xFFFF) {
		return 0;
	}
	ObjIdEntry* entry = getObjIdTable()->find(id);
	if (entry != 0) {
		return entry->mObj;
	}
	return 0;
}

// ---- TObjAction -----------------------------------------------------------------------------------------

// 0x8000FB24
u16 TObjAction::registerId()
{
	u16 id = mObjId;
	if (id == 0xFFFF) {
		return id;
	}
	if (getKind() == 5) {
		return id;
	}
	int isNew;
	ObjIdEntry* entry = getObjIdTable()->add(id, &isNew);
	if (entry != 0) {
		entry->mObj = this;
	}
	mIdEntry = entry;
	return id;
}

// 0x8000FBA4
u16 TObjAction::unregisterId()
{
	u16 id = mObjId;
	if (id == 0xFFFF) {
		return id;
	}
	ObjIdEntry* entry = (ObjIdEntry*)mIdEntry;
	if (entry != 0) {
		getObjIdTable()->remove(entry);
		mIdEntry = 0;
	}
	return id;
}

const char* gTObjActionName = "TObjAction";

// 0x8000FC04
TObjAction::TObjAction(TObject* parent)
	: TObject(parent)
{
	_00 = (u32)gTObjActionName;
	mSize = 0x30;
	mPrevAction = -1;
	mAction = -1;
	mPlayer = -1;
	mObjId = 0xFFFF;
	mHolderId = 0xFFFF;
	mIdEntry = 0;
}

// 0x8000FC78
TObjAction::~TObjAction()
{
}

// 0x8000FCD4
// reserve extra ids are skipped after the one taken
u16 TObjAction::assignId(u32 reserve)
{
	if (mObjId != 0xFFFF) {
		return mObjId;
	}
	{
		switch (getKind()) {
		case 0:
			mObjId = gNextObjId0;
			gNextObjId0 += (u16)reserve + 1;
			break;
		case 1:
			mObjId = gNextObjId1;
			gNextObjId1 += (u16)reserve + 1;
			if (gNextObjId1 > 0x1FFF) {
				gNextObjId1 = 0x1000;
			}
			break;
		case 2:
			mObjId = gNextObjId2;
			gNextObjId2 += (u16)reserve + 1;
			break;
		case 3:
			mObjId = gNextObjId3;
			gNextObjId3 += (u16)reserve + 1;
			break;
		case 4:
			mObjId = gNextObjId4;
			gNextObjId4 += (u16)reserve + 1;
			break;
		}
	}
	return mObjId;
}

// 0x8000FE00
extern "C" void setObjId(TObjAction* obj, u16 id)
{
	if (id == 0xFFFF) {
		obj->assignId(0);
	} else {
		obj->mObjId = id;
	}
	obj->registerId();
}

// 0x8000FE4C
extern "C" void releaseObjId(TObjAction* obj)
{
	obj->unregisterId();
}

// 0x8000FE6C
void TObjAction::setAction(int action)
{
	if (mAction != -1) {
		exitAction(mAction);
	}
	mPrevAction = mAction;
	mAction = action;
	if (action != -1) {
		enterAction(action);
	}
}

// The empty and default virtuals

// 0x8000FEE0
void TObjAction::enterAction(int) { }

// 0x8000FEE4
void TObjAction::exitAction(int) { }

// 0x8000FEE8
void TObjAction::vfn_68(int) { }

// 0x8000FEEC
void TObjAction::vfn_64(int) { }

// 0x8000FEF0
extern "C" int objDebugEnabled(TObject* obj)
{
	return debugInfoActive();
}

// 0x8000FF10
void TObjAction::initFromDesc(SetDesc*) { }

// 0x8000FF14
void TObjAction::readEnemyDesc(SetDesc*) { }

// 0x8000FF18
void TObjAction::debugPrint(void*) { }

// 0x8000FF1C
void TObjAction::vfn_3C() { }

// 0x8001082C
int TObjAction::getRotY() { return 0; }

// 0x80010834
int TObjAction::canAct() { return 0; }

// ---- Range test -----------------------------------------------------------------------------------------

// 0x8000FF20
// Returns the old value
extern "C" int setRangeCheck(int on)
{
	int old = gRangeCheck;
	gRangeCheck = on;
	return old;
}

// 0x8000FF30
extern "C" void setRangeCenter(const Vec3f* center)
{
	gRangeCenter = *center;
}

// 0x8000FF50
// The first argument is never read
extern "C" int isInRange(void*, const Vec3f* pos, f32 range)
{
	if (debugEditActive() != 0) {
		return true;
	}
	if (gRangeCheck != 0) {
		return njDistanceP2P(pos, &gRangeCenter) <= range;
	}
	return true;
}

// ---- Heap -----------------------------------------------------------------------------------------------

// 0x80010880
extern "C" ObjHeap* objHeapInit(ObjHeap* heap, u32 size, u32 align)
{
	heap->mAlign = align;
	heap->mHead = (HeapBlock*)syMalloc(size);
	if (heap->mHead != 0) {
		memset(heap->mHead, 0, size);
		HeapBlock* first     = heap->mHead + 1;
		heap->mHead->mNext = first;
		heap->mHead->mSize = 0;
		first->mNext       = 0;
		first->mSize = size - sizeof(HeapBlock);
	}
	return heap;
}

// 0x800108FC
extern "C" ObjHeap* objHeapDestroy(ObjHeap* heap, s16 flag)
{
	if (heap != 0) {
		syFree(heap->mHead);
		if (flag > 0) {
			objFree(heap);
		}
	}
	return heap;
}

// 0x80010948
extern "C" void* objHeapAlloc(void* heapArg, u32 size)
{
	// The heap pointer becomes the block before the one being looked at
	HeapBlock* prev = (HeapBlock*)heapArg;
	HeapBlock* block;
	u32 align = ((ObjHeap*)prev)->mAlign;
	HeapBlock* rest;
	u32 need;
	prev = ((ObjHeap*)prev)->mHead;
	need = align + size;
	need += 7;
	need &= -align;
	for (; (block = prev->mNext) != 0; prev = block) {
		if (block->mSize >= need) {
			if (need == block->mSize) {
				prev->mNext = block->mNext;
			} else {
				rest = (HeapBlock*)((u8*)block + need);
				rest->mNext  = block->mNext;
				rest->mSize  = block->mSize - need;
				block->mSize = need;
				prev->mNext  = rest;
			}
			return block + 1;
		}
	}
	return 0;
}

// The block header in front of an allocation
#define MEM_BLOCK(mem) ((HeapBlock*)(mem) - 1)

// 0x800109C4
// Built without common-subexpression elimination the DOL works the header address out again at each use
#pragma opt_common_subs off
extern "C" void objHeapFree(void* heapArg, void* mem)
{
	ObjHeap* heap = (ObjHeap*)heapArg;
	if (mem == 0) {
		return;
	}
	HeapBlock* prev;
	HeapBlock* next;
	for (prev = heap->mHead; (next = prev->mNext) < MEM_BLOCK(mem); prev = next) {
	}
	if ((HeapBlock*)((u8*)MEM_BLOCK(mem) + MEM_BLOCK(mem)->mSize) == next) {
		MEM_BLOCK(mem)->mNext = next->mNext;
		MEM_BLOCK(mem)->mSize += next->mSize;
	} else {
		MEM_BLOCK(mem)->mNext = next;
	}
	if ((HeapBlock*)((u8*)prev + prev->mSize) == MEM_BLOCK(mem)) {
		prev->mNext = MEM_BLOCK(mem)->mNext;
		prev->mSize += MEM_BLOCK(mem)->mSize;
	} else {
		prev->mNext = MEM_BLOCK(mem);
	}
}
#pragma opt_common_subs reset

// 0x800104E8
ObjRef::ObjRef()
{
	mList = 0;
}

// 0x800104F4
ObjRef::~ObjRef()
{
	unlink();
}

// 0x8001053C
// The list sits inside the id table entry and its _08 leads to the object
TObjAction* ObjRef::get()
{
	if (mList != 0) {
		return mList->mOwner->mObj;
	}
	return 0;
}

// 0x8001055C
u16 ObjRef::getId()
{
	if (mList != 0) {
		return mList->mOwner->mObj->mObjId;
	}
	return 0xFFFF;
}

// 0x80010584
ObjRef& ObjRef::setId(u16 id)
{
	link(id);
	return *this;
}

// 0x800105B4
ObjRef& ObjRef::assign(const ObjRef& other)
{
	unlink();
	objRefAttach(other.mList, this);
	return *this;
}

// 0x800105F4
void ObjRef::link(u16 id)
{
	unlink();
	ObjIdEntry* entry = getObjIdTable()->find(id);
	if (entry != 0) {
		objRefAttach(&entry->mRefs, this);
	}
}

// 0x80010644
void ObjRef::unlink()
{
	if (mList != 0) {
		objRefDetach(mList, this);
	}
}

// 0x800106E0
extern "C" void objRefAttach(ObjRefList* list, ObjRef* ref)
{
	appendObjLink(&list->mLink, &ref->mLink);
	ref->mList = list;
}

// 0x800104C4
ObjIdIter TObjIdTable::iterate()
{
	return ObjIdIter(mAll);
}

// 0x80010674
ObjRefList::ObjRefList()
{
}

// 0x800106A4
ObjRefList::~ObjRefList() { }

// 0x80010714
extern "C" void objRefDetach(ObjRefList* list, ObjRef* ref)
{
	unlinkObjLink(list, &ref->mLink);
	ref->mList = 0;
}

// 0x80010748
// The references are left pointing at nothing not unlinked one by one
void ObjRefList::clear()
{
	for (ObjLink* link = mLink.mNext; link != &mLink;) {
		ObjLink* next          = link->mNext;
		((ObjRef*)link)->mList = 0;
		link                   = next;
	}
	mLink.mPrev = &mLink;
	mLink.mNext = &mLink;
}

// 0x80010774
ObjIdIter::ObjIdIter() { }

// 0x80010778
ObjIdIter::ObjIdIter(ObjLink* head)
{
	mHead = head;
}

// 0x80010780
void ObjIdIter::begin()
{
	mCur = mHead->mNext;
}

// 0x80010790
void ObjIdIter::next()
{
	mCur = mCur->mNext;
}

// 0x800107A0
BOOL ObjIdIter::atEnd()
{
	return mCur == mHead;
}

// 0x800107B8
TObjAction* ObjIdIter::get()
{
	return ((ObjAllLink*)mCur)->mEntry->mObj;
}

// 0x800107C8
TObjEgg* ObjIdIter::getEgg()
{
	return (TObjEgg*)((ObjAllLink*)mCur)->mEntry->mObj;
}

// 0x800107D8
extern "C" int sendMsgById(int id, ObjMsg* msg)
{
	TObjAction* obj = findObjById((u16)id);
	if (obj != 0) {
		return obj->sendToHandle(msg);
	}
	return 0;
}

// A global with nothing to build
struct ObjActionStatic {
	ObjActionStatic() { }
};
static ObjActionStatic sObjActionStatic;

// 0x80010840
extern "C" void* objAlloc(u32 size)
{
	return syMalloc(size);
}

// 0x80010860
extern "C" void objFree(void* mem)
{
	syFree(mem);
}

// ---- Math -----------------------------------------------------------------------------------------------

#define ANGLE_PER_RADIAN 10430.38043493439 // 0x10000 / 2 pi
// 0x80010A54
// Makes v a unit vector and gives its old length
extern "C" f32 normalizeVector(Vec3f* v)
{
	f32 len = njUnitVector(v);
	if (fpclassify(len) == FP_NAN || len == 0.0f) {
		len  = 0.0f;
		v->x = len;
		v->y = len;
		v->z = len;
	}
	return len;
}

// 0x80010AF8
// Turns from toward to by at most step the short way round
extern "C" int angleApproach(int from, int to, int step)
{
	s16 diff = (u16)to - (u16)from;
	if (diff > step || diff < -step) {
		s16 angle = (diff & 0x8000) ? (s16)((u16)from - step) : (s16)((u16)from + step);
		return (u16)angle;
	}
	return (u16)to;
}

// 0x80010B44
// angleApproach also giving the size of the turn left in *left
extern "C" int angleApproachDiff(int from, int to, int step, int* left)
{
	s16 diff = (u16)to - (u16)from;
	if (diff <= step && diff >= -step) {
		if ((s16)diff < 0) {
			*left = -diff;
			return (u16)to;
		}
		*left = diff;
		return (u16)to;
	}
	s16 angle;
	if (diff & 0x8000) {
		*left = -diff;
		angle = (u16)from - step;
	} else {
		angle = (u16)from + step;
		*left = diff;
	}
	return (u16)angle;
}

// 0x80010BB4
// The signed turn from one angle to another
extern "C" int angleDiff(int from, int to)
{
	return (s16)((u16)to - (u16)from);
}

// 0x80010BC8
// The size of the turn between two angles
extern "C" int angleDist(int from, int to)
{
	s16 diff = (u16)to - (u16)from;
	if (diff < 0) {
		diff = -diff;
	}
	return (u16)diff;
}

// 0x80010BEC
// The pitch and yaw that point along v
extern "C" void vectorToAngles(const Vec3f* v, int* pitch, int* yaw)
{
	f32 x = v->x;
	f32 z = v->z;
	f32 a = (f32)atan2(x, z);
	x *= x;
	z *= z;
	x += z;
	f32 r  = __fres((f32)__frsqrte(x));
	*yaw  = (int)(ANGLE_PER_RADIAN * a);
	*pitch = -(int)(ANGLE_PER_RADIAN * (f32)atan2(v->y, r));
}

// 0x80010CAC
// The radius of the circle on which a step of length step turns by angle
extern "C" f32 turnRadius(f32 step, int angle)
{
	return 0.5f * step / njSin(angle / 2);
}

// 0x80010CF4
extern "C" f32 quadDiscriminant(f32 a, f32 b, f32 c)
{
	b = b * b;
	b = b - 4.0f * a * c;
	return b;
}

// 0x80010D0C
// The roots of a x^2 + b x + c the larger first when a is positive
extern "C" void quadRoots(f32 a, f32 b, f32 c, f32* plus, f32* minus)
{
	f32 inv = __fres(a);
	f32 d = b * b;
	d -= 4.0f * a * c;
	f32 half = 0.5f * inv;
	f32 root = __fres((f32)__frsqrte(d));
	if (plus != 0) {
		*plus = half * (-b + root);
	}
	if (minus != 0) {
		*minus = half * (-b - root);
	}
}

// 0x80010D68
extern "C" f32 quadRoot(f32 a, f32 b, f32 c)
{
	f32 root;
	quadRoots(a, b, c, &root, 0);
	return root;
}

// ---- Blending and culling -------------------------------------------------------------------------------

typedef f32 Mtx[3][4];

// Ninja's blend factors in its alphabetical order
enum NjBlend {
	NJ_BLEND_INV_DST_COLOR = 5,
	NJ_BLEND_INV_SRC_ALPHA = 6,
	NJ_BLEND_INV_SRC_COLOR = 7,
	NJ_BLEND_SRC_ALPHA     = 8,
	NJ_BLEND_ONE           = 10,
};

// 0x80010D94
extern "C" void blendModeAdd()
{
	njColorBlendingMode(0, NJ_BLEND_SRC_ALPHA);
	njColorBlendingMode(1, NJ_BLEND_ONE);
}

// 0x80010DC8
extern "C" void blendModeAlpha()
{
	njColorBlendingMode(0, NJ_BLEND_SRC_ALPHA);
	njColorBlendingMode(1, NJ_BLEND_INV_SRC_ALPHA);
}

// 0x80010DFC
extern "C" void blendModeInvert()
{
	njColorBlendingMode(0, NJ_BLEND_INV_DST_COLOR);
	njColorBlendingMode(1, NJ_BLEND_INV_SRC_COLOR);
}

// 0x80010E30
extern "C" void blendModeInvertAlpha()
{
	njColorBlendingMode(0, NJ_BLEND_INV_DST_COLOR);
	njColorBlendingMode(1, NJ_BLEND_INV_SRC_ALPHA);
}

int sCullMode;          // 0x802C4BB0 Ninja's as last set here
int sSavedCullMode;     // 0x802C4BB4
int sSavedGXCull;        // 0x802C4BB8

// 0x80010E64
extern "C" void getCullMode(int* mode)
{
	*mode = sCullMode;
}

// 0x80010E70
extern "C" void setCullMode(int mode)
{
	if (sCullMode != mode) {
		sCullMode = mode;
		njPolygonCullingMode(mode);
	}
}

// 0x80010EA0
extern "C" void pushCullMode(int mode)
{
	getCullMode(&sSavedCullMode);
	setCullMode(mode);
}

// 0x80010ED8
extern "C" void popCullMode()
{
	setCullMode(sSavedCullMode);
}

// 0x80010EFC
extern "C" void pushGXCullMode(int mode)
{
	GXGetCullMode((int*)&sSavedGXCull);
	GXSetCullMode(mode);
}

// 0x80010F34
extern "C" void popGXCullMode()
{
	GXSetCullMode(sSavedGXCull);
}

// ---- Setup lists ----------------------------------------------------------------------------------------

// 0x80010F58
// Runs each pair's setup in order
extern "C" void runInits(const ObjInitPair* pairs, int count)
{
	for (int i = 0; i < count; i++) {
		if (pairs[i].mInit != 0) {
			pairs[i].mInit();
		}
	}
}

// 0x80010FB4
// Runs each pair's teardown last first
extern "C" void runTerms(const ObjInitPair* pairs, int count)
{
	for (int i = count - 1; i >= 0; i--) {
		if (pairs[i].mTerm != 0) {
			pairs[i].mTerm();
		}
	}
}

// ---- Screen matrix and the constant colour --------------------------------------------------------------

// 0x80011010
// A view at distance dist scaled to the projection
extern "C" void makeScreenMatrix(Mtx m, f32 dist)
{
	f32 scale = ((f32*)&gProjection)[21] * dist;
	PSMTXIdentity((f32*)m);
	m[0][0] = scale;
	m[1][1] = scale;
	m[2][3] = -dist;
}

// 0x80011080
extern "C" void loadScreenMatrix(f32 dist, int id)
{
	Mtx m;
	makeScreenMatrix(m, dist);
	GXLoadPosMtxImm((const float*)m, id);
	PSMTXIdentity((f32*)m);
	GXLoadNrmMtxImm((const float*)m, id);
	GXSetCurrentMtx(id);
}

// 0x800110D8
// A TEV order callback the stage blends the texture toward the constant colour by its alpha
static void kColorTevOrder(int stage, int coord, int map, int color)
{
	GXSetTevKColorSel(stage, GX_TEV_KCSEL_K0);
	GXSetTevKAlphaSel(stage, GX_TEV_KASEL_K0_A);
	GXSetTevColorIn(stage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_KONST);
	GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_KONST);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevOrder(stage, coord, map, color);
}

// 0x8001119C
static void setKColorTev()
{
	void* funcs[10];
	funcs[3] = (void*)kColorTevOrder;
	gjChangeDrawObjIF(funcs, 0x20);
}

// 0x800111D0
// Draws after this add color
extern "C" void beginKColor(GXColor* color)
{
	setKColorTev();
	GXSetTevKColor(GX_KCOLOR0, *color);
}

// 0x80011210
extern "C" void endKColor()
{
	void* funcs[10];
	funcs[3] = 0;
	gjChangeDrawObjIF(funcs, 0x20);
}

// ---- Debug shapes ---------------------------------------------------------------------------------------

// 0x80011240
// Flat see-through unlit colour for the shapes below
static void debugDrawSetup(GXColor* color)
{
	GXSetNumTexGens(0);
	GXSetNumChans(1);
	GXSetNumTevStages(1);
	GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GXSetChanCtrl(GX_COLOR0, GX_TRUE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
	GXSetChanCtrl(GX_ALPHA0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	GXSetChanMatColor(GX_COLOR0A0, *color);
	gjLoadPosMatrix(0);
	gjLoadNrmMatrix(0);
	GXSetCurrentMtx(GX_PNMTX0);
	GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0xFF);
	gjGXSetBlendMode(1);
	GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
}

// 0x8001135C
// The colour defaults to red
static const GXColor sDebugRed = { 0xFF, 0x20, 0x20, 0x80 };

extern "C" void debugSphere(const Vec3f* pos, GXColor* color, f32 radius)
{
	njPushMatrixEx();
	if (pos != 0) {
		njTranslateEx(pos);
	}
	njScale(0, radius, radius, radius);
	if (color == 0) {
		GXColor red;
		red.r = sDebugRed.r;
		red.g = sDebugRed.g;
		red.b = sDebugRed.b;
		red.a = sDebugRed.a;
		color       = &red;
	}
	debugDrawSetup(color);
	GXDrawSphere1(1);
	njPopMatrixEx();
}

// 0x80011400
// Upright The colour defaults to green
static const GXColor sDebugGreen = { 0x20, 0xFF, 0x20, 0x80 };

extern "C" void debugCylinder(const Vec3f* pos, GXColor* color, f32 radius, f32 height)
{
	njPushMatrixEx();
	if (pos != 0) {
		njTranslateEx(pos);
	}
	njScale(0, radius, height, radius);
	njRotateX(0, -0x4000);
	if (color == 0) {
		GXColor green;
		green.r = sDebugGreen.r;
		green.g = sDebugGreen.g;
		green.b = sDebugGreen.b;
		green.a = sDebugGreen.a;
		color         = &green;
	}
	debugDrawSetup(color);
	GXDrawCylinder(8);
	njPopMatrixEx();
}

// 0x800114BC
// size is the full size
static const GXColor sDebugBlue = { 0x20, 0x20, 0xFF, 0x80 };

extern "C" void debugCube(const Vec3f* pos, const Vec3f* size, GXColor* color)
{
	njPushMatrixEx();
	if (pos != 0) {
		njTranslateEx(pos);
	}
	if (size != 0) {
		njScale(0, 0.8660251f * size->x, 0.8660251f * size->y, 0.8660251f * size->z);
	}
	if (color == 0) {
		GXColor blue;
		blue.r = sDebugBlue.r;
		blue.g = sDebugBlue.g;
		blue.b = sDebugBlue.b;
		blue.a = sDebugBlue.a;
		color        = &blue;
	}
	debugDrawSetup(color);
	GXDrawCube();
	njPopMatrixEx();
}

#define GX_FIFO_F32 (*(volatile f32*)0xCC008000)

static const GXColor sDebugWhite = { 0xFF, 0xFF, 0xFF, 0xFF }; // 0x802C6410

// 0x8001156C
// The colour defaults to white
extern "C" void debugLine(const Vec3f* from, const Vec3f* to, GXColor* color)
{
	if (color == 0) {
		color = (GXColor*)&sDebugWhite;
	}
	debugDrawSetup(color);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
	GXSetChanCtrl(GX_ALPHA0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetLineWidth(12, GX_TO_ZERO);
	GXBegin(GX_LINESTRIP, GX_VTXFMT0, 2);
	GXPosition3f32(from->x, from->y, from->z);
	GXPosition3f32(to->x, to->y, to->z);
}

// 0x8001166C
// A line from from along dir
extern "C" void debugArrow(const Vec3f* from, Vec3f dir, GXColor* color)
{
	Vec3f to;
	to.set(from->x, from->y, from->z);
	Vec3f d;
	d.set(dir.x, dir.y, dir.z);
	njAddVector(&to, &d);
	debugLine(from, &to, color);
}
