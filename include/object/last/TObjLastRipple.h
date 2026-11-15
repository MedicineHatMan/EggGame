#ifndef _OBJECT_LAST_TOBJLASTRIPPLE_H_
#define _OBJECT_LAST_TOBJLASTRIPPLE_H_
#include "engine/CharaColli.h"
#include "object/ObjAction.h"
#include "stage/Stage.h"
struct TObjLastRipple;
struct TObject;
struct Vec3f;

#include "core/types.h"

extern "C" {
TObjLastRipple* createTObjLastRipple(TObject*, const Vec3f*);
}

class TObjLastRipple : public TObjAction {
public:
	TObjLastRipple(TObject* parent, const Vec3f* pos);
	virtual ~TObjLastRipple();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual int getKind();

	CharaColli* mShape;  // _30
	Vec3f mPos;    // _34
	f32 mRadius;   // _40
	u16 mLife;     // _44
	u16 _46;
};

#endif
