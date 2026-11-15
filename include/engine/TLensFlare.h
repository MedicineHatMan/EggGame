#ifndef _TLENSFLARE_H
#define _TLENSFLARE_H
#include "Ninja/ninja.h"
#include "core/types.h"
struct FlareQuad;
struct FlareSprite;

#include "stage/Stage.h"

extern "C" void objHeapFree(void*, void*);
extern "C" void* gObjHeap;

/**
 * @size 0x40
 */
class TLensFlare : public TObject {
public:
	TLensFlare(TObject* parent, const Vec3f* pos);
	virtual ~TLensFlare();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();

	void* mTexture;  // _20
	DrawTask* mTask; // _24
	Vec3f mPos;      // _28
	u8 mAlpha;       // _34
	f32 mFade;       // _38
	int mVisible;    // _3C
};

extern "C" TLensFlare* lensFlareCreate(void* owner, const Vec3f* pos); // 0x800D3B18

/**
 * @size 0x10
 */
struct FlareSprite {
	f32 mAlong; // _00 how far along the line
	f32 mFar;   // _04 size at the far end
	f32 mNear;  // _08 size at the near end
	f32 mU;     // _0C
};

/**
 * @size 0x20
 */
struct FlareQuad {
	f32 x1, y1, x2, y2; // _00
	f32 u1, v1, u2, v2; // _10
};

extern "C" {
extern const char* gTLensFlareName;
extern u16 sLensFlareFrame;
extern s8 sLensFlareInit;
extern FlareSprite sFlareSprites[8];
extern FlareQuad sFlareZeroQuad;
}

#endif
