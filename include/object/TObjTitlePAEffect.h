#ifndef _TOBJTITLEPAEFFECT_H
#define _TOBJTITLEPAEFFECT_H
struct SetDesc;
struct TObject;

#include "stage/SetActor.h"

extern "C" void objHeapFree(void*, void*);

/**
 * A title-screen particle that follows the camera. In mode 3 of gTitleScene the effect id is 0 and the matrix sits
 * on the camera. Otherwise the effect id is 1 and the matrix is 100 to the left of the camera, then 50 up and 100
 * forward, aimed along (1, 0, 0).
 *
 * @size 0x9C
 */
class TObjTitlePAEffect : public TSetActor {
public:
	TObjTitlePAEffect(TObject* parent, SetDesc* desc);
	virtual ~TObjTitlePAEffect();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw(); // _0C

	void* mFx;       // _64
	u32 _68;         // _68
	f32 mMatrix[12]; // _6C the effect follows this
};

extern "C" {
void initTObjTitlePAEffect();
void termTObjTitlePAEffect();
}

extern "C" {
TObject* createTObjTitlePAEffect(TObject*, SetDesc*);
}

extern "C" {
extern const char* gTObjTitlePAEffectName;
}

#endif
