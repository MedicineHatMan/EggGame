#ifndef _OBJECT_TOBJSCENELIGHTCOLLISION_H_
#define _OBJECT_TOBJSCENELIGHTCOLLISION_H_
#include "object/ObjAction.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjSceneLightCollision;
struct TObject;

#include "core/types.h"

class SetDesc;
struct Vec3f;

extern "C" {
void lightAreaExit();
int lightAreaFind(Vec3f*);
void lightAreaInit();
void prepareTObjSceneLightCollision(SetDesc*);
}

extern "C" {
TObject* createTObjSceneLightCollision(TObject*, SetDesc*);
}

class TObjSceneLightCollision : public TObjAction {
public:
	TObjSceneLightCollision(TObject* parent, const SetDesc* desc);

	virtual void initFromDesc(SetDesc* desc);

	virtual ~TObjSceneLightCollision();
	virtual void draw();            // _0C goes away out of range
	virtual Vec3f* getPos();        // _20
	virtual int getKind();          // _2C 3
	virtual void debugPrint(void*); // _34

	SetLinkMember mLink; // _30
	Vec3f mPos;          // _3C
};

extern "C" {
extern const char* gTObjSceneLightCollisionName;
}

#endif
