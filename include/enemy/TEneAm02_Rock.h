#ifndef _TENEAM02_ROCK_H
#define _TENEAM02_ROCK_H
struct CharaColli;

#include "object/ObjAction.h"
struct DrawTask;
#include "object/ObjCommon.h"
#include "engine/RockFlight.h"

/**
 * The rock TEneAm02 throws. Kind 3, so it takes an id from the set-object range.
 *
 * mState: 0 while it grows in the hand, 1 in the air, 2 shrinking after 300 frames.
 *
 * @size 0xF4
 */
class TEneAm02_Rock : public TObjAction {
public:
	TEneAm02_Rock(TObject* parent, Vec3f* pos, int rotY);
	virtual ~TEneAm02_Rock();

	inline void operator delete(void* mem);

	virtual void draw();                                              // _0C the per-frame update
	virtual void vfn_10();                                            // _10
	virtual void render();                                            // _1C draws the model
	virtual Vec3f* getPos();                                          // _20
	virtual int getKind();                                            // _2C
	virtual int vfn_48(TObjAction* other);                            // _48
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58

	void hold(Vec3f* pos, int rotY); // 0x80041304
	void throwAt(Vec3f* target);     // 0x80041140

	RockFlight mFlight; // _30
	int mAngle;         // _A4 the holder's facing while it's held
	f32 mScale;         // _A8 grows from 0.1 to 1 then back down
	ObjShadow mShadow;  // _AC
	CharaColli* mShape; // _E4
	DrawTask* mTask;    // _E8
	int mState;         // _EC
	int mTimer;         // _F0 frames in the air
};

#endif
