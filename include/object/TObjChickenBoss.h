#ifndef _OBJECT_TOBJCHICKENBOSS_H_
#define _OBJECT_TOBJCHICKENBOSS_H_
#include "Ninja/ninja.h"
#include "Ninja/njtypes.h"
#include "camera/TCamera.h"
#include "engine/DrawTask.h"
#include "engine/Fx.h"
#include "engine/MotionPlayer.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
#include "ui/TSimpleText.h"
struct Vec3f;
struct SetDesc;
struct TChickenBossDemo;
struct TObjChickenBoss;
struct TObject;

#include "core/types.h"

struct CamAim;
struct CamLink;

extern "C" {
void camAimAdd(CamAim*, CamLink*);
void camAimClear(CamAim*);
void camAimRemove(CamAim*, CamLink*);
void camAimUpdate(CamAim*);
void camLinkInit(CamLink*, int);
int camLinkIsAbove(const CamLink*, const CamLink*);
int camLinkIsCut(const CamLink*);
int camLinkIsEntered(const CamLink*);
int camLinkIsPicked(const CamLink*);
CamLink* camLinkPickBest(CamLink*);
void camLinkSetCut(CamLink*);
void chickenBossTev(int);
void chickenBossTexGen(u8);
void chickenBossTexNum(int, int, int, int);
void initChickenBoss();
void termChickenBoss();
}

extern "C" {
TChickenBossDemo* createChickenBossDemo(TObject*, int);
TObject* createTObjChickenBoss(TObject*, SetDesc*);
}

extern "C" {
TObjChickenBoss* chickenBossHatch(const Vec3f*);
}

class TObjChickenBoss : public TSetActor {
public:
	TObjChickenBoss(TObject* parent, SetDesc* desc);
	TObjChickenBoss(TObject* parent, const Vec3f* pos);
	virtual ~TObjChickenBoss();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual int sendToHandle(ObjMsg* msg);
	virtual void reset(); // _74

	void start();
	void startHatch();
	void makeShape();
	void calcMatrix();
	void calcNodes();
	void calcGolden();
	int setState(int state);
	void stateHatch();
	void stateRise();
	void stateDaybreak();
	void stateScript();
	void stateIdle(); // 7 solid only while no egg ring is held
	void state4();
	void state5();
	void state6();
	void stateTalk();
	void stateTurn();
	void stateGreet();
	void drawGolden();

	int mFromSet;                    // _64 1 when a set record placed it
	NJS_OBJECT** mGoldenModels;      // _68 the golden egg arc's "models"
	NJS_TEXLIST** mGoldenTexLists;   // _6C
	NJS_OBJECT* mModel;              // _70
	NJS_TEXLIST* mTexList;           // _74
	NJS_OBJECT* mGoldenModel;        // _78
	ObjModel mObj;                   // _7C
	MotionPlayer mMotion;            // _9C
	DrawTask* mDraw;                 // _D8
	ObjShadow mShadow;               // _DC
	f32 mMatrix[12];                 // _114
	int mFrameCount;                 // _144
	int mParam[4];                   // _148 the set record's ints
	f32 mValue[4];                   // _158
	int mState;                      // _168 ChickenBossState
	TCamConversation* mConversation; // _16C
	TSimpleText* mText;              // _170
	ObjRef mTalker;                  // _174
	int mFaceAngle;                  // _180
	u32 mFlags;                      // _184 ChickenBossFlag
	TCamPath* mCamPath;              // _188
	f32 mNodeMtx[3][12];             // _18C nodes 0 8 and 38
	int _21C;                        // _21C
	f32 mGoldenFrame;                // _220
	f32 mGoldenMtx[12];              // _224
	f32 mGoldenScale;                // _254
	Fx* mFx;                         // _258
	f32 mHomeMtx[12];                // _25C
	f32 _28C;                        // _28C
	TChickenBossDemo* mDemo;         // _290
	u32 mSound;                      // _294
};

/**
 * The daybreak after the elder hatches: plays the "daybreak" event camera, fades the landscape at frames 0, 125 and
 * 270 and draws a 2D wash over the screen. Vtable 0x80257E8C. The name is ours.
 *
 * @size 0x68
 */
class TChickenBossDemo : public TObject {
public:
	TChickenBossDemo(TObject* parent, int autoDelete);
	virtual ~TChickenBossDemo();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();

	int isDone();

	DrawTask* mDraw; // _20
	f32 mFade;       // _24 0 to 1 over 240 frames
	Fx* mFx;         // _28
	f32 mMtx[12];    // _2C in front of the camera
	TCamPath* mPath; // _5C
	u8 mPhase;       // _60
	int mAutoDelete; // _64
};

extern "C" {
extern const char* gTObjChickenBossName;
extern const MotionEntry sChickenBossMotionTable[];
}

#endif
