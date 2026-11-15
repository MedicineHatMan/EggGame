#ifndef _TANIMALRACER_H
#define _TANIMALRACER_H
struct TArc;
#include "core/types.h"

class TSimpleText;
struct RacerPack;
struct GjActor;
struct CharaColli;

#include "object/ObjAction.h"
#include "animal/AniAsset.h"
#include "engine/MotionPlayer.h"
#include "object/ObjCommon.h"
#include "object/ObjModel.h"
#include "engine/DrawTask.h"
#include "enemy/TEneAm02.h"
#include "animal/RacerMover.h"

/** A Vec3f set to one value on all three axes */
struct RacerScale : public Vec3f {
	inline RacerScale(f32 s)
	{
		x = s;
		y = s;
		z = s;
	}
};

extern "C" void objHeapFree(void*, void*);

// One row of the course table at gAnimalParams
struct RacerInfo {
	u8 _00[4];
	int mKind; // _04 copied into mStep and minus 3 is the mover's kind
	f32 mRate; // _08
	u8 _0C[4];
	f32 mCap;   // _10
	f32 mBlend; // _14
	u8 _18[8];
	void* mJump;    // _20
	int mClipCount; // _24
	void** mClips;  // _28
};

// A frame window on mClips
struct RacerClip {
	int mMotion; // _00
	int mFrom;   // _04
	int mTo;     // _08
	int mA;      // _0C
	int mB;      // _10
};

// The cylinder colliShapeMakeCylinder fills at +0x190
struct RacerShape {
	u8 _00[0x24];
	u32 mFlags; // _24 8 and 0x200
	u8 _28[8];
};

/**
 * The animal that runs the course. Kind 4. getPos is the live position at +0xB4.
 *
 * @size 0x288
 */
class TAnimalRacer : public TObjAction {
public:
	TAnimalRacer(TObject* parent, const Vec3f* pos, f32 scale);
	virtual ~TAnimalRacer();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                                              // _0C
	virtual void vfn_10();                                            // _10
	virtual void render();                                            // _1C
	virtual Vec3f* getPos();                                          // _20
	virtual int getRotY();                                            // _28
	virtual int getKind();                                            // _2C
	virtual int sendToHandle(ObjMsg* msg);                            // _4C
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58

	void setup();
	void setSpeed(f32 rate);
	void playMotion(u16 id, f32 rate, int = 1); // callers pass 1 third never read
	void applyMotion();
	void pose();
	void buildMtx();
	void fireClips();
	void clearClips();
	void probeShadow();
	void setMode(int mode);
	void runMode(int mode, int phase);
	void modeWait(int phase);
	void modeRun(int phase);
	void modeJump(int phase);
	void modeDash(int phase);
	void modeGlide(int phase);
	void modeLand(int phase);
	void chase();
	void leap();
	void fly();
	void launch(const Vec3f* vel, void* jump);
	void fall();
	void showSpark();
	void hideSpark();
	void retireShapes();
	int answerKind();
	int answerRange(void* msg);
	int answerAim(void* msg);

	s16 mMode;            // _30 -1 none -2 while switching
	s16 mPrevMode;        // _32
	s16 mStep;            // _34
	s16 mMotion;          // _36
	u32 _38;              // _38
	RacerMover mMover;    // _3C
	Vec3f mStart;         // _9C
	Vec3f mSaved;         // _A8
	Vec3f mPos;           // _B4
	Vec3f mVel;           // _C0
	s32 mRotX;            // _CC
	s32 mRotY;            // _D0
	s32 mRotZ;            // _D4
	f32 mMtx[12];         // _D8
	RacerScale mScale;    // _108
	Vec3f mNudge;         // _114
	AniAsset mAsset;      // _120
	void* mCourse;        // _138
	RacerInfo* mInfo;     // _13C
	GjActor* mBuf;        // _140
	void* mModel;         // _144
	MotionEntry* mTable;  // _148
	MotionPlayer mPlayer; // _14C
	f32 mRate;            // _188
	CharaColli* mFx;      // _18C
	RacerShape mShape;    // _190
	ObjRef mRef;          // _1C0
	RacerClip* mClips;    // _1CC
	s8 mClipCount;        // _1D0
	u8 _1D1[3];
	void* mJump;        // _1D4
	s32 mAim;           // _1D8
	TSimpleText* mMark; // _1DC
	TObject* mSpark;    // _1E0
	ObjShadow mShadow;  // _1E4
	DrawTask* mTask;    // _21C
	EnemyLight mLight;  // _220
	ObjDrawState mDraw; // _260
	u32 mFlags;         // _284 1 and 2 are jump frames 4 landed 0x20 holds the rate 0x40 at the goal
};

// The name is ours
TAnimalRacer* createTAnimalRacer(const Vec3f* pos, f32 scale);

struct Vec3f;

extern "C" {
void racerExit();
void racerFree();
void racerInit();
void racerLoad(int);
int racerLoadEvent(void*);
void* racerMotionTable(int);
}

// The archive gRacerPack points at
struct RacerPack {
	void** mHead;   // _00
	void* mMotions; // _04
	u8 _08[0xC];
	void** mModel; // _14 the first word is the texture list
	u8* mGvm;      // _18
};

extern "C" {
extern MotionEntry gSaruKoumoriMotions[];
extern const char* gTAnimalRacerName;
extern int gRacerKind;
extern RacerPack* gRacerPack;
extern TArc* gRacerArc;
}

#endif
