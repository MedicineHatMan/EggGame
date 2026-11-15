#ifndef _STAGE_H
#define _STAGE_H
#include "Ninja/ninja.h"
struct StageCamera;

#include "core/types.h"
#include "io/nt_arc.h"

// Only TModeAllocator is an original name from an OSPanic string in ModeCtrl.cpp

/**
 * Per-world files, shared by every stage of that world.
 * @size 0x1C
 */
struct StageResources {
	const char* mSoundBank;   // _00 bank04*.mpb
	const char* mCommonBank;  // _04 bank06.mpb not read by TStage::load
	const char* mWorldBank;   // _08 bank07_*.mpb can be null
	const char* mParticles;   // _0C ptcl2_*.bin
	const char* mObjMotion;   // _10 pa_*.pad can be null
	const char* mStageObjArc; // _14 stgobj_*.arc
	const char* mGeometryArc; // _18 geobj_*.arc
};

struct Vec3f {
	inline void set(f32 newX, f32 newY, f32 newZ)
	{
		x = newX;
		y = newY;
		z = newZ;
	}

	f32 x, y, z;
};

/**
 * A quaternion. The real part comes first, as in the Ninja library. Plain assignment copies v as words and w as a float.
 *
 * @size 0x10
 */
struct Quat {
	f32 w;   // _00
	Vec3f v; // _04
};

/**
 * @size 0x10
 */
struct StageSpawn {
	Vec3f mPos; // _00
	f32 mAngle; // _0C degrees
};

/**
 * One entry of ge_stagedef.bin. File names are relocated pointers, and null when the stage has no such file.
 * @size 0x90
 */
struct StageDef {
	const char* mName;         // _00 "blue1" "red_boss" "battle_green" "title"
	StageResources* mRes;      // _04 null only for "practice"
	const char* mTheme;        // _08 picks the TStage subclass
	const char* mMission;      // _0C see sMissionTypes Null for battle stages
	const char* mLandscape;    // _10 stg_*.lnd
	const char* mBsp;          // _14 stg_*.bsp not read by TStage::load and not on the disc
	const char* mCollision;    // _18 stg_*.mc2
	const char* mSetObj;       // _1C set_obj_*.bin
	const char* mSetEnemy;     // _20 set_ene_*.bin
	const char* mSetCamera;    // _24 set_cam_*.bin
	const char* mSetDesign;    // _28 set_design_*.bin
	const char* mPath;         // _2C path_*.pth
	const char* mEventCamArc;  // _30 ar_event_cam_*.arc
	const char* mMessages;     // _34 mes_*_hiyoko.bin gets a language suffix when opened
	const char* mEvent;        // _38 event_*.arc
	StageSpawn mSpawns[4];     // _3C TModeStage uses the first battle mode all four
	int mTimeLimit;            // _7C 0 to 1000 across stages maybe seconds
	int mRankScores[4];        // _80 descending green1 has 7000 5500 4500 2500
};

/** The root of ge_stagedef.bin 88 entries on USA retail */
struct StageDefTable {
	u16 mCount;         // _00
	StageDef* mEntries; // _04
};

// Files from the shared file cache
#include "io/ResFile.h"

struct ArcFile { // size 0x70
	u8 _00[0x4C];
	void* mData; // _4C
};

struct PortalVis; // PortalVis.h

struct LightGroup;

/** What getSceneRecord returns. @size 0x10 */
struct SceneRecord {
	SceneRecord();
	int mCamera;      // _00
	PortalVis* mVis;  // _04
	void* _08;
	LightGroup* mLights; // _0C
};

#include "engine/Light.h" // needs Vec3f

// Only here for the null check its inline destructor adds
/**
 * A scene's lights: the sun, the ambient colour, and the set to give to a group. SceneLights.cpp, 0x8004D3C8.
 * The name is ours. @size 0xE8
 */
struct SceneLights {
	SceneLights();

	DirLight mSun;         // _00 flag 2 diffuse
	AmbientLight mAmbient; // _70
	LightSet mSet;         // _D4 holds both
};

extern "C" {
void sceneLightsSetSunDir(SceneLights* scene, const Vec3f* dir);    // 0x8004D428
void sceneLightsSetSunColor(SceneLights* scene, u32* color);  // 0x8004D444
void sceneLightsSetAmbient(SceneLights* scene, u32* color);   // 0x8004D450
}

