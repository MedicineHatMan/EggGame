#ifndef _TOBJTITLEFLOWER_H
#define _TOBJTITLEFLOWER_H
struct SetDesc;
struct TObject;

#include "engine/DrawTask.h"
#include "engine/MotionPlayer.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"

extern "C" void objHeapFree(void*, void*);

/**
 * A title-screen flower. Set float 0 is the scale, clamped to -1..10 and then added to 1.
 *
 * @size 0xC8
 */
class TObjTitleFlower : public TSetActor {
public:
	TObjTitleFlower(TObject* parent, SetDesc* desc);
	virtual ~TObjTitleFlower();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void*);           // _34

	DrawTask* mDraw;      // _64
	ObjModel mModel;      // _68
	MotionPlayer mMotion; // _88
	f32 mScale;           // _C4 set float 0 plus 1
};

extern "C" {
void initTObjTitleFlower();
void termTObjTitleFlower();
}

extern "C" {
TObject* createTObjTitleFlower(TObject*, SetDesc*);
}

extern "C" {
extern const char* gTObjTitleFlowerName;
extern const MotionEntry sTitleFlowerMotions[];
}

#endif
