#ifndef _TPOLYGONANIM_H
#define _TPOLYGONANIM_H

#include "stage/Stage.h"
#include "engine/DrawTask.h"

extern "C" void objHeapFree(void*, void*);

/**
 * The polygon-effect list. Five draw tasks, one per pass, and a vector draw() scales for the passes to add on.
 *
 * @size 0x44
 */
class TPolygonAnim : public TObject {
public:
	TPolygonAnim(TObject* parent);
	virtual ~TPolygonAnim();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw(); // _0C

	// The three copies add mAt onto the vec at +0x3C and return 0
	void addAt(void* dst) const;
	int addAt0(void* dst);
	int addAt1(void* dst);
	int addAt2(void* dst);

	DrawTask* mTask[5]; // _20
	Vec3f mAt;          // _34
	u32 _40;            // _40
};

// Builds one and stores it in gFxPolygonAnim
void createTPolygonAnim();

extern "C" {
extern const char* gTPolygonAnimName;
}

#endif
