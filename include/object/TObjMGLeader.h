#ifndef _OBJECT_TOBJMGLEADER_H_
#define _OBJECT_TOBJMGLEADER_H_
#include "camera/TCamera.h"
#include "egg/ObjEgg.h"
#include "engine/DrawTask.h"
#include "engine/MotionPlayer.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
#include "ui/TSimpleText.h"
struct SetDesc;
struct TAeObjGameOver;
struct TObjMGLeader;
struct TObject;

#include "core/types.h"

extern "C" {
void initAeGameOver();
void initMGLeader();
int loadAeGameOver();
void loadMGLeader();
void releaseAeGameOver();
void releaseMGLeader();
int scriptLoadMGLeader(void*);
void termAeGameOver();
}

extern "C" {
TObject* createTObjMGLeader(TObject*, SetDesc*);
TAeObjGameOver* createAeGameOver();
}

class TObjMGLeader : public TSetActor {
public:
	TObjMGLeader(TObject* parent, SetDesc* desc); // 0x80112B7C
	virtual ~TObjMGLeader();                      // 0x80112C3C

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                       // _0C 0x80113120
	virtual void vfn_10();                     // _10 0x801136B4
	virtual void render();                     // _1C 0x80113764
	virtual void initFromDesc(SetDesc* desc);  // _30 0x80112A84
	virtual void debugPrint(void*);            // _34 0x80112AD8
	virtual int sendToHandle(ObjMsg* msg);     // _4C 0x801134FC
	virtual void reset();                      // _74 0x80112CE4

	void makeShape();          // 0x80112DF4
	void makeCylinder();       // 0x80112E50
	void moveShape();          // 0x80112EA8
	void freeShape();          // 0x80112F00
	void calcMatrix();         // 0x80112F38
	void calcShadow();         // 0x80112F7C
	void setShadow(EggHits*);  // 0x8011302C
	void startConversation();  // 0x80113050
	void endConversation();    // 0x801130C8
	void startMode();          // 0x801130EC
	void think();              // 0x801131DC
	int setAction(int action); // 0x801131FC
	void startIdle();          // 0x801132DC
	void updateAction();       // 0x80113360
	void actWait();            // 0x801133CC
	void actTurn();            // 0x801133D0
	void actTalk();            // 0x8011346C

	DrawTask* mDraw;        // _64
	ObjModel mObj;          // _68
	MotionPlayer mMotion;   // _88
	ObjShadow mShadow;      // _C4
	f32 mMtx[12];           // _FC
	ObjRef mTalker;         // _12C
	Vec3f mStart;           // _138
	int mAim;               // _144 the angle it turns to
	u16 mEventId;           // _148
	int mReady;             // _14C takes talk
	TSimpleText* mText;     // _150
	int mMsgId;             // _154
	TCamConversation* mCam; // _158
	int mHold;              // _15C never set non-zero stops it acting
	int mAction;            // _160
	int _164;               // _164
	s16 mTimer;             // _168
	s16 mIdleTime;          // _16A
};

extern "C" {
extern const char* gTObjMGLeaderName;
}

#endif
