#ifndef _CAMCHASE_H
#define _CAMCHASE_H
struct EggHits;
struct MeshList;

#include "camera/TCamera.h"
#include "egg/ObjEgg.h"

struct CamPath;

/** The last 15 places something was each at least some way from the one before */
struct CamTrail {
	int mIndex;        // _00 the newest
	HitVec mPoints[15]; // _04
};

/** What a camera's view looks like to the chase */
struct CamViewInfo {
	Vec3f mEye;   // _00
	u8 _0C[0xC];
	Vec3f mDir;   // _18 the way the view faces
	u8 _24[0x24];
	f32 mSlope;   // _48 how far the view reaches up for each unit ahead
};

/** One steered value a position from -1 to 1 its speed and how many frames the input has pushed one way negative for
 * the other way */
struct CamSteer {
	int mCount; // _00
	f32 mVel;   // _04
	f32 mPos;   // _08
};

/** The states of a CamChase */
enum CamChaseState {
	CHASE_FOLLOW = 0, // behind the head swinging round when it turns
	CHASE_SWING  = 1, // the stick turns the camera
	CHASE_BEHIND = 2, // the button swings it back behind the head
	CHASE_LOCK   = 3, // looking at a lock-on target
	CHASE_IDLE   = 4,
};

/** What a state handler is called for */
enum CamChaseEvent {
	CHASE_ENTER  = 0,
	CHASE_CHECK  = 1, // may change the state
	CHASE_UPDATE = 2,
	CHASE_EXIT   = 3,
};

/**
 * The chase camera: sits behind a CamHead at a height and a distance, steered by the pad, the ground ahead and
 * the walls round it. TCamZelda5 keeps one at _278, with its head at _60 and its curve at _160. 0x80095A44-
 * 0x80098734. The names are ours.
 *
 * @size 0x1A0
 */
struct CamChase {
	CamCurve* mCurve;     // _00
	CamHead* mHead;       // _04
	CamViewInfo* mView;   // _08 set each update
	Vec3f mPos;           // _0C the eye
	f32 mBaseHeight;      // _18
	f32 mBaseDist;        // _1C
	f32 mHeight;          // _20
	f32 mDist;            // _24 negative behind
	CamSteer mGround;     // _28 the pitch the ground ahead asks for
	CamSteer mStick;      // _34 the pitch the stick asks for
	int mPitchMax;        // _40
	int mPitch;           // _44 the stick pitch's top
	int mPitchMin;        // _48
	int mWallCount;       // _4C frames since a wall was near
	f32 _50;              // _50
	f32 mWall;            // _54 how much the walls pull the camera in
	int mSideCount;       // _58
	f32 mSideVel;         // _5C
	f32 _60;              // _60
	f32 mSide;            // _64 the turn the side walls ask for
	f32 _68;              // _68
	f32 mOffset[2];       // _6C the eye's height and distance from the head this frame
	int mRot;             // _74 the way the camera looks
	f32 mRotVel;          // _78
	f32 mBaseSpeed;       // _7C
	f32 mSpeed;           // _80
	f32 mRiseRate;        // _84
	f32 mFallRate;        // _88
	f32 mDropRate;        // _8C
	f32 mFollowRate;      // _90
	Vec3f mTarget;        // _94 the lock-on target
	Vec3f mToTarget;      // _A0
	CamTrail mTrail;      // _AC
	CamPath* mPad;        // _164
	u32 mHits;            // _168 what the walls did this frame 1 up 2 down 4 8 the sides 0x10 0x20 ahead 0x40 no sight
	u32 mFlags;           // _16C 1 2 the follow has stopped the walls
	int mState;           // _170 a CamChaseState
	int mSub;             // _174
	int mTimer;           // _178
	int mSubFlag;         // _17C
	int mSubRot;          // _180
	f32 mSubStep;         // _184
	int mFollow;          // _188
	int mFollowTimer;     // _18C
	int mWallFrames;      // _190
	f32 mGap;             // _194
	f32 mRise;            // _198
	u32 mGapFlags;        // _19C
};

typedef char CamChase_size_check[(sizeof(CamChase) == 0x1A0) ? 1 : -1];

