#ifndef _TSPEAKCURSOR_H
#define _TSPEAKCURSOR_H

#include "stage/Stage.h"
struct NJS_TEXLIST;
struct DrawTask;

extern "C" void objHeapFree(void*, void*);

/**
 * The speech-balloon cursor. It is hidden by clearing flag 0x10. draw() advances
 * mAnim by a small step and wraps it at 1. render() skips while an event is busy,
 * then draws two quads from the texlists export of ar_icon3d.arc. The second quad's
 * size tracks mAnim, folded so it grows and shrinks.
 *
 * @size 0x40
 */
class TSpeakCursor : public TObject {
public:
	TSpeakCursor(TObject* parent);
	virtual ~TSpeakCursor();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void render(); // _1C

	void hide();                            // 0x8010F2B0
	void show();                            // 0x8010F2C0
	void place(const Vec3f* pos, f32 lift); // 0x8010F2D0

	Vec3f mPos;      // _20
	f32 mLift;       // _2C added to Y when drawing
	f32 mAnim;       // _30
	u32 _34;         // _34
	DrawTask* mDraw; // _38
	void* mTex;      // _3C first pointer in the "texlists" export
};

extern "C" {
extern const char* gTSpeakCursorName;
extern NJS_TEXLIST** sSpeakCursorTexLists;
extern f32 sSpeakCursorMtx[];
}

#endif
