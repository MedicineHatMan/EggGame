#ifndef _SETMANAGER_H
#define _SETMANAGER_H

#include "core/types.h"
#include "stage/Stage.h"
#include "object/ObjAction.h"

class SetDesc;

struct SetRot {
	inline void set(int newX, int newY, int newZ)
	{
		x = newX;
		y = newY;
		z = newZ;
	}

	int x, y, z;
};

enum SetKind {
	SET_KIND_OBJ    = 0, // set_obj_*.bin
	SET_KIND_ENEMY  = 1, // set_ene_*.bin
	SET_KIND_DESIGN = 2, // set_design_*.bin
	SET_KIND_COUNT
};

/**
 * One object or design record, as stored in the file.
 * @size 0x40
 */
struct SetRecord {
	int mId;        // _00 a class ID 0 ends the list
	Vec3f mPos;     // _04
	int mRot[3];    // _10
	int mInts[4];   // _1C
	f32 mFloats[4]; // _2C
	s16 _3C;        // _3C
	s16 _3E;        // _3E
};

/**
 * One enemy record, as stored in the file.
 * @size 0x50
 */
struct SetEnemyRecord {
	int mId;        // _00 a class ID 0 ends the list
	Vec3f mPos;     // _04
	int mRot[3];    // _10
	s16 _1C;        // _1C
	s16 _1E;        // _1E
	int _20;        // _20
	int _24[3];     // _24
	int mInts[4];   // _30
	f32 mFloats[4]; // _40
};

/**
 * What is known about a record while the stage runs.
 * @size 0x18
 */
struct SetSlot {
	inline SetSlot& operator=(const SetSlot& other)
	{
		mState = other.mState;
		mObjId = other.mObjId;
		mTimer = other.mTimer;
		mObj   = other.mObj;
		mData  = other.mData;
		_10    = other._10;
		mLink  = other.mLink;
		return *this;
	}

	void init();
	void clear();
	void reset();
	void setSpawned(void* obj);
	int canSpawn(int tick);
	int hasNoData();
	void* getData();
	void setData(void* data);
	void setDead(void* obj, int timer);
	void setDeadFree(void* obj);
	void setDeadDone(void* obj);
	void setDeadTimed(void* obj, u16 timer);
	void setDeadForever(void* obj);
	void setBlocked();
	int takeBlocked();
	int takeHidden();
	int takeAny();
	int checkHidden();
	void setFinished();
	void setLinkIndex(int index);
	void killObject();

	int mState; // _00 0 free 1 blocked 2 spawned 3 timed 4 and 5 hidden
	u16 mObjId; // _04 0xFFFF = none
	u16 mTimer; // _06
	void* mObj; // _08
	void* mData; // _0C
	int _10;    // _10
	struct SetLink* mLink; // _14
};

/**
 * Connects a live object to its slot, and tells the slot how the object died.
 * @size 0xC
 */
struct SetLink {
	inline SetLink();
	void attach(const SetDesc* desc);
	void reset();
	void* getData();
	void setData(void* data);
	void onDeath(void* obj);
	inline void setMode(s16 mode) { mMode = mode; }

	int mKind;   // _00 -1 = detached
	s16 mIndex;  // _04
	s16 mMode;   // _06
	u16 mTimer;  // _08
};

/**
 * A SetLink member whose constructor and destructor are not inlined, which is how the egg holds one.
 * @size 0xC
 */
struct SetLinkMember {
	SetLinkMember();
	~SetLinkMember();

	inline SetLink* get() { return (SetLink*)this; }

	int mKind;  // _00
	s16 mIndex; // _04
	s16 mMode;  // _06
	u16 mTimer; // _08
};

/**
 * All the records of one placement file.
 * @size 0x1C
 */
class SetList {
public:
	SetList(int kind, int capacity);
	~SetList();

	int isValid(int index);
	SetSlot* getSlot(int index);
	void attachLink(SetLink* link);

	int mKind;       // _00
	SetSlot* mSlots; // _04
	int mCount;      // _08 up to the first empty record
	int mCapacity;   // _0C
	BinFile* mFile;  // _10

	// _14 = vtable
	virtual void resetAll() = 0;                                  // _08
	virtual void clearAll() = 0;                                  // _0C
	virtual void load(const char* name) = 0;                      // _10
	virtual void prepareAll() = 0;                                // _14
	virtual void* spawnNow(TObject* parent, int index) = 0;          // _18
	virtual void* spawn(TObject* parent, int index, Vec3f* centre, int check) = 0; // _1C
	virtual void remove(int index) = 0;                           // _20
	// getDesc is declared in each subclass SetList's vtable has 7 slots
};

class SetListEnemy : public SetList {
public:
	SetListEnemy(int kind, int capacity);
	~SetListEnemy();

	SetEnemyRecord* getRecord(int index);
	void countRecords();