/**
 * The world itself: owns everything loaded from a StageDef. There is one subclass per theme.
 * @size 0x30, and subclasses grow from there
 */
class TStage {
public:
	TStage();

	void load(const char* name);
	void unload();
	void start(int first);
	void calc();
	void drawScene();
	void drawEffects(int withExtras);
	void drawObjects();

	SceneRecord* _00;            // _00
	SceneLights* mLights; // _04
	u8 _08[0x4];           // _08
	ChunkFile* mCollision; // _0C
	void* _10;             // _10
	ChunkFile* mPath;      // _14
	TResource* _18;            // _18 cleared and released but nothing here sets it
	BinFile* mMessages;    // _1C
	PortalVis* mVis;       // _20
	LightGroup* _24;       // _24
	const char* mName;     // _28

	// _2C = vtable
	virtual ~TStage();         // _08
	virtual void loadAssets(); // _0C the default is empty 0x80014DD0 in TModeTitle.cpp
	virtual void freeAssets(); // _10 empty by default 0x80014E60
	virtual void vfn_14();    // _14 empty by default 0x80015054
	virtual void run(u32 flags); // _18 1 = calc 2 = draw
};

/**
 * The stage for themes with no subclass of their own, like "green".
 * @size 0x30
 */
class TStageDefault : public TStage {
public:
	TStageDefault();
	virtual ~TStageDefault();
};

extern "C" void* objHeapAlloc(void*, u32);
extern "C" void objHeapFree(void*, void*);
extern "C" void* gObjHeap;

/** Base of the game's objects */
class TObject {
public:
	TObject(TObject* parent);

	static void* operator new(unsigned long size) { return objHeapAlloc(gObjHeap, size); }
	static void* operator new(unsigned long, void* mem) { return mem; }
	static void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

u32 _00;      // _00
	u16 mFlags;   // _04 bit 0 = delete me
	u16 _06;      // _06
	TObject* mPrev;   // _08
	TObject* mNext;   // _0C
	TObject* mParent; // _10
	TObject* mChild;  // _14

	// _18 = vtable
	virtual ~TObject();       // _08
	virtual void draw();   // _0C
	virtual void vfn_10(); // _10
	virtual void vfn_14(); // _14
	virtual void vfn_18(TObject* obj); // _18
	virtual void render(); // _1C

	u16 _1C;   // _1C
	u16 mSize; // _1E
};

struct ObjMsg;
class TGameState;
class TGameContext;

/**
 * Holds the current game state and calls its exit and enter when it changes. The name is ours.
 *
 * @size 0x4
 */
struct GameStateSlot {
	GameStateSlot();  // 0x80042AFC
	~GameStateSlot(); // 0x80042B08

	void update(TGameContext* context);                                 // 0x80042B44
	TGameState* change(TGameContext* context, TGameState* next);        // 0x80042B7C returns the old state

	TGameState* mState; // _00
};

/** Eight flag bits. Its constructor is out of line. The name is ours. @size 0x1 */
struct FlagByte {
	FlagByte();

	u8 mBits; // _00 2 = high level 8 = busy 0x10 = left home 0x20 = hidden 0x40 = player state done
};

// Compiled with sym off the DOL keeps its destructor in TGameState.cpp's main text and its two other inlines in
// whatever section was set last
#pragma push
#pragma sym off
/** Base of the mission rules */
class TCustomGameContext : public TObject {
public:
	TCustomGameContext(TObject* parent); // 0x8019F7C0
	virtual ~TCustomGameContext() { }    // 0x80042310

	virtual int onMessage(ObjMsg* msg) { return 0; } // _20 0x80042A84 1 = handled
	virtual int getStatus() { return 0; }            // _24 0x80042A8C
};
#pragma pop

/**
 * The default mission rules. The name comes from the string at sGameContextName.
 *
 * @size 0x38
 */
class TGameContext : public TCustomGameContext {
public:
	TGameContext(TObject* parent); // 0x80042278
	virtual ~TGameContext();    // 0x8004236C

	virtual void draw();                    // _0C 0x800426EC
	virtual int onMessage(ObjMsg* msg);     // _20 0x800426A0 passed to the state
	virtual int getStatus();                // _24 0x800424D4
	virtual void startState() { }           // _28 0x80042A4C
	virtual void start();                   // _2C 0x800423F8
	virtual void respawn();                 // _30 0x80042438

