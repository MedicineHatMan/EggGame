#include "stage/SetManager.h"
#include "object/TObjPlayer.h"
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "stage/SetSlot.h"

#include "object/TObjRestartPoint.h"

inline void TObjRestartPoint::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}

const char* gTObjRestartPointName = "TObjRestartPoint";

// 0x800669CC
TObjRestartPoint::TObjRestartPoint(TObject* parent, const SetDesc* desc)
    : TObjAction(parent)
{
	_00 = (u32)gTObjRestartPointName;
	mSize = 0x50;
	setObjId(this, desc->mObjId);
	mLink.get()->attach((SetDesc*)desc);
	initFromDesc((SetDesc*)desc);
	restartPointSet(mPointId, &mPos, mRotY);
}

// 0x80066A64
TObjRestartPoint::~TObjRestartPoint()
{
	notifySetLinkDeath(mLink.get(), this);
	unregisterId();
}

// 0x80066AE4
// Position at +0x34 the rotation's Y at +0x40 and the first int as a halfword at +0x30
void TObjRestartPoint::initFromDesc(SetDesc* desc)
{
	SetRot rot;
	desc->getPos(&mPos);
	desc->getRot(&rot);
	mRotY = rot.y;
	mPointId = (u16)desc->getInt(0);
}

// 0x80066B64
extern "C" TObject* createTObjRestartPoint(TObject* parent, SetDesc* desc)
{
	return new TObjRestartPoint(parent, desc);
}
