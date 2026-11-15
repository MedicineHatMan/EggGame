#ifndef _CAMERA_TCAMVERTICALLINE_H_
#define _CAMERA_TCAMVERTICALLINE_H_
#include "Ninja/ninja.h"
#include "camera/TCamera.h"
#include "egg/ObjEgg.h"
#include "stage/Stage.h"
struct TCamVerticalLine;
struct TObject;
struct TCamera;
struct Vec3f;

#include "core/types.h"

extern "C" {
TCamera* createTCamVerticalLine(int, int, const Vec3f*, const void*);
}

class TCamVerticalLine : public TCamera {
public:
	TCamVerticalLine(TObject* parent, int player, int look, const Vec3f* target, const void* line);
	virtual ~TCamVerticalLine();
	virtual void vfn_10();

	virtual void setTarget(u16 id); // _2C 0x8008AA00
	void vfn_20();          // 0x8008AA5C

	CamHead mHead;       // _50
	HitVec mSample[15]; // _90
	CamCurve mCurve;    // _144
	HitVec mKnot[15];   // _1A8
	Vec3f mTarget; // _25C
	f32 mRise;     // _268 the float at line+4
	f32 mReach;    // _26C
};

#endif