	int isLastLife();                       // 0x80042490
	void finish();                          // 0x800424A4
	void setCleared();                      // 0x800424B4
	void setWarped();                       // 0x800424C4
	int handleMessage(ObjMsg* msg);         // 0x80042520
	void sendLives();                       // 0x8004265C

	FlagByte mEndFlags;        // _20 4 = finished 2 and 8 pick the status
	u8 mSlot;                  // _21
	u8 _22[0x2];               // _22
	int mLives;                // _24
	void* mPool;               // _28 where the states come from
	GameStateSlot mStateSlot;  // _2C
	u32 mTime;                 // _30 milliseconds
	u32 mTimeLimit;            // _34 where the clock stops
};

class TIntro : public TObject {
public:
	u8 _20[0x30]; // _20
	int mDone;    // _50
};

class TPauseMenu : public TObject {
public:
	u8 _20[0x38];  // _20
	int mChoice;   // _58
	u8 _5C[0x4];   // _5C
	int mNoRetry;  // _60
	int _64;       // _64
	u8 _68[0x8];   // _68
	int mEggCount; // _70
	int mClosed;   // _74
};

/**
 * Time on the stage clock.
 * @size 0x6
 */
struct StageTime {
	inline u32 toMillis() const
	{
		return mMilli + mSecond * 1000 + mMinute * 60 * 1000 + mHour * 60 * 60 * 1000;
	}

	u8 mHour;   // _00
	u8 mMinute; // _01
	u8 mSecond; // _02
	u16 mMilli; // _04
};

/**
 * What the player did on a stage, reset by stageRecordClear.
 * @size 0x20
 */
struct StageRecord {
	inline StageRecord& operator=(const StageRecord& other)
	{
		mScore[0] = other.mScore[0];
		mScore[1] = other.mScore[1];
		mScore[2] = other.mScore[2];
		mScore[3] = other.mScore[3];
		mCount[0] = other.mCount[0];
		mCount[1] = other.mCount[1];
		mCount[2] = other.mCount[2];
		mCount[3] = other.mCount[3];
		mCount[4] = other.mCount[4];
		mCount[5] = other.mCount[5];
		mCount[6] = other.mCount[6];
		return *this;
	}

	int mScore[4]; // _00 the four scores the results screen counts up
	s16 mCount[7]; // _10 the counts the results screen shows beside them
};

class TMode;

// Two fixed slots A free slot's next pointer sits at +4 which is TMode::_04 once a mode is constructed there
class TResult;

class TModeAllocator {
public:
	TModeAllocator(u32 slotSize);
	void* alloc(u32 size);
	void free(void* block);

	u32 mSlotSize; // _00
	void* mBuffer; // _04 two slots of mSlotSize
	void* mFree;   // _08
};

inline void* operator new(unsigned long size, TModeAllocator* allocator)
{
	return allocator->alloc(size);
}

// 0 loadFiles has been called waiting on the disc
enum ModePhase {
	MODE_PHASE_LOAD = 0,
	MODE_PHASE_INIT = 1,
	MODE_PHASE_RUN  = 2,
};

// The active mode and how far its loading has got
class TModeCtrl {
public:
	TModeCtrl();

	TMode* setMode(TMode* mode);
	int getState();
	void release();
	void update();

	TMode* mMode; // _00
	int mPhase;   // _04 a ModePhase Starts at MODE_PHASE_RUN so the first update does nothing
};

class TMode {
public:
	inline TMode();

	void loadPrd(const char* name);

	// _00 vtable TMode's at 0x802523B0 starts with 8 zero bytes so the slot offsets below count from its start
	inline void operator delete(void*) { } // modes live in TModeAllocator's buffer

	virtual ~TMode() { }      // _08 inline no mode calls it
	virtual void loadFiles(); // _0C
	virtual void freeFiles(); // _10
	virtual int init();       // _14
	virtual int shutdown();   // _18
	virtual void update();    // _1C

	u32 _04; // _04 the free-list link until the constructor runs
	int _08; // _08 TModeCtrl::getState returns this 8 means none
};

inline TMode::TMode()
{
	_08 = 8;
}

/**
 * The game mode that plays one stage.
 * @size 0xA4
 */
class TModeStage : public TMode {
public:
	TModeStage();

	static TModeStage* create(TModeAllocator* allocator);

