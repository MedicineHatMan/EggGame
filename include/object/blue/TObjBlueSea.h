#ifndef _TOBJBLUESEA_H
#define _TOBJBLUESEA_H

#include "stage/Stage.h"
struct NJS_TEXLIST;
struct DrawTask;

extern "C" void objHeapFree(void*, void*);

/**
 * The blue stage's sea. One task draws the textured grid. A non-zero second pass adds the
 * indirect-warp grid on top of it. The scrolls wrap through 0..1, and the waves are two sines.
 *
 * @size 0x48
 */
class TObjBlueSea : public TObject {
public:
	TObjBlueSea(TObject* parent, int secondPass, f32 height);
	virtual ~TObjBlueSea();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C advances the scrolls and the waves
	virtual void vfn_10(); // _10 empty
	virtual void render(); // _1C the indirect pass

	void drawSea();

	void* mWarpTask;     // _20 drawObjectTask when mSecondPass != 0
	DrawTask* mDrawTask; // _24 the textured grid
	f32 mHeight;         // _28
	int mSecondPass;     // _2C
	f32 mScrollU;        // _30
	f32 mScrollV;        // _34
	f32 mWaveU;          // _38
	f32 mWaveV;          // _3C
	int mAngleU;         // _40
	int mAngleV;         // _44
};

extern "C" {
void initBlueSea();
void termBlueSea();
}

extern "C" {
TObjBlueSea* createTObjBlueSea();
}

extern "C" {
void setBlueSeaColor(f32);
}

#include "io/ResBank.h"
extern "C" {
extern const char* gTObjBlueSeaName;
extern ResBank sBlueSeaBank;
extern NJS_TEXLIST** sBlueSeaTextures;
extern u32* sBlueSeaColor;
}

#endif
