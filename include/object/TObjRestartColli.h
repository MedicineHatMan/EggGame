#ifndef _OBJECT_TOBJRESTARTCOLLI_H_
#define _OBJECT_TOBJRESTARTCOLLI_H_
#include "object/ObjAction.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct CharaColli;
struct TObjRestartColli;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjRestartColli(TObject*, SetDesc*);
}

class TObjRestartColli : public TObjAction {
public:
	TObjRestartColli(TObject* parent, const SetDesc* desc);
	virtual ~TObjRestartColli();

	virtual void draw();
	virtual Vec3f* getPos();
	virtual int getKind();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void* arg);

	inline void operator delete(void* mem);

	u16 mRestartId;      // _30
	CharaColli* mShape;  // _34
	SetLinkMember mLink; // _38
	Vec3f mPos;          // _44
};

extern "C" {
extern const char* gTObjRestartColliName;
}

#endif
