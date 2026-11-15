#ifndef _TENELASTBOSSMAGIC_H
#define _TENELASTBOSSMAGIC_H
struct CharaColli;

#include "object/ObjAction.h"

extern "C" void objHeapFree(void*, void*);

/**
 * @size 0x4C
 */
class TEneLastBossMagic : public TObjAction {
public:
	TEneLastBossMagic(TObject* parent, const Vec3f* pos, f32 life); // 0x8015EA1C
	virtual ~TEneLastBossMagic();

	virtual void draw();     // _0C
	virtual Vec3f* getPos(); // _20
	virtual int getKind();   // _2C
	virtual int onHit(const TObjAction* other, int arg, int* result);

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	void resize();

	Vec3f mPos;         // _30
	CharaColli* mShape; // _3C
	f32 mLife;          // _40
	f32 mStep;          // _44
	f32 mScale;         // _48
};

extern "C" {
void initLastEggLightSet();
void termLastEggLightSet();
}

#endif