	virtual void resetAll();
	virtual void clearAll();
	virtual void load(const char* name);
	virtual void prepareAll();
	virtual void* spawnNow(TObject* parent, int index);
	virtual void* spawn(TObject* parent, int index, Vec3f* centre, int check);
	virtual void remove(int index);
	virtual SetSlot* getDesc(int index, SetDesc* desc);

	SetEnemyRecord* mRecords; // _18
};

class SetListObj : public SetList {
public:
	SetListObj(int kind, int capacity);
	~SetListObj();

	void countRecords();

	virtual void resetAll();
	virtual void clearAll();
	virtual void load(const char* name);
	virtual void prepareAll();
	virtual void* spawnNow(TObject* parent, int index);
	virtual void* spawn(TObject* parent, int index, Vec3f* centre, int check);
	virtual void remove(int index);
	virtual SetSlot* getDesc(int index, SetDesc* desc);

	SetRecord* mRecords; // _18
};

/**
 * @size 0x28
 */
struct SetManager {
	SetManager();
	~SetManager();

	void tick();
	void load(int kind, const char* name);
	SetEnemyRecord* getEnemyRecord(int index);
	SetSlot* getSlot(int kind, int index);
	void getCounts(int kind, int* outCount, int* outCapacity);
	TObject* setParent(int kind, TObject* parent);
	void spawnNowObj(void* arg);
	int scan(int kind, Vec3f* centre, int start, int count, int check);
	u32 spawnById(int kindA, int kindB, int id);
	void onDeath(int kind, void* obj, SetLink* link);
	void resetKind(int kind);
	void clearKind(int kind);
	void attachLink(SetLink* link);

	SetList* mLists[3];         // _00 indexed by SetKind
	SetListObj* mObjLists[2];   // _0C kinds 0 and 2 in that order
	SetListEnemy* mEnemy;       // _14
	TObject* mParents[3];         // _18
	int mTick;                  // _24
};

/**
 * Position and velocity. The slots past the destructor are unread. The data comes first so the compiler puts the
 * vtable after it, at _28.
 *
 * @size 0x2C
 */
struct SetTransformBase {
	f32 mRadius; // _00
	Vec3f mPos;  // _04
	Vec3f mVel;  // _10
	Vec3f mForce; // _1C the velocity to add this frame

	SetTransformBase();
	inline virtual ~SetTransformBase() { } // _08
	virtual void vfn_0C();                 // _0C
	virtual void vfn_10(const Vec3f* point, const Vec3f* force); // _10
	virtual void vfn_14();                 // _14
	virtual void vfn_18();                 // _18

	Vec3f mGravity; // _2C
};

/**
 * Adds gravity and rotation. TSetObj holds one at _30, TObjEnemy at _74, and TObjFogCollision at _4C.
 *
 * @size 0x44
 */
struct SetTransform : public SetTransformBase {
	SetTransform();
	inline virtual ~SetTransform() { } // _08
	virtual void vfn_1C(const SetRot* rot); // _1C
	virtual void vfn_20();             // _20

	SetRot mRot; // _38
};

/** Base of the objects that come from a set_obj or set_design record */
class TSetObj : public TObjAction {
public:
	TSetObj(TObject* parent, SetDesc* desc);
	virtual ~TSetObj();

	inline void operator delete(void* mem);

	virtual Vec3f* getPos();                // _20
	virtual int getRotY();                  // _28
	virtual int getKind();                  // _2C
	virtual void initFromDesc(SetDesc* desc);        // _30
	virtual void debugPrint(void* arg);     // _34
	virtual void setPos(const Vec3f* pos);  // _6C
	virtual SetLink* getLink();             // _70
	virtual void readDesc(SetDesc* desc);   // _74
	virtual void vfn_78(SetDesc* desc);     // _78

	void readHome(SetDesc* desc);

	SetTransform mTrans;  // _30
	Vec3f mHome;          // _74
	int _80;              // _80
	SetLinkMember mLink;  // _84
};

/**
 * A view of one record, handed to the class's create function.
 * @size 0x10
 */
class SetDesc {
public:
	SetDesc();

	int isValid();
	int isDataEmpty() const;
	void setKind(int kind);
	int getKind();
	int isSpawned();

	int mKind;   // _00 -1 = none
	s16 mIndex;  // _04
	u16 mObjId;  // _06 0xFFFF = none

	// _08 = vtable
	virtual ~SetDesc();                           // _08
	virtual void clear() = 0;                     // _0C
	virtual void getPos(Vec3f* out) = 0;          // _10
	virtual void getRot(SetRot* out) = 0;            // _14
	virtual int getId() = 0;                      // _18
	virtual int isEmpty() = 0;                    // _1C
	virtual void prepare() = 0;                   // _20
	virtual void* spawnNow(TObject* parent) = 0;     // _24
	virtual void* spawn(TObject* parent, Vec3f* centre, int check) = 0; // _28
	virtual int getInt(int index) = 0;            // _2C
	virtual f32 getFloat(int index) = 0;          // _30
};

class SetDescObj : public SetDesc {
public:
	SetDescObj();
	~SetDescObj();

	void setRecord(SetRecord* record);
	int get3E();
	int get3C();

