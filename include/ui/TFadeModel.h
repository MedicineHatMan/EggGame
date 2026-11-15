#ifndef _UI_TFADEMODEL_H_
#define _UI_TFADEMODEL_H_
#include "stage/Stage.h"
struct DrawTask;
struct TFadeModel;

#include "core/types.h"

extern "C" {
void fadeModelFill(f32 size, f32 depth, u32 alpha);
}

/**
 * @size 0x3C
 */
class TFadeModel : public TObject {
public:
	TFadeModel(TObject* parent, int shape, int open);
	virtual ~TFadeModel();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void render();

	void setTime(int frame);

	int mShape; // _20
	int mOpen;  // _24
	int mTime;  // _28
	f32 mZoom;  // _2C
	int mRotZ;  // _30
	int mAlpha; // _34
	DrawTask* mTask; // _38
};

extern "C" {
extern const char* gTFadeModelName;
extern void* sFadeShapes[2];
}

#endif
