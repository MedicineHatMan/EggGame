#ifndef _OBJECT_TOBJEXPLODE_H_
#define _OBJECT_TOBJEXPLODE_H_
#include "Ninja/ninja.h"
#include "object/ObjAction.h"
#include "stage/Stage.h"
struct TObjExplode;

#include "core/types.h"

struct Vec3f;

extern "C" {
void spawnExplode(const Vec3f*, u16, int);
}

extern "C" {
void spawnChickenBomb(s16, const Vec3f*);
}

class TObjExplode : public TObjAction {
public:
	TObjExplode(TObject* parent, const Vec3f* pos, u16 id, u16 kind);
	virtual ~TObjExplode();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                                              // _0C
	virtual Vec3f* getPos();                                          // _20
	virtual int getKind();                                            // _2C
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58

	u16 mLife;    // _30
	u16 mKind;    // _32 the damage it deals
	void* mShape; // _34
	Vec3f mPos;   // _38
};

#endif
