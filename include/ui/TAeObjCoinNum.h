#ifndef _TAEOBJCOINNUM_H
#define _TAEOBJCOINNUM_H

#include "ui/TAeLayer.h"
struct DrawTask;

/**
 * @size 0x40
 */
class TAeObjCoinNum : public TAeObject {
public:
	TAeObjCoinNum(TObject* parent, void* data, void* assets);
	virtual ~TAeObjCoinNum();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void render(); // _1C

	void setCount(u32 count);
	void show(u32 count);
	void hide();

	DrawTask* mTask;  // _34
	TAeLayer* mLayer; // _38 made by show
	int mTimer;       // _3C name is ours
};

#include "stage/Stage.h"
extern "C" {
extern const char* gTAeObjCoinNumName;
}

#endif