	virtual void clear();
	virtual void getPos(Vec3f* out);
	virtual void getRot(SetRot* out);
	virtual int getId();
	virtual int isEmpty();
	virtual void prepare();
	virtual void* spawnNow(TObject* parent);
	virtual void* spawn(TObject* parent, Vec3f* centre, int check);
	virtual int getInt(int index);
	virtual f32 getFloat(int index);

	SetRecord* mRecord; // _0C
};

class SetDescEnemy : public SetDesc {
public:
	SetDescEnemy();
	~SetDescEnemy();

	void setRecord(SetEnemyRecord* record);

	virtual void clear();
	virtual void getPos(Vec3f* out);
	virtual void getRot(SetRot* out);
	virtual int getId();
	virtual int isEmpty();
	virtual void prepare();
	virtual void* spawnNow(TObject* parent);
	virtual void* spawn(TObject* parent, Vec3f* centre, int check);
	virtual int getInt(int index);
	virtual f32 getFloat(int index);

	SetEnemyRecord* mRecord; // _0C
};

/**
 * How one class ID is created. The tables live in SetManager.cpp.
 * @size 0x10
 */
struct SetClass {
	int mId;                               // _00
	TObject* (*mCreate)(TObject* parent, SetDesc* desc); // _04
	void (*mPrepare)(SetDesc* desc);       // _08 can be null
	f32 mRangeSq;                          // _0C squared
};

struct SetClassList {
	const SetClass* mClasses; // _00
	int mCount;               // _04
};

struct SetInitPair {
	void (*mInit)(); // _00
	void (*mTerm)(); // _04
};

struct SetInitList {
	const SetInitPair* mPairs; // _00
	int mCount;                // _04
};

const SetClass* findObjClass(int id);

class TObject;
struct Vec3f;

extern "C" {
void attachSetLink(SetLink*);
void clearSets();
void createSetManager();
const SetClass* findSetClass(int, int*);
const SetClass* findSetClassIn(const SetClassList*, int, int*);
void freeSetInits(const SetInitList**);
void freeSetManager();
void freeSetThemes();
void getSetCounts(int, int*, int*);
void getSetDesignCounts(int*, int*);
void getSetEnemyCounts(int*, int*);
SetEnemyRecord* getSetEnemyRecord(int);
SetSlot* getSetEnemySlot(int);
void getSetObjCounts(int*, int*);
int hasSetManager();
void initSetManager();
void loadSetDesignFile(const char*);
void loadSetEnemyFile(const char*);
void loadSetFile(int, const char*);
void loadSetObjFile(const char*);
void loadSetTheme(const char*);
void onSetDeath(int, void*, SetLink*);
void resetSets();
extern const SetClassList* sSetCommonClasses;
extern const SetInitList* sSetCommonInits;
extern SetManager* sSetManagerObj;
extern const SetClassList* sSetThemeClasses;
extern const SetInitList* sSetThemeInits;
int scanSet(int, Vec3f*, int, int, int);
int scanSetDesign(Vec3f*, int, int);
int scanSetEnemy(Vec3f*, int, int);
int scanSetObj(Vec3f*, int, int);
TObject* setSetDesignParent(TObject*);
TObject* setSetEnemyParent(TObject*);
TObject* setSetObjParent(TObject*);
u32 spawnSetById(int);
void spawnSetObjsNow(void*);
void tickSetManager();
}

extern "C" {
SetSlot* getSetSlot(int, int);
}

extern "C" {
extern const f32 gKey00SetRange;
extern const f32 gCannon01SetRange;
extern const f32 gAnchor01SetRange;
extern const f32 gMapFoothold01SetRange;
extern const f32 gStageObjSetRange;
extern const f32 gDoorASetRange;
extern const f32 gChickenBossSetRange;
extern const f32 gPilingSwitchSetRange;
extern const f32 gEggFeedSetRange;
extern const f32 gEggCannonSetRange;
extern const f32 gHiyokoSetRange;
extern const f32 gPurpleIceWallSetRange;
extern const f32 gPurpleIceFloorSetRange;
extern const f32 gOrangePanelDeOpenSetRange;
extern const f32 gYellowSlideDoorSetRange;
extern const f32 gYellowTimberBrickSetRange;
extern const f32 gYellowSandFloorSetRange;
extern const f32 gYellowSandfallSetRange;
extern const f32 gYellowPillarSetRange;
extern const f32 gYellowFogTrapCtrlSetRange;
extern const f32 gYellowFogTrapStatueSetRange;
extern const f32 gLastTorchSetRange;
extern const f32 gLastAnimusSetRange;
extern const f32 gLastCrowLemuresCtrlSetRange;
extern const f32 gYellowGateSetSetRange;
extern const f32 gCoinSetRange;
extern const f32 gLastSealedCrestSetRange;
extern const f32 gGreenRaySetRange;
extern const f32 gLastRippleFieldSetRange;
extern const f32 gBrkScaffoldSetRange;
}

#endif
