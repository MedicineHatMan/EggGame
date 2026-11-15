#ifndef _TCAMPOINTLOOK_H
#define _TCAMPOINTLOOK_H

#include "camera/TCamera.h"

class TCamPointLook : public TCamera {
public:
	TCamPointLook(TObject* parent, int player, int look, const Vec3f* target, f32 distance);
	virtual ~TCamPointLook();
	virtual void vfn_10();

	void setTarget(const Vec3f* target); // 0x800A5D18
	void setDistance(f32 distance);      // 0x800A5D34
	virtual void setTarget(u16 id);      // _2C 0x800A5D50
	void vfn_20();                       // 0x800A5DAC

	CamHead mHead;      // _50
	HitVec mSample[15]; // _90
	CamCurve mCurve;    // _144
	HitVec mKnot[15];   // _1A8
	Vec3f mTarget;      // _25C
	f32 mDistance;      // _268
};

TCamPointLook* createTCamPointLook(int player, int look, const Vec3f* target, f32 distance);

#endif
