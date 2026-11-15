#ifndef _STAGE_SETSPAWNER_H_
#define _STAGE_SETSPAWNER_H_
#include "stage/Stage.h"
struct TSetGenerator;

#include "core/types.h"

extern "C" {
void createSetSpawner();
void setSpawnerNone();
void setSpawnerOff();
}

extern "C" {
int setSpawnerCamera();
int setSpawnerMode(int);
}

class TSetGenerator : public TObject {
public:
	TSetGenerator(TObject* parent);
	virtual ~TSetGenerator();
	virtual void draw();

	inline void* operator new(unsigned long size, void* pool) { return objHeapAlloc(pool, size); }

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	int mCursors[3]; // _20 where each kind's scan stopped
	int mMode;       // _2C see SetCentreMode
	int mFullScans;  // _30 frames left that scan the whole of each file
};

extern "C" {
extern void* gTSetGeneratorName;
}

#endif
