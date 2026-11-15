#ifndef _TOBJORANGEMAPMIRROR_H
#define _TOBJORANGEMAPMIRROR_H
struct TObject;

#include "stage/Stage.h"
#include "Ninja/ninja.h"
#include "engine/DrawTask.h"

struct CharaColli;

extern "C" void objHeapFree(void*, void*);

/**
 * One of the three map mirrors. The centre and radius come from the model's first node. Ours.
 *
 * @size 0x20
 */
struct MirrorPiece {
	MirrorPiece();

	NJS_OBJECT* mModel; // _00
	void* mTex;    // _04
	Vec3f mCenter; // _08
	f32 mRadius;   // _14
	int mShow;     // _18 1 when the camera is inside the radius plus 200
	CharaColli* mShape; // _1C
};

/**
 * The three orange map mirrors. One of them is kept in gMapMirrorObj. A piece is drawn while mShow
 * is set, and the first piece whose sphere is touching something hands its two vectors to mirrorSetPlane.
 *
 * @size 0x28
 */
class TObjOrangeMapMirror : public TObject {
public:
	TObjOrangeMapMirror(TObject* parent);
	virtual ~TObjOrangeMapMirror();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_10(); // _10
	virtual void render(); // _1C

	DrawTask* mDrawTask;   // _20
	MirrorPiece* mPieces;  // _24 three
};

void mapMirrorDropSphere(int);

extern "C" {
TObjOrangeMapMirror* createTObjOrangeMapMirror(TObject*);
}

extern "C" {
extern TObjOrangeMapMirror* gMapMirrorObj;
extern const char* gTObjOrangeMapMirrorName;
extern const u32 gPauseBanks[2];
}

#endif
