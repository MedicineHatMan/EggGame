#ifndef _CORE_TSCENEMANAGER_H_
#define _CORE_TSCENEMANAGER_H_
#include "stage/Stage.h"
struct TSceneManager;

#include "core/types.h"

struct SceneRecord;

extern "C" {
void sceneDrawPass1(void*);
void sceneDrawPass2(void*);
void setSceneRecord(SceneRecord*);
}

extern "C" {
SceneRecord* getSceneRecord();
}

/**
 * @size 0x2C
 */
class TSceneManager : public TObject {
public:
	TSceneManager(TObject* parent);
	virtual ~TSceneManager();

	inline void* operator new(unsigned long size, void* pool) { return objHeapAlloc(pool, size); }
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();

	SceneRecord* mRecord; // _20
	void* mTask1;   // _24
	void* mTask2;   // _28
};

extern "C" {
extern const char* gTSceneManagerName;
extern SceneRecord* sSceneRecord;
}

#endif
