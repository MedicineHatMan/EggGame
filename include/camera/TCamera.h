#ifndef _TCAMERA_H
#define _TCAMERA_H
struct TCameraCtrl;

#include "stage/Stage.h"

// One view The other cameras are built on this

extern "C" {
void objHeapFree(void*, void*);
extern void* gObjHeap;
}

/**
 * How a camera hangs on its player. camLinkInit clears it and keeps the look, camAddLink adds it to the player's
 * list and camRemoveLink takes it off.
 *
 * @size 0x10
 */
struct CamLink {
	CamLink* mPrev; // _00
	CamLink* mNext; // _04
	int mLook;      // _08 the highest wins
	u32 mFlags;     // _0C 1 = picked 2 = became current 4 = lost current 8 = cut when removed 0x10 = cut in
};

/**
 * What a point camera follows: an object found by id (camHeadStart) and where it was. camHeadGetPos reads the point
 * out.
 *
 * @size 0x40
 */
/** One player's view where the eye is what it looks at the projection and the axes and matrix apply() works out */
struct CamView { // size 0x98
	CamView();

	void init();          // 0x80007BD0
	void setFov(int fov); // 0x80007C88
	void resetFov();      // 0x80007CF0
	void apply();         // 0x80007D28 sets the projection and the camera matrix

	Vec3f mPos;     // _00 the eye
	Vec3f mTarget;  // _0C
	Vec3f mFront;   // _18 the view's axes in world space
	Vec3f mSide;    // _24
	Vec3f mUp;      // _30
	int mRoll;      // _3C
	int mBaseFov;   // _40 what resetFov goes back to
	int mFov;       // _44
	f32 mSlopeX;    // _48 the view's horizontal slope
	f32 mAspect;    // _4C brings a vertical offset onto the same scale
	f32 mNear;      // _50
	f32 mFar;       // _54
	Vec3f mFlat;    // _58 mFront along the ground
	int mYaw;       // _64 the angle of mFlat
	f32 mMtx[12];   // _68 the camera matrix
};

/**
 * One player's list of CamLinks. camAimUpdate picks the link with the highest look as mCurrent once a frame.
 * mFlags: 1 = the list changed, 2 = mCurrent changed this frame, 4 = the next change is a cut. Names are ours.
 *
 * @size 0xC
 */
struct CamAim {
	CamLink* mHead;    // _00
	CamLink* mCurrent; // _04
	u32 mFlags;        // _08
};

extern "C" {
void camLinkInit(CamLink* link, int look);                 // 0x8008A0C4
int camLinkIsAbove(const CamLink* link, const CamLink* other); // 0x8008A0DC
int camLinkIsPicked(const CamLink* link);                  // 0x8008A0FC
int camLinkIsEntered(const CamLink* link);                 // 0x8008A108
void camLinkSetCut(CamLink* link);                         // 0x8008A114
int camLinkIsCut(const CamLink* link);                     // 0x8008A124
CamLink* camLinkPickBest(CamLink* head);                   // 0x8008A130
void camAimClear(CamAim* aim);                             // 0x8008A1BC
void camAimAdd(CamAim* aim, CamLink* link);                // 0x8008A1D0
void camAimRemove(CamAim* aim, CamLink* link);             // 0x8008A220
void camAimUpdate(CamAim* aim);                            // 0x8008A2CC once a frame
}

struct CamHead {
	Vec3f mPos;     // _00
	TObject* mObj;  // _0C the object with mId or 0
	u16 mId;        // _10 0xFFFF = none
	u8 _12[2];
	u32 _14;        // _14
	Vec3f mLastPos; // _18
	f32 _24[5];     // _24
	int _38;        // _38
	u8 _3C[4];      // _3C camTrailClear sets it up
};

/**
 * The smoothed path a point camera moves along. camCurveStart starts it at a point, camCurveUpdate feeds it from a
 * CamHead and camCurveGetPos reads the point.
 *
 * @size 0x64
 */
struct CamCurve {
	u8 _00[4];
	f32 _04;      // _04
	f32 _08;      // _08
	u8 _0C[0x34];
	f32 _40[5];   // _40
	u8 _54[0x10];
};

class TCamera : public TObject {
public:
	TCamera(TObject* parent, int player, int look);
	virtual ~TCamera();
	virtual void draw();            // _0C 0x80005850 moves the view blending from where it was
	virtual void vfn_20();          // _20 the update while its link is the picked one
	virtual void vfn_24();          // _24 0x80005B6C while it is not
	virtual int vfn_28();           // _28 0x80005C40 whether the view needs redoing
	virtual void setTarget(u16 id); // _2C 0x80005C90 writes mKind the id of the object it follows
	virtual void vfn_30();                 // _30 0x80005DE0
	virtual void setEye(const Vec3f* eye);  // _34 0x80005DE4 writes the view's eye
	virtual void getEye(Vec3f* out);        // _38 0x80005E04
	virtual void setAt(const Vec3f* at);    // _3C 0x80005E24 writes the point the view looks at
	virtual void getAt(Vec3f* out);        // _40 0x80005E44
	virtual int getYaw();                  // _44 0x80005E6C the view's mYaw 0 without one
	virtual void vfn_48();                 // _48 0x80005E64
	virtual void vfn_4C();                 // _4C 0x80005E68
	virtual void shake(f32 power);         // _50 0x80005E88 empty here TCameraCtrl shakes the cameras near a point
	virtual void setMode(int mode);        // _54 0x80005E8C empty here TCameraCtrl passes its mMode on

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	BOOL isPicked();                // 0x80005B70
	void release();                 // 0x80005B94
	void setBlendStep(f32 step);    // 0x80005BC0
	int canBlend() const;                 // 0x80005BF0
	void setCut();                  // 0x80005C64
	BOOL getTargetPos(Vec3f* out);  // 0x80005D54 raised by mLift
	void setLift(f32 lift);         // 0x80005DAC
	void setFov(int fov);           // 0x80005DB4
	u16 getTarget(); // 0x80005C88 mKind
	void readFov(); // 0x80005DBC

