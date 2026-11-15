#ifndef _UI_TSUPEREMBLEM_H_
#define _UI_TSUPEREMBLEM_H_
#include "stage/Stage.h"
struct DrawTask;
struct TSuperEmblem;

#include "core/types.h"

extern "C" {
void superEmblemTevStage(u8);
void superEmblemTexGen(u8);
}

/**
 * @size 0x88
 */
class TSuperEmblem : public TObject {
public:
	TSuperEmblem(TObject* parent, const f32* matrix);
	virtual ~TSuperEmblem();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void render();

	f32 mMatrix[12]; // _20
	f32 mBase[12];   // _50
	int mTime;       // _80
	DrawTask* mTask;     // _84
};

extern "C" {
extern const char* gTSuperEmblemName;
}

#endif
