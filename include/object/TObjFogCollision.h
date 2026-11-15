#ifndef _OBJECT_TOBJFOGCOLLISION_H_
#define _OBJECT_TOBJFOGCOLLISION_H_
#include "Ninja/ninja.h"
#include "object/ObjAction.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjFogCollision;
struct TObject;

#include "core/types.h"

class SetDesc;

extern "C" {
int fogCollisionGetDefault(SetDesc*);
void prepareTObjFogCollision(SetDesc*);
}

extern "C" {
TObject* createTObjFogCollision(TObject*, SetDesc*);
TObject* createTObjFogCollision2C(TObject*, SetDesc*);
}

/**
 * @size 0xB0
 */
class TObjFogCollision : public TObjAction {
public:
	TObjFogCollision(TObject* parent, const SetDesc* desc, f32 value);
	virtual ~TObjFogCollision();

	virtual void draw();                                          // _0C
	virtual int getKind() { return 3; }                           // _2C
	virtual Vec3f* getPos() { return &mXform.mPos; }              // _20
	virtual void initFromDesc(SetDesc* desc);                     // _30
	virtual void debugPrint(void* arg);                           // _34
	virtual void setPos(const Vec3f* pos) { mXform.mPos = *pos; } // _6C

	void setShape(int shape, f32 width, f32 height, f32 depth);

	int mShape;          // _30 a FogShape
	Vec3f mCenter;       // _34 mid-height
	Vec3f mSize3;        // _40 radius or width half the height depth
	SetTransform mXform; // _4C
	SetLinkMember mLink; // _90
	int mFogId;          // _9C
	f32 mRate;           // _A0
	int mOrder;          // _A4
	f32 mFar;            // _A8 the value create passes
	int mDefault;        // _AC a FogDefault
};

#endif
