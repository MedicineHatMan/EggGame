#ifndef _TCAMZELDA5_H
#define _TCAMZELDA5_H

#include "camera/CamChase.h"

/**
 * @size 0x424
 */
class TCamZelda5 : public TCamera {
public:
	enum {
		ZELDA_CHANGED   = 1, // a setting changed this frame
		ZELDA_LOOK_FROM = 4, // the next cut turns the chase to look away from mLookFrom
	};

	TCamZelda5(TObject* parent, int player, int look);
	virtual ~TCamZelda5();
	virtual void vfn_10();          // _10 0x80063D10 empty
	virtual void vfn_20();          // _20 0x800639A4 the update
	virtual void vfn_24();          // _24 0x80063D0C empty
	virtual int vfn_28();           // _28 0x800636CC
	virtual void setTarget(u16 id); // _2C 0x80063530 starts over when it gets something to follow
	virtual void vfn_30();          // _30 0x800635E8 starts over
	virtual void shake(f32 power);  // _50 0x80063694
	virtual void setMode(int mode); // _54 0x800636C8 empty

	void start();                              // 0x800633D4
	void snap();                               // 0x80063460 straight behind the head
	void restart();                            // inlined in setTarget and vfn_30
	void setCurveHeight(f32 height);           // 0x80063718
	void setChaseBase(f32 height, f32 dist);   // 0x80063750
	void setChaseBaseSpeed(f32 speed);         // 0x8006378C
	void setCurveLift(f32 lift);               // 0x800637C4
	void resetCurveLift();                     // 0x80063800
	void setChaseOffset(f32 height, f32 dist); // 0x8006383C
	void resetChaseOffset();                   // 0x8006387C
	void setChaseSpeed(f32 speed);             // 0x800638B8
	void resetChaseSpeed();                    // 0x800638DC
	void setChasePitch(int pitch);             // 0x80063900
	void resetChasePitch();                    // 0x80063924
	void resetChase(int rot);                  // 0x80063948
	void setLookFrom(const Vec3f* from);       // 0x8006397C

	u32 mCamFlags;      // _50 ZELDA_ bits
	BOOL mShaking;      // _54
	f32 mShakeVel;      // _58
	f32 mShake;         // _5C the swing settling back to 0
	CamHead mHead;      // _60
	HitVec mSample[15]; // _A0
	Vec3f mCurvePos;    // _154
	CamCurve mCurve;    // _160
	HitVec mKnot[15];   // _1C4
	CamChase mChase;    // _278
	Vec3f mLookFrom;    // _418
};

#endif
