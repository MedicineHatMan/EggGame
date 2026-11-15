#include "stage/SetManager.h"
#include "stage/SetSlot.h"
#include "core/dolphin.h"
#include "object/ObjAction.h"

inline void TSetObj::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}

// 0x80018704
TSetObj::TSetObj(TObject* parent, SetDesc* desc)
    : TObjAction(parent)
{
	setObjId(this, desc->mObjId);
	mLink.get()->attach(desc);
	initFromDesc(desc);
	_80 = 0;
	readHome(desc);
}

// 0x80018798
TSetObj::~TSetObj()
{
	notifySetLinkDeath(mLink.get(), this);
	releaseObjId(this);
}

void TSetObj::readDesc(SetDesc* desc)
{
	desc->getPos(&mTrans.mPos);
	desc->getRot(&mTrans.mRot);
}

void TSetObj::readHome(SetDesc* desc)
{
	desc->getPos(&mHome);
}

void TSetObj::initFromDesc(SetDesc* desc)
{
	readDesc(desc);
}

int TSetObj::getKind()
{
	return 3;
}

int TSetObj::getRotY()
{
	return *(int*)((u8*)this + 0x6C);
}

Vec3f* TSetObj::getPos()
{
	return &mTrans.mPos;
}

void TSetObj::setPos(const Vec3f* pos)
{
	mTrans.mPos.x = pos->x;
	mTrans.mPos.y = pos->y;
	mTrans.mPos.z = pos->z;
}

SetLink* TSetObj::getLink()
{
	return mLink.get();
}

void TSetObj::vfn_78(SetDesc*)
{
}

void TSetObj::debugPrint(void* arg)
{
	vfn_78((SetDesc*)arg);
}
