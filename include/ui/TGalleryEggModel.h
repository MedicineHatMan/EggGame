#ifndef _TGALLERYEGGMODEL_H
#define _TGALLERYEGGMODEL_H

#include "stage/Stage.h"
struct DrawTask;

extern "C" void objHeapFree(void*, void*);

/**
 * One egg on the gallery screen. mEgg picks a texture. Ids 0x28, 0x29 and 0x2A
 * take the post-tev path with the texture at +0xB8; id 0x2C uses the texture at
 * +0xBC. draw() advances mFrame by mRate and wraps it on the motion length at +0x04.
 *
 * @size 0x60
 */
class TGalleryEggModel : public TObject {
public:
	TGalleryEggModel(TObject* parent, u16 egg);
	virtual ~TGalleryEggModel();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_10(); // _10 empty
	virtual void render(); // _1C

	void initMotion(); // 0x801B6434

	s16 mEgg;      // _20
	u16 _22;       // _22
	f32 mMtx[12];  // _24
	DrawTask* mDraw;   // _54
	f32 mFrame;    // _58
	f32 mRate;     // _5C
};

class TObject;

extern "C" {
TGalleryEggModel* createTGalleryEggModel(TObject*, u16);
void freeGalleryEggs();
void loadGalleryEggs();
}

extern "C" {
extern const char* gTGalleryEggModelName;
}

#endif