extern "C" {
void camChaseInit(CamChase* chase, CamHead* head, CamCurve* curve); // 0x80095E74
void camChaseReset(CamChase* chase, int rot);                       // 0x80095F30
void camChaseSnap(CamChase* chase, int rot);                        // 0x80096010
void camChaseGetPos(const CamChase* chase, Vec3f* out);                   // 0x800960F8
void camChaseSetBase(CamChase* chase, f32 height, f32 dist);        // 0x80096114
void camChaseSetOffset(CamChase* chase, f32 height, f32 dist);      // 0x80096120
void camChaseResetOffset(CamChase* chase);                          // 0x8009612C
void camChaseSetBaseSpeed(CamChase* chase, f32 speed);              // 0x80096140
void camChaseSetSpeed(CamChase* chase, f32 speed);                  // 0x80096158
void camChaseResetSpeed(CamChase* chase);                           // 0x80096170
void camChaseSetPitch(CamChase* chase, int pitch);                  // 0x8009617C
void camChaseResetPitch(CamChase* chase);                           // 0x80096184
void camChaseUpdate(CamChase* chase, CamViewInfo* view, CamPath* pad); // 0x80097668
void camChaseSetState(CamChase* chase, int state);                  // 0x800976D8
}

struct EggHit;
struct Vec3f;

extern "C" {
f32 camAccel(f32, f32, f32, f32);
f32 camAngleSign(int, int, int*);
f32 camAngleToMove(f32, int);
void camChaseBlendWall(CamChase*, f32);
void camChaseCallState(CamChase*, int, int);
int camChaseCheckBehind(CamChase*);
bool camChaseCheckButtons(CamChase*);
int camChaseCheckFollow(CamChase*);
int camChaseCheckLock(CamChase*);
void camChaseCheckSight(CamChase*);
void camChaseCheckWall(CamChase*);
void camChaseClearFollow(CamChase*);
void camChaseDampSide(CamChase*);
void camChaseEyeAt(CamChase*, const Vec3f*, Vec3f*);
int camChaseFindView(CamChase*, f32, int, int);
int camChaseHardHeld(CamChase*);
void camChaseHeadTop(CamChase*, Vec3f*);
void camChaseLookGround(CamChase*);
void camChaseMove(CamChase*);
void camChasePullIn(CamChase*, f32);
void camChaseReadStick(CamChase*);
void camChaseStateBehind(CamChase*, int);
void camChaseStateFollow(CamChase*, int);
void camChaseStateIdle(CamChase*, int);
void camChaseStateLock(CamChase*, int);
void camChaseStateSwing(CamChase*, int);
void camChaseSteerGround(CamChase*, f32);
void camChaseSteerSide(CamChase*, f32);
void camChaseSteerStick(CamChase*, f32);
int camChaseStickSide(CamChase*);
void camChaseTiltGround(CamChase*);
void camChaseTiltSide(CamChase*);
void camChaseTiltStick(CamChase*);
int camChaseTryLift(CamChase*, f32, const f32*, int, const Vec3f*);
int camChaseTryTurn(CamChase*, f32, const f32*, int, const Vec3f*);
void camClampUnit(CamChase*, f32*, f32*);
void camDamp(CamChase*, f32*, f32*, int*);
f32 camFitRange(f32, f32, f32, f32, f32, u32*);
int camLineHit(EggHit*, const Vec3f*, const Vec3f*, int);
f32 camMaxScaled(f32, f32, f32);
int camMoveToAngle(f32, f32);
void camSteer(CamChase*, f32, f32*, f32*, int*, int, int);
void camTilt(CamChase*, f32, f32*, int, int);
void camTurn(f32, f32, f32, int*);
}

extern "C" {
u32 camLineHitList(EggHits*, MeshList*, const Vec3f*, const Vec3f*, u32);
f32 camProbeAlong(MeshList*, const Vec3f*, const Vec3f*, f32, f32);
void camChasePushUp(CamChase*, f32, MeshList*, const Vec3f*);
void camChasePushSide(CamChase*, f32, MeshList*, const Vec3f*, const Vec3f*);
}

#endif
