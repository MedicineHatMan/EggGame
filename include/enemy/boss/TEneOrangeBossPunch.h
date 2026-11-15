#ifndef _ENEMY_BOSS_TENEORANGEBOSSPUNCH_H
#define _ENEMY_BOSS_TENEORANGEBOSSPUNCH_H
#include "object/ObjAction.h"
#include "stage/Stage.h"
struct TEneOrangeBossPunch;

#include "core/types.h"

class TEneOrangeBossPunch : public TObjAction {
public:
	TEneOrangeBossPunch(TObject* parent, const Vec3f* from, const Vec3f* to);
	virtual ~TEneOrangeBossPunch();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                                              // _0C
	virtual Vec3f* getPos();                                          // _20
	virtual int getKind();                                            // _2C
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58

	Vec3f mFrom;  // _30
	Vec3f mTo;    // _3C
	int mLife;    // _48
	void* mShape; // _4C
};

#endif
