#ifndef TOBJBLAST_H
#define TOBJBLAST_H
struct CharaColli;

#include "object/ObjAction.h"
#include "engine/DrawTask.h"

/**
 * A blast that grows for 30 frames, then stops colliding, and deletes itself at frame 75. Spawned, not placed.
 * The radius grows by mGrow each frame, and mGrow shrinks by 0.85.
 *
 * @size 0x58
 */
class TObjBlast : public TObjAction {
public:
	TObjBlast(TObject* parent, const Vec3f* pos, f32 radius, f32 grow, int power);
	virtual ~TObjBlast();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual Vec3f* getPos();
	virtual int getKind();
	virtual int vfn_48(TObjAction* other);
	virtual int onHit(const TObjAction* other, int arg, int* result);

	void build();
	void grow();

	int mAct;        // _30 0 growing 1 finished
	int mPower;      // _34
	u32 mFrame;      // _38
	Vec3f mPos;      // _3C
	f32 mRadius;     // _48
	f32 mGrow;       // _4C
	CharaColli* mShape;    // _50
	DrawTask* mDraw; // _54
};

extern "C" {
extern const char* gTObjBlastName;
}

#endif