	virtual ~TModeStage() { }
	virtual void loadFiles();
	virtual int init();
	virtual int shutdown();
	virtual void update();

	void endFrame();
	void startStage(int first);
	void beginPlay();
	void restart();
	void stopPlay();
	int updateRecord(int* outNewBest);
	void updateUnlocks();
	void runStage();
	void runStagePaused();
	void runState(int state, int phase);
	void setState(int state);
	void setCameraActive(int active);
	void freePlayer();
	void setTime(const StageTime& time);
	void setRecord(const StageRecord& record);
	void stateLoad(int phase);
	void stateIntro(int phase);
	void stateWait(int phase);
	void stateExit(int phase);
	void statePlay(int phase);
	void stateResult(int phase);
	void statePause(int phase);
	void stateMenu(int phase);

	s8 mState;         // _0C 8 = none
	u8 mPlayerNo;      // _0D
	u8 _0E;            // _0E lives to start with
	u8 _0F;            // _0F
	u16 mTimer;        // _10
	u8 _12[0x2];       // _12
	int mMissionType;  // _14
	TStage* mStage;    // _18
	const char* mName; // _1C
	int mSkipDraw;     // _20 drops every other draw when the frame ran late
	int _24;           // _24
	f32 mStartX;       // _28
	f32 mStartY;       // _2C
	f32 mStartZ;       // _30
	int mStartAngle;   // _34 Ninja angle
	TPauseMenu* mPauseMenu;   // _38
	TIntro* mIntro;           // _3C
	TResult* mResult;         // _40
	TGameContext* mContext;       // _44
	StageTime mStartTime;     // _48
	StageTime mSavedTime;     // _4E
	StageRecord mRecord;      // _54 a plain struct the ctor resets it last with stageRecordClear
	StageRecord mSavedRecord; // _74
	int mStageIndex;          // _94
	u16 _98;                  // _98
	int _9C;           // _9C
	int _A0;           // _A0
};

enum StatePhase {
	STATE_ENTER  = 0,
	STATE_EXIT   = 1,
	STATE_DRAW   = 2,
	STATE_UPDATE = 3,
};

void setCurrentStage(TStage* stage);
TStage* getCurrentStage();
void getSpawnPos(Vec3f* out, const TStage* stage, int index);
void updateLight();
TStage* createDefaultStage();

void loadStageDefs();
StageDef* findStageDef(const char* name);
StageDefTable* getStageDefTable();

/**
 * One mounted .prd file: a U8 archive, loaded and PRS-decompressed by the async file reader.
 * Each pool is a linked list starting with an empty sentinel.
 * @size 0x4C
 */
struct PrdFile {
	PrdFile(PrdFile* list);
	~PrdFile();

	int isNamed(const char* name);
	void addRef();
	int startLoad(const char* name);
	void* find(const char* path, u32* outSize);

	BinFile* mFile;  // _00
	ARCHandle mHandle; // _04
	s16 mState;      // _20 0 = empty 1 = ready 2 = loading
	s16 mRefCount;   // _22
	char mName[32];  // _24
	PrdFile* mPrev;  // _44
	PrdFile* mNext;  // _48
};

enum PrdPool {
	PRD_POOL_BOOT = 0, // k_boot.prd
	PRD_POOL_MODE = 1, // everything else freed by TMode::freeFiles
	PRD_POOL_COUNT
};

void initPrdPools();
PrdFile* getPrdPool(int pool);
u8 loadPrd(int pool, const char* name);
u8 loadBootPrd(const char* name);
u8 loadModePrd(const char* name);
void freePrdPool(int pool);
void freeModePrds();
void* findInPrds(const char* path, u32* outSize);

class ChunkFile;
class TArc;

extern "C" {
void stageLoadBossConsole(TStage*);
void stageCamLockAll();
extern TStage* sCurrentStage;
extern TArc* sPlayerMotionArc;
extern TArc* sRegularObjMotionArc;
extern ChunkFile* sStageDefFile;
extern int sStageFirstLoad;
extern int sStageLoadCount;
extern TArc* sStageObjMotionArc;
}

extern "C" {
extern void* sStageClassFuncs[0x48];
}

extern "C" {
void stageFreeBossConsole();
}

struct StageCamera {
	u8 _00[0x44];
	int mFov;  // _44
	u8 _48[0x8];
	f32 mNear; // _50
	f32 mFar;  // _54
};

#endif
