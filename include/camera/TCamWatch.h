#ifndef _CAMERA_TCAMWATCH_H_
#define _CAMERA_TCAMWATCH_H_
#include "Ninja/ninja.h"
#include "camera/TCamera.h"
#include "egg/ObjEgg.h"
#include "stage/Stage.h"
struct TCamWatch;
struct TObject;
struct TCamera;
struct Vec3f;

#include "core/types.h"

extern "C" {
TCamera* createTCamWatch(int, int, const Vec3f*);
}

class TCamWatch : public TCamera {
public:
	TCamWatch(TObject* parent, int player, int look, const Vec3f* target);
	virtual ~TCamWatch();
	virtual void vfn_10();

	virtual void setTarget(u16 id); // _2C 0x8008A768
	virtual void vfn_20();          // 0x8008A7C4

	CamHead mHead;       // _50
	HitVec mSample[15]; // _90
	CamCurve mCurve;    // _144
	HitVec mKnot[15];   // _1A8
	Vec3f mTarget;     // _25C
};

#endif
