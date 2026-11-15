#ifndef _TAEOBJGO_H
#define _TAEOBJGO_H
struct TArc;

#include "ui/TAeLayer.h"
struct DrawTask;

/**
 * "Go" banner. Layer id 7, draw list 0x3C. draw() steps the layer, and when the clip
 * finishes it sets _30 and marks this object deleted.
 *
 * @size 0x3C
 */
class TAeObjGo : public TAeObject {
public:
	TAeObjGo(TObject* parent, void* data, void* assets);
	virtual ~TAeObjGo();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void render(); // _1C

	TAeLayer* mLayer; // _34
	DrawTask* mTask;      // _38
};

extern "C" {
void createGoAnimal__Fv();
void freeGoAnimalPad__Fv();
void loadGoAnimalPad__Fv();
}

extern "C" {
extern const char* gTAeObjGoName;
struct TAniManager;
extern TAniManager* gAniManager;
extern TArc* sAeGoArc;
}

#endif
