#ifndef _OBJECT_LAST_TOBJLASTRIPPLEFIELD_H_
#define _OBJECT_LAST_TOBJLASTRIPPLEFIELD_H_
#include "engine/DrawTask.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjLastRippleField;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjLastRippleField(TObject*, SetDesc*);
}

class TObjLastRippleField : public TSetActor {
public:
	TObjLastRippleField(TObject* parent, SetDesc* desc);
	virtual ~TObjLastRippleField();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void*);           // _34

	void beginDraw();
	void endDraw();

	DrawTask* mDraw;  // _64
	f32 mMtx[12];     // _68
	f32 mReach;       // _98 the half-diagonal of the box for the view test
	int mIndExp;      // _9C the warp matrix's scale exponent int 0
	f32 mIndScale;    // _A0 1 + float 2 how many times the warp texture repeats
	f32 mScrollSpeed; // _A4 float 1 / 100
	f32 mScroll;      // _A8 kept within -1 to 1
	s16 mCooldown;    // _AC frames until the next ripple can start
	u16 _AE;
};

#endif
