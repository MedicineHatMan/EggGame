#ifndef _TEGGEFFBREAK_H
#define _TEGGEFFBREAK_H
struct EggRot;
struct Vec3f;

#include "egg/ObjEgg.h"
struct DrawTask;
struct NJS_TEXLIST;

extern "C" void objHeapFree(void*, void*);

/**
 * One child of the broken model. Built in an array of mCount.
 *
 * @size 0x68
 */
struct EggBreakPiece {
	EggBreakPiece();

	void* mModel;  // _00
	Vec3f mPos;    // _04
	Vec3f mVel;    // _10
	EggRot mRot;   // _1C
	Vec3f mSpin;   // _2C
	f32 mMtx[12];  // _38
};

/**
 * The pieces of an egg model flying apart. Lives 0xB4 frames, then sets the delete flag.
 *
 * @size 0x70
 */
class TEggEffBreak : public TObject {
public:
	TEggEffBreak(f32 radius, TObject* parent, const Vec3f* pos, const EggRot* rot, int id);
	virtual ~TEggEffBreak();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void render(); // _1C

	u16 mId;       // _20 passed to the texture manager
	u16 mLife;     // _22 starts at 0xB4
	u16 mFrame;    // _24
	u16 mCount;    // _26 child nodes of mModel
	void* mAltTex; // _28 0 when the texture manager has no entry for mId
	DrawTask* mTask;   // _2C draw list 0x21
	u8 _30[0x30];  // _30
	f32 mRadius;   // _60
	void* mModel;  // _64 the "models" export
	NJS_TEXLIST* mTexList; // _68 the "texlists" export
	EggBreakPiece* mPieces; // _6C
};

extern "C" {
void createTEggEffBreak(f32, const Vec3f*, const EggRot*, int);
}

extern "C" {
extern const char* gTEggEffBreakName;
}

#endif
