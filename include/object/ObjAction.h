#ifndef _OBJACTION_H
#define _OBJACTION_H
struct HeapBlock;
struct TObject;
struct ObjHeap;
struct ObjInitPair;
#include "Dolphin/GX/GXTypes.h"

#include "core/types.h"
#include "stage/Stage.h"

class SetDesc;
struct ObjMsg;
struct ObjIdEntry;

/** A node of a circular doubly linked list */
struct ObjLink {
	ObjLink* mPrev; // _00
	ObjLink* mNext; // _04
};

/** A list head starts empty linked to itself */
struct ObjBucket : public ObjLink {
	ObjBucket(); // 0x80010004
};

/** The references to one object, kept in its id table entry. @size 0xC */
struct ObjRefList {
	ObjRefList();  // 0x80010674
	~ObjRefList(); // 0x800106A4
	void clear();  // 0x80010748 empties every reference on it

	ObjBucket mLink;    // _00
	ObjIdEntry* mOwner; // _08
};

/**
 * A reference to another object by id, which goes empty when that object does. The name is ours.
 *
 * @size 0xC
 */
class ObjRef {
public:
	ObjRef();
	~ObjRef();

	class TObjAction* get();
	u16 getId();
	ObjRef& setId(u16 id);
	ObjRef& assign(const ObjRef& other);
	void link(u16 id);
	void unlink();

	ObjLink mLink;     // _00 on mList
	ObjRefList* mList; // _08 the target's list of references
};

/** A set of numbered bits */
struct BitFlags {
	BitFlags();
	void orWith(const BitFlags& other);
	void operator=(const BitFlags& other);
	BitFlags* setBit(int bit);
	void clear();
	int testBit(int bit);

	u32 mBits;
};

/**
 * What onHit fills in. TObjEnemy holds one in its hit record, and EggDamage in EggLevel.cpp is the same thing.
 *
 * @size 0x18
 */
struct HitInfo {
	HitInfo();
	HitInfo& operator=(const HitInfo& o)
	{
		mDamage = o.mDamage;
		mFlags  = o.mFlags;
		mPos    = o.mPos;
		_14     = o._14;
		return *this;
	}

	int mDamage;     // _00
	BitFlags mFlags; // _04
	Vec3f mPos;      // _08 set with flag 0xC
	u32 _14;         // _14
};

extern "C" void resetHitInfo(HitInfo* hit);

/**
 * The base of everything placed in or spawned into a stage. The name is the one its constructor stores.
 * Slot names come from what the subclasses do with them. A plain vfn_ means only the base's empty body is read.
 *
 * @size 0x30
 */
class TObjAction : public TObject {
public:
	TObjAction(TObject* parent);
	virtual ~TObjAction();

	virtual Vec3f* getPos();                                          // _20
	virtual void getNodeMatrix(int node, f32* mtx);                   // _24 where something attached to this goes
	virtual int getRotY();                                            // _28
	virtual int getKind();                                            // _2C picks the id range
	virtual void initFromDesc(SetDesc* desc);                         // _30
	virtual void debugPrint(void* arg);                               // _34 a candidate eggs and TObjStageObj print debug text here
	virtual void readEnemyDesc(SetDesc* desc);                        // _38 only TObjEnemy has been seen to use it
	virtual void vfn_3C();                                            // _3C
	virtual f32 vfn_40();                                             // _40
	virtual int canAct();                                             // _44
	virtual int vfn_48(TObjAction* other);                            // _48
	virtual int sendToHandle(ObjMsg* msg);                            // _4C
	virtual int vfn_50();                                             // _50
	virtual f32 getRadiusV();                                         // _54
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58
	virtual void enterAction(int action);                             // _5C
	virtual void exitAction(int action);                              // _60
	virtual void vfn_64(int action);                                  // _64
	virtual void vfn_68(int action);                                  // _68

	u16 assignId(u32 reserve);
	u16 registerId();
	u16 unregisterId();
	void setAction(int action);

