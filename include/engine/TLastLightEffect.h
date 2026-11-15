#ifndef _TLASTLIGHTEFFECT_H
#define _TLASTLIGHTEFFECT_H

#include "stage/Stage.h"
#include "engine/Light.h"

/**
 * @size 0x10C
 */
class TLastLightEffect : public TObject {
public:
	TLastLightEffect(TObject* parent, void* motion, int fx, const f32* mtx, const u32* color);
	virtual ~TLastLightEffect();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw(); // _0C

	void* mFx;         // _20
	void* mMotion;     // _24
	f32 mMtx[12];      // _28
	f32 mBase[12];     // _58
	int mFxId;         // _88
	int mFrame;        // _8C
	PointLight mLight; // _90
};

extern "C" {
extern const char* gTLastLightEffectName;
}

#endif
