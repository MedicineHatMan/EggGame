#ifndef _OBJECT_TOBJRESTARTPOINT_H_
#define _OBJECT_TOBJRESTARTPOINT_H_
#include "object/ObjAction.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjRestartPoint;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjRestartPoint(TObject*, SetDesc*);
}

class TObjRestartPoint : public TObjAction {
public:
	TObjRestartPoint(TObject* parent, const SetDesc* desc);
	virtual ~TObjRestartPoint();
	virtual int getKind() { return 3; } // 0x80066BB8

	virtual void initFromDesc(SetDesc* desc);

	inline void operator delete(void* mem);
	u16 mPointId;        // _30
	Vec3f mPos;          // _34
	int mRotY;           // _40
	SetLinkMember mLink; // _44
};

extern "C" {
extern const char* gTObjRestartPointName;
}

#endif
