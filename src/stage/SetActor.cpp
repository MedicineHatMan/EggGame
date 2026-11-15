#include "stage/SetActor.h"
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "core/PlacementNew.h"
#include "stage/SetSlot.h"

typedef char TSetActor_size_check[(sizeof(TSetActor) == 0x64) ? 1 : -1];

inline void TSetActor::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}

// 0x8004D9D0
TSetActor::TSetActor(TObject* parent, SetDesc* desc)
    : TObjAction(parent)
{
	setObjId(this, desc->mObjId);
	initFromDesc(desc);
	mLink.get()->attach(desc);
	mShape = 0;
}

// 0x8004DA50
// No record mObjId is 0xFFFF from TObjAction and setObjId reads that as "assign one"
TSetActor::TSetActor(TObject* parent)
    : TObjAction(parent)
{
	setObjId(this, (s16)mObjId);
	mShape = 0;
}

// 0x8004DAA8
TSetActor::~TSetActor()
{
	notifySetLinkDeath(mLink.get(), this);
	releaseObjId(this);
}

// 0x8004DB28
// Subclasses call this first in their own version
void TSetActor::initFromDesc(SetDesc* desc)
{
	desc->getPos(&mHome);
	desc->getPos(&mPos);
	desc->getRot(&mRot);
}

// 0x8004DB9C
void TSetActor::debugPrint(void*)
{
}
