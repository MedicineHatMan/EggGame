#ifndef _TAEOBJGAMEOVER_H
#define _TAEOBJGAMEOVER_H

#include "ui/TAeLayer.h"
struct DrawTask;

/**
 * @size 0x3C
 */
class TAeObjGameOver : public TAeObject {
public:
	TAeObjGameOver(TObject* parent, void* data, void* assets);
	virtual ~TAeObjGameOver();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void render(); // _1C

	TAeLayer* mLayer; // _34
	DrawTask* mTask;      // _38 from addDrawTask
};

extern "C" {
extern const char* gTAeObjGameOverName;
}

#endif
