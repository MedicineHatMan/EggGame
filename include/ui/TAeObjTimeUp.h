#ifndef _TAEOBJTIMEUP_H
#define _TAEOBJTIMEUP_H

#include "ui/TAeLayer.h"
struct DrawTask;

/**
 * @size 0x3C
 */
class TAeObjTimeUp : public TAeObject {
public:
	TAeObjTimeUp(TObject* parent, void* data, void* assets);
	virtual ~TAeObjTimeUp();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void render(); // _1C

	TAeLayer* mLayer; // _34
	DrawTask* mTask;  // _38
};

extern "C" {
extern const char* gTAeObjTimeUpName;
extern u32 sAeTimeUpArc;
extern u32 sAeTimeUpData;
extern u32 sAeTimeUpTex;
}

TAeObjTimeUp* createTimeUp();
int loadTimeUp();

#endif