	void* mView;    // _20
	u8 _24[4];      // _24
	u16 mKind;      // _28
	u16 _2A;
	int mPlayer;    // _2C whose view this drives
	CamLink mLook;  // _30
	f32 mLift;      // _40 how far above its target the camera looks
	int mFov;       // _44
	f32 mBlendStep; // _48 how much of the blend goes each frame
	f32 mBlend;     // _4C 1 to 0 how much of the old view is left
};

// A camera that rides a path
/** Where a camera motion starts: an eye, a point to look at, a roll and a field of view. @size 0x20 */
struct CamPathStart {
	Vec3f mPos;    // _00
	Vec3f mTarget; // _0C
	s16 mRoll;     // _18
	s16 mFov;      // _1A
	u32 _1C;
};

class TCamPath : public TCamera {
public:
	TCamPath(TObject* parent, int player, int look, void* object, void* motion, f32 start, f32 speed);
	virtual ~TCamPath();
	virtual void vfn_10(); // _10 0x80064204 empty
	virtual void vfn_20(); // _20 0x8006404C plays a step into the view
	virtual void vfn_24(); // _24 0x800641E4 a step without the view

	void setStart(const CamPathStart* start); // 0x80063F30
	void startFromView();               // 0x80063F7C

	virtual void vfn_30();  // _30 0x80063F04 back to frame 0
	void setAutoDelete();   // 0x80063F10 deletes itself on the last frame
	void clearAutoDelete(); // 0x80063F20
	void attachToMtx();     // 0x80063FE0 the path rides mMtx
	f32* getMtx();          // 0x80063FF0
	f32 getFrame();         // 0x80063FF8
	void setFrame(f32 frame); // 0x80064000
	int isDone();           // 0x80064008 on the last frame
	void advance();         // 0x80064024 one step of mSpeed held on the last frame

	void* mMotion;      // _50 the camera motion
	CamPathStart mStart; // _54 what the motion is played against
	f32 mFrame;        // _74
	f32 mSpeed;        // _78 frames a step
	f32 mLastFrame;    // _7C one less than the motion's frame count
	u32 mPathFlags;    // _80 2 = delete on the last frame 4 = ride mMtx
	f32 mMtx[12];      // _84
};

// 0x80063D14
// at the end of TCamZelda5.cpp's code
TCamPath* createCamPath(int player, int kind, void* object, void* motion, int autoDelete, f32 start, f32 speed);

// Looks at one point and rolls by two angles
class TCamConversation : public TCamera {
public:
	TCamConversation(TObject* parent, int player, int look, const Vec3f* focus, int yaw, int roll);
	virtual ~TCamConversation();

	void setFocus(const Vec3f* focus); // 0x8008AC74
	void vfn_20();                      // 0x8008AC90

	Vec3f mFocus; // _50
	u32 _5C;
	int mYaw;  // _60
	int mRoll; // _64
};

// 0x8008AB30
extern "C" TCamConversation* createTCamConversation(int player, int look, const Vec3f* focus, int yaw, int roll);

// Sits on one line and looks at one point
class TCamFix : public TCamera {
public:
	TCamFix(TObject* parent, int player, int look, const Vec3f* at, const Vec3f* eye);
	virtual ~TCamFix();
	virtual void vfn_10();

	virtual void setAt(const Vec3f* at);   // _3C 0x8008A560
	virtual void setEye(const Vec3f* eye); // _34 0x8008A57C
	void vfn_20();                  // 0x8008A598

	Vec3f mAt;  // _50
	Vec3f mEye; // _5C
};

// 0x8008A3B0
// at the end of TObjChickenBoss.cpp's code
extern "C" TCamFix* createTCamFix(int player, int look, const Vec3f* at, const Vec3f* eye);

class TObjAction;
struct CamSaved;
struct Vec3f;

extern "C" {
void camCtrlInit();
void camCtrlTerm();
TObjAction* camFindTarget(int, Vec3f*);
void camSavedClear(CamSaved*);
void lerpAngle(int, int, int*, f32);
}

extern "C" {
TCameraCtrl* createCameraCtrl();
TCameraCtrl* createEditCameraCtrl();
}

#endif
