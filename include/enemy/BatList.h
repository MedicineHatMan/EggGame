#ifndef _BATLIST_H
#define _BATLIST_H

#include "enemy/TEneAm01.h"

/**
 * @size 0xC
 */
struct BatList {
	BatList();
	~BatList();

	void link(BatList* head);
	void unlink();
	void remove();
	int hasAny();
	int isEmpty();
	void spawn(Vec3f* center, int count);

	BatList* mPrev; // _00 toward the head The head has none
	BatList* mNext; // _04
	int mCount;     // _08 the head's only
};

/**
 * The brain of a TEneAm01 a bat lets go. It first lands somewhere on a ring around where the bat was, then leaps like
 * Am01LeapBrain.
 *
 * @size 0x70
 */
struct Am01BatBrain : public Am01LeapBrain {
	enum {
		STATE_DROP = 5,
	};

	Am01BatBrain(BatList* head, const Vec3f* center, int angle);
	virtual ~Am01BatBrain();

	virtual void onPhase(int state, int phase); // _0C
	virtual void onDeathStart();                // _20
	virtual void attach(TEneAm01* owner);       // _24

	void phaseDrop(int phase);

	Vec3f mCenter; // _54 where the bat was
	int mAngle;    // _60 around mCenter
	BatList mLink; // _64
};

#endif
