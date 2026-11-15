#ifndef _OBJECT_GREEN_TOBJGREENSPORE_H_
#define _OBJECT_GREEN_TOBJGREENSPORE_H_
#include "stage/Stage.h"
struct TObjGreenSpore;

#include "core/types.h"

extern "C" {
int createTObjGreenSpore(s8*);
}

class TObjGreenSpore : public TObject {
public:
	TObjGreenSpore(TObject* parent, int arg);
	virtual ~TObjGreenSpore();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	void* mFx;       // _20
	u8 _24[4];
	f32 mMatrix[12]; // _28
};

extern "C" {
extern u32 gTObjGreenSporeName;
}

#endif
