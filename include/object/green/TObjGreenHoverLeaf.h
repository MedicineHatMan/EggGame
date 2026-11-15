#ifndef _OBJECT_GREEN_TOBJGREENHOVERLEAF_H_
#define _OBJECT_GREEN_TOBJGREENHOVERLEAF_H_
#include "stage/Stage.h"
struct TObjGreenHoverLeaf;

#include "core/types.h"

extern "C" {
int createTObjGreenHoverLeaf(s8*);
}

class TObjGreenHoverLeaf : public TObject {
public:
	TObjGreenHoverLeaf(TObject* parent, int arg);
	virtual ~TObjGreenHoverLeaf();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	void* mFx; // _20
	u8 _24[4];
	f32 mMatrix[12]; // _28
};

extern "C" {
extern const char* gTObjGreenHoverLeafName;
}

#endif
