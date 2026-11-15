#ifndef _ENGINE_TFRAMEBLUR_H_
#define _ENGINE_TFRAMEBLUR_H_
#include "Ninja/ninja.h"
struct DrawTask;
#include "stage/Stage.h"
struct TFrameBlur;
struct TexVtx;

#include "core/types.h"

extern "C" {
void frameBlurCreate();
}

extern "C" {
void frameBlurStart(int, f32);
}

struct TexVtx {
	f32 x, y, z; // _00
	f32 u, v;    // _0C
	u32 col;     // _14
};

/**
 * @size 0x30
 */
class TFrameBlur : public TObject {
public:
	TFrameBlur(TObject* parent);
	virtual ~TFrameBlur();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void render();

	void start(int frames, f32 alpha);

	DrawTask* mTask;  // _20
	f32 mAlpha;   // _24
	int mFrames;  // _28
	int mCaptured; // _2C
};

extern "C" {
extern const char* gTFrameBlurName;
extern TFrameBlur* sFrameBlurObj;
extern int sFrameBlurOn;
}

#endif