	u16 mObjId;      // _20 0xFFFF until one is assigned
	u16 mHolderId;   // _22
	s8 mPlayer;      // _24
	u8 _25;          // _25
	s16 mAction;     // _26 -1 = none
	s16 mPrevAction; // _28
	u8 _2A[2];       // _2A
	void* mIdEntry;  // _2C the object's entry in the id table
};

class TObjEgg;

/** Walks every object with an id. Its code is next to ObjAction.cpp's. The name is ours. @size 0x8 */
struct ObjIdIter {
	ObjIdIter();              // 0x80010774
	ObjIdIter(ObjLink* head); // 0x80010778

	void begin();      // 0x80010780
	void next();       // 0x80010790
	BOOL atEnd();      // 0x800107A0
	TObjAction* get(); // 0x800107B8
	TObjEgg* getEgg(); // 0x800107C8

	ObjLink* mHead; // _00 the id table's list of all entries
	ObjLink* mCur;  // _04
};

extern "C" {
void objRefAttach(ObjRefList* list, ObjRef* ref); // 0x800106E0
void objRefDetach(ObjRefList* list, ObjRef* ref); // 0x80010714
}

struct Vec3f;

extern "C" {
int angleApproachDiff(int, int, int, int*);
int angleDiff(int, int);
int angleDist(int, int);
void blendModeAdd();
void blendModeAlpha();
void blendModeInvert();
void blendModeInvertAlpha();
void endKColor();
void initObjIds();
void loadScreenMatrix(f32, int);
f32 normalizeVector(Vec3f*);
void* objAlloc(u32);
void objFree(void*);
void objHeapFree(void*, void*);
ObjIdIter objIdIter();
void popCullMode();
void popGXCullMode();
void pushGXCullMode(int);
f32 quadDiscriminant(f32, f32, f32);
f32 quadRoot(f32, f32, f32);
void shutdownObjIds();
f32 turnRadius(f32, int);
void vectorToAngles(const Vec3f*, int*, int*);
}

extern "C" {
void getCullMode(int*);
void setCullMode(int);
TObjAction* findObjById(u16);
void setObjId(TObjAction*, u16);
void releaseObjId(TObjAction*);
}

extern "C" {
int setRangeCheck(int);
void debugLine(const Vec3f*, const Vec3f*, GXColor*);
void setRangeCenter(const Vec3f*);
void debugArrow(const Vec3f*, Vec3f, GXColor*);
}

extern "C" {
int angleApproach(int, int, int);
ObjHeap* objHeapDestroy(ObjHeap*, s16);
ObjHeap* objHeapInit(ObjHeap*, u32, u32);
int sendMsgById(int, ObjMsg*);
#define sendMsgByIdU16(id, msg) (((int (*)(u16, ObjMsg*))sendMsgById)(id, msg))
int isInRange(void*, const Vec3f*, f32);
void runInits(const ObjInitPair*, int);
void runTerms(const ObjInitPair*, int);
void beginKColor(GXColor*);
void debugSphere(const Vec3f*, GXColor*, f32);
void debugCylinder(const Vec3f*, GXColor*, f32, f32);
void debugCube(const Vec3f*, const Vec3f*, GXColor*);
}

extern "C" {
void pushCullMode(int);
void quadRoots(f32, f32, f32, f32*, f32*);
int objDebugEnabled(TObject*);
}

// First fit with free blocks kept in address order and merged on free
struct HeapBlock {
	HeapBlock* mNext;
	u32 mSize;
};

struct ObjHeap {
	HeapBlock* mHead; // _00 a sentinel of size 0 at the start of the memory
	u32 _04;
	u32 mAlign; // _08 a power of two
};

/** One class's setup and teardown as SetManager and Stage list them */
struct ObjInitPair {
	void (*mInit)(); // _00
	void (*mTerm)(); // _04
};

extern "C" {
extern void* gObjTable;
// The next free id of each kind from 0 0x1000 wrapping at 0x2000 0x2000 0x4000 and 0x8000
extern s32 gNextObjId0;
extern s32 gNextObjId1;
extern s32 gNextObjId2;
extern s32 gNextObjId3;
extern s32 gNextObjId4;
extern int gRangeCheck;
extern Vec3f gRangeCenter;
}

#endif
