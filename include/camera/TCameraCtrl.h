#ifndef _TCAMERACTRL_H
#define _TCAMERACTRL_H
struct CamView;
struct TObjAction;

#include "camera/TCamera.h"
#include "camera/CamArea.h"

class TCamLockOnCursor;

/** A short record camKeyClear clears a half and two bytes */
struct CamPathKey { // size 0x4
	u16 mValue;     // _00
	u8 mPressed;    // _02 this frame 1 down 2 hard
	u8 mHeld;       // _03 1 down 2 hard
};

/** A record camSpanClear clears */
struct CamPathSpan { // size 0x10
	u32 mPressed;    // _00 the bits new this frame
	u32 mHeld;       // _04
	f32 mValue;      // _08 -1 to 1
	s16 mRepeat;     // _0C frames it has been one way negative for back
	s16 mRaw;        // _0E
};

/** Per camera the pad as the camera reads it */
struct CamPath {          // size 0x2C
	CamPathKey mKey[2];   // _00
	int mKeyIndex;        // _08 the button the checks read
	CamPathSpan mSpan[2]; // _0C x then y
};

/** Per camera a setting saved to put back later whether it is set and three values */
struct CamSaved { // size 0x10
	int mSet;     // _00
	f32 mA;       // _04 for camSetCurveLift
	f32 mB;       // _08 for camSetChaseOffset
	f32 mC;       // _0C
};

class TCameraCtrl : public TObject {
public:
	TCameraCtrl();
	virtual ~TCameraCtrl();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw(); // _0C 0x8000632C

	void setView(int i);                                              // 0x80006490
	void setTarget(int i, TObjAction* obj);                           // 0x8000660C
	u16 getTarget(int i);                                             // 0x800066F4
	void getPos(int i, Vec3f* out);                                   // 0x80006AC4
	void getFlat(int i, Vec3f* out);                                  // 0x80006BB4
	int getYaw(int i);                                                // 0x80006D10
	void getAt(int i, Vec3f* out);                                    // 0x80006DA8
	CamView* getView(int i);                                          // 0x80006E50
	CamView* getDrawView();                                           // 0x80006ED8
	TCamera* getCamera(int i);                                        // 0x80006F44
	f32* getMtx(int i);                                               // 0x80006F94
	f32* getDrawMtx();                                                // 0x80006FE0
	int getRoll(int i);                                               // 0x80007004
	int getDrawRoll();                                                // 0x8000704C
	int isVisible(const Vec3f* pos, Vec3f* out, f32 far, f32 radius); // 0x800070E0
	int getDrawIndex();                                               // 0x80007220
	int getPad(int i);                                                // 0x80007270
	CamPath* getPath(int i);                                          // 0x80007308
	void resetCamera(int i);                                          // 0x80007398
	void shake(int i, f32 power);                                     // 0x80007420
	void shakeNear(const Vec3f* pos, f32 range, f32 power);           // 0x800074EC
	void setMode(int mode);                                           // 0x80007620
	int lockA(int i);                                                 // 0x80007740
	void lockB(int i);                                                // 0x800077D4
	void lockC(int i);                                                // 0x80007860
	void lockD(int i);                                                // 0x800078EC
	u16 lockGetId(int i);                                             // 0x80007984
	void lockE(int i);                                                // 0x80007A7C
	void lockF(int i);                                                // 0x80007B08
	void lockAll();                                                   // 0x80007B84

	void makeGameCameras();     // 0x80005FD0 in TCamera.cpp
	void makeEditCameras();     // 0x80005FF4
	void makeCameras(int mode); // 0x80006018 0 the game cameras 1 the edit ones

	CamView mView[4];           // _20
	TCamera* mCamera[4];        // _280
	TCamLockOnCursor* mLock[4]; // _290
	CamAim mAim[4];             // _2A0
	CamPath mPath[4];           // _2D0
	int mPad[4];                // _380 the pad each camera reads
	CamAreaCtrl* mAreas;        // _390
	CamSaved mSaved[4];         // _394
	TCamera* mEditor[4];        // _3D4 the edit cameras when makeCameras(1) made them
	int mDrawIndex;             // _3E4 the camera being drawn from
	int mMode;                  // _3E8
};

extern "C" TCameraCtrl* gCameraCtrl; // 0x802C4B40

typedef char TCameraCtrl_size_check[(sizeof(TCameraCtrl) == 0x3EC) ? 1 : -1];

class TCamera;
struct CamLink;
struct Vec3f;

extern "C" {
void camAddLink(int, CamLink*);
void camClearSaved(int);
void camGetAt(int, Vec3f*);
TCamera* camGetCamera(int);
void camGetDrawFlat(Vec3f*);
f32* camGetDrawMtx();
void camGetDrawPos(Vec3f*);
int camGetDrawRoll();
int camGetDrawYaw();
void camGetFlat(int, Vec3f*);
int camGetPad(int);
CamPath* camGetPath(int);
CamSaved* camGetSaved(int);
u16 camGetTarget(int);
int camIndexValid(int);
void camLoadAreas(const char*);
void camLockAll();
u16 camLockGetId(int);
void camRemoveLink(int, CamLink*);
void camResetCamera(int);
void camSave(int, f32, f32, f32);
void camSetDrawView(int);
void camSetFar(f32, int);
void camSetModeOne();
void camShake(int, f32);
void camShakeNear(f32, f32, const Vec3f*);
int cameraCtrlExists();
void camSetCurveLift(f32, int);
void camResetCurveLift(int);
void camSetChaseOffset(f32, f32, int);
void camResetChaseOffset(int);
void camSetChasePitch(int, int);
void camResetChasePitch(int);
void camSetLookFrom(int, const Vec3f*);
}

extern "C" {
int cameraIsVisible(f32, f32, const Vec3f*, Vec3f*);
void camSetTarget(int, TObjAction*);
int camGetYaw(int);
}

extern "C" {
CamView* camGetDrawView();
void camShakeNearHard(f32, const Vec3f*);
void camShakeHard(int);
int camLockA(int);
void camLockB(int);
void camLockC(int);
void camLockD(int);
void camLockGetTarget(int, void*);
void camGetPos(int, Vec3f*);
void camUndoSaved(int);
void camRestoreSaved(int);
}

extern "C" {
CamView* camGetView(int);
void camSetChaseBase(int, f32, f32);
}

extern "C" {
int camGetDrawIndex();
void camShakeNearSoft(f32, const Vec3f*);
void camLockE(int);
void camLockF(int);
}

extern "C" {
extern const char* gTCameraCtrlName;
}

#endif
