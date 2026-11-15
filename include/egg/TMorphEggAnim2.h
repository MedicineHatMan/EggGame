#ifndef _TMORPHEGGANIM2_H
#define _TMORPHEGGANIM2_H

#include "object/ObjModel.h"

extern "C" void objHeapFree(void*, void*);

/**
 * The egg that opens into an animal. Kind 0 starts large and clear, kind 2 starts small and solid.
 * draw spins a 2 by 3 basis by mScale and fades mDraw's ambient.
 *
 * @size 0x7C
 */
class TMorphEggAnim2 : public TObject {
public:
	TMorphEggAnim2(TObject* parent, int kind);
	virtual ~TMorphEggAnim2();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw(); // _0C

	int mState;      // _20 0 fade in 1 shrink 2 grow 3 fade out 4 resting
	int mHatched;    // _24 set when the shrink or the fade-out finishes
	int mOpened;     // _28 set when the fade-in finishes
	int mFinished;   // _2C set when the grow finishes
	ObjDrawState mDraw; // _30 flags 0x1000 _20 points at mBasis
	TObject* mChild; // _54 flagged for delete Nothing in this file creates it
	f32 mBasis[6];   // _58 two rows sine and cosine scaled
	f32 mScale;      // _70
	int mAngle;      // _74 0..0xFFFF
	int mSpin;       // _78 added to mAngle each draw
};

class TObject;

extern "C" {
extern const char* gTMorphEggAnim2Name;
}

#endif
