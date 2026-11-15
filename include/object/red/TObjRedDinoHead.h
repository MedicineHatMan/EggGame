#ifndef _TOBJREDDINOHEAD_H
#define _TOBJREDDINOHEAD_H
#include "core/types.h"
struct RedHeadList;
class TParticleGenerator;
struct SetDesc;
struct TObject;

#include "stage/SetActor.h"
#include "object/ObjModel.h"
#include "engine/MotionPlayer.h"
#include "engine/DrawTask.h"
#include "enemy/TEneAm01Parts.h"
#include "engine/TMobileObject.h"
#include "object/ObjArchive.h"

extern "C" {
void objHeapFree(void*, void*);
extern void* gObjHeap;
}

/**
 * One node of the live-head list at gRedHeadList. The owner is the head.
 * @size 0x10
 */
struct RedHeadLink {
	RedHeadLink()
	{
		mNext = 0;
		mPrev = 0;
		mList = 0;
	}

	RedHeadLink* mNext; // _00 toward the newer node
	RedHeadLink* mPrev; // _04
	void* mOwner;       // _08
	void* mList;        // _0C
};

/**
 * A red-world dino head. Hitting it spends mLife, plays the death motion and sets mSwitchId.
 * Areas with the same mGroupId call open and close; the first open shows the side particles,
 * enables the shape and plays a sound. Out of range, draw marks the object for deletion.
 *
 * @size 0x194
 */
class TObjRedDinoHead : public TSetActor {
public:
	TObjRedDinoHead(TObject* parent, SetDesc* desc);
	virtual ~TObjRedDinoHead();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void*);           // _34
	virtual void setup();                     // _74

	// open counts up and close counts down
	void open();
	void close();

	void stateAlive();
	void stateDying();
	void stateDead();
	void setAct(int act);
	int switchIsClear();
	void notifyAreas();
	void setupColli();
	void setupShape();
	void updateShapePos();
	void freeShape();
	void buildMatrix();
	void pose();
	int drawNodes(void* node, int index);

	RedHeadLink mLink;              // _64
	DrawTask* mDrawTask;            // _74
	ObjModel mModel;                // _78
	void* mGeo;                     // _98 getGeObjModels()[8]
	ObjDrawState mDrawState;        // _9C
	MotionPlayer mMotion;           // _C0
	f32 mMatrix[12];                // _FC
	f32 mScale;                     // _12C 1 + the record's first float
	EnemyTint mTint;                // _130
	MobileLoopEffect mLoop;         // _138 sound 0x21C while the head is alive
	Vec3f mFxPos;                   // _164 scaled node centre yawed and added to mPos
	s16 mGroupId;                   // _170 record int 0
	u16 mSwitchId;                  // _172 record int 3
	s16 mLife;                      // _174 record int 1
	u32 mHitFlags;                  // _178 bit 0 = the hit flash is running
	u32 mOpen;                      // _17C non-zero while at least one area holds it open
	TParticleGenerator* mPartL;     // _180 particle id 8 yaw + 0x4000
	TParticleGenerator* mPartR;     // _184 particle id 8 yaw - 0x4000
	TParticleGenerator* mPartFront; // _188 particle id 10 at mFxPos
	int mOpenCount;                 // _18C
	int mAct;                       // _190 0 = up 1 = dying 2 = down
};

extern "C" {
void dinoHeadSetExit();
void dinoHeadSetInit();
}

extern "C" {
TObject* createTObjRedDinoHead(TObject*, SetDesc*);
}

// The live heads built by the file's __sinit
struct RedHeadList {
	RedHeadList();  // 0x800C6668
	~RedHeadList(); // 0x800C667C

	RedHeadLink* mHead; // _00
	RedHeadLink* mTail; // _04
	int mCount;         // _08
};

// 0x802803D8
// built right after the list
struct RedHeadSlots {
	RedHeadSlots()
	    : _00(0)
	    , _20(0)
	{
	}

	int _00;      // _00
	u8 _04[0x1C]; // _04
	int _20;      // _20
};

#include "stage/Stage.h"
extern "C" {
extern const char* gTObjRedDinoHeadName;
extern const MotionEntry sRedDinoHeadMotions[];
}

extern RedHeadList gRedHeadList;

void initRedDinoHeadSet();
void termRedDinoHeadSet();

#endif
