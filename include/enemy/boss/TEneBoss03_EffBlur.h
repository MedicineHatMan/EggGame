#ifndef _TENEBOSS03_EFFBLUR_H
#define _TENEBOSS03_EFFBLUR_H

#include "stage/Stage.h"
struct DrawTask;

/**
 * @size 0x3C
 */
class TEneBoss03_EffBlur : public TObject {
public:
	TEneBoss03_EffBlur(TObject* parent, const Vec3f* pos);
	virtual ~TEneBoss03_EffBlur();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();

	void setPos(const Vec3f* pos);
	void end();

	Vec3f mPos;      // _20
	u32 mColor;      // _2C the low byte is alpha
	u32 mEnding;     // _30 bit 0
	f32 mScale;      // _34
	DrawTask* mDrawTask; // _38
};

extern "C" TEneBoss03_EffBlur* createTEneBoss03_EffBlur(TObject* parent, const Vec3f* pos); // 0x800C4488

extern "C" {
extern const char* gTEneBoss03_EffBlurName;
}

#endif
