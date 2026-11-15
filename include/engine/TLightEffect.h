#ifndef TLIGHTEFFECT_H
#define TLIGHTEFFECT_H

#include "stage/Stage.h"
#include "engine/Light.h"

/**
 * @size 0x10C
 */
class TLightEffect : public TObject {
public:
	TLightEffect(TObject* parent, void* motion, int effect, const f32* matrix, const u32* color);
	virtual ~TLightEffect();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	void* mEffect;     // _20
	void* mMotion;     // _24
	f32 mMatrix[12];   // _28
	f32 mBase[12];     // _58
	int mEffectId;     // _88
	int mFrame;        // _8C
	PointLight mLight; // _90 joins the player's lights at frame 110
};

extern "C" {
int lightEffectLoad();
void lightEffectLoadArc();
}

#endif
