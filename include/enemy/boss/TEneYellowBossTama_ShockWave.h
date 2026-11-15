#ifndef _TENEYELLOWBOSSTAMA_SHOCKWAVE_H
#define _TENEYELLOWBOSSTAMA_SHOCKWAVE_H

#include "object/ObjAction.h"

struct CharaColli;

class TEneYellowBossTama_ShockWave : public TObjAction {
public:
	TEneYellowBossTama_ShockWave(TObject* parent, const Vec3f* pos);
	virtual ~TEneYellowBossTama_ShockWave();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                                              // _0C
	virtual Vec3f* getPos();                                          // _20
	virtual int getKind();                                            // _2C
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58

	Vec3f mPos;         // _30
	CharaColli* mShape; // _3C
	f32 mRadius;        // _40 grows by 3.33 a frame
	int mLife;          // _44 frames left once the ring is full size
};

extern "C" {
extern const char* sShockWaveName;
}

#endif
