#ifndef _OBJECT_TOBJWHOLEATTACK_H_
#define _OBJECT_TOBJWHOLEATTACK_H_
#include "object/ObjAction.h"
#include "stage/Stage.h"
struct TObjWholeAttack;
struct TObject;

#include "core/types.h"

struct Vec3f;

extern "C" {
void createWholeAttack(const Vec3f*);
}

class TObjWholeAttack : public TObjAction {
public:
	TObjWholeAttack(TObject* parent, const Vec3f* pos);
	virtual ~TObjWholeAttack();
	virtual void draw();
	virtual Vec3f* getPos();
	virtual int getKind();
	virtual int onHit(const TObjAction* other, int arg, int* result);

	void* mShape; // _30
	Vec3f mPos;   // _34
	u16 mTimer;   // _40
};

extern "C" {
extern u32 gTObjWholeAttackName;
}

#endif
