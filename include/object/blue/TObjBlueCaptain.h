#ifndef _TOBJBLUECAPTAIN_H
#define _TOBJBLUECAPTAIN_H
struct BeamArena;
struct Vec3f;
struct SetDesc;
struct TObject;

#include "engine/DrawTask.h"
#include "engine/MotionPlayer.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"

class TCamPath;

extern "C" void objHeapFree(void*, void*);

/**
 * A captain the on-switch wakes and the off-switch puts away. Three models share one motion id.
 *
 * 0 waits, 1 plays the wake sound, 2 stays up, 3 plays the put-away motion, 4 stays down.
 *
 * @size 0x1BC
 */
// A spiral of points copied into gBeamPeraGround
struct CaptainRing {
	f32 mA;     // _00 0
	f32 mB;     // _04 0
	f32 mX;     // _08 200
	f32 mY;     // _0C 180
	f32 mZ;     // _10 190
	int mFlag0; // _14 1
	int mFlag1; // _18 1
	int mKind;  // _1C 0x4C
	int mId;    // _20 0x4B
};

void buildCaptainRing(const CaptainRing* desc); // 0x800BDA50
void freeCaptainRing();                         // 0x800BDC60

class TObjBlueCaptain : public TSetActor {
public:
	enum State { STATE_WAIT = 0, STATE_START = 1, STATE_ON = 2, STATE_STOP = 3, STATE_OFF = 4 };

	TObjBlueCaptain(TObject* parent, SetDesc* desc);
	virtual ~TObjBlueCaptain();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void*);           // _34
	virtual void vfn_74();                    // _74 loads the models and picks the starting motion

	void buildMatrix();
	void play(int id);
	int advance();
	void stateWait();
	void stateStart();
	void stateOn();
	void stateStop();
	void stateOff();

	DrawTask* mDraw;       // _64
	ObjModel mModel0;      // _68
	ObjModel mModel1;      // _88
	ObjModel mModel2;      // _A8
	MotionPlayer mMotion0; // _C8
	MotionPlayer mMotion1;
	MotionPlayer mMotion2;
	int mOnSwitch;   // _17C
	int mOffSwitch;  // _180
	f32 mMatrix[12]; // _184
	s16 mState;      // _1B4
	u8 _1B6[2];
	TCamPath* mEventCam; // _1B8 the event camera the on-switch plays
};

extern "C" void objHeapFree(void*, void*);

extern "C" {
void freeCaptainRing__Fv();
void initCaptainRing__Fv();
}

extern "C" {
TObject* createTObjBlueCaptain(TObject*, SetDesc*);
}

extern "C" {
extern BeamArena* gBeamPeraGround;
extern Vec3f* gHitPoints;
extern int gHitPointCount;
extern f32 sCaptainLen;
}

#endif
