#include "stage/SetManager.h"
#include "egg/EggCreature.h"

SetLinkMember::SetLinkMember()
{
	mKind  = -1;
	mIndex = 0;
	mMode  = 0;
	mTimer = 0;
}

SetLinkMember::~SetLinkMember() { }

// 0x80061244
void SetLink::attach(const SetDesc* desc)
{
	mKind  = ((SetDesc*)desc)->getKind();
	mIndex = desc->mIndex;
	mMode  = 0;
	mTimer = 0;
	attachSetLink(this);
}

void SetLink::reset()
{
	mKind  = -1;
	mIndex = 0;
}

void* SetLink::getData()
{
	SetSlot* slot = getSetSlot(mKind, mIndex);
	if (slot != 0) {
		return slot->getData();
	}
	return 0;
}

void SetLink::setData(void* data)
{
	SetSlot* slot = getSetSlot(mKind, mIndex);
	if (slot != 0) {
		slot->setData(data);
	}
}

extern "C" void notifySetLinkDeath(SetLink* link, void* obj)
{
	onSetDeath(link->mKind, obj, link);
}

extern "C" void setLinkFree(SetLink* link)
{
	link->mMode = 0;
}

extern "C" void setLinkDone(SetLink* link)
{
	link->mMode = 1;
}

extern "C" void setLinkTimed(SetLink* link, int timer)
{
	link->mMode  = 2;
	link->mTimer = timer;
}

extern "C" void setLinkForever(SetLink* link)
{
	link->mMode = 3;
}

void SetSlot::init()
{
	mState = 0;
	mObjId = 0xFFFF;
	mTimer = 0;
	mObj   = 0;
	mData  = 0;
	_10    = 0;
	mLink  = 0;
}

void SetSlot::clear()
{
	killObject();
	if (mState != 5) {
		mState = 0;
	}
	mObjId = 0xFFFF;
	mObj   = 0;
	mTimer = 0;
	mLink  = 0;
	_10    = 0;
}

void SetSlot::reset()
{
	clear();
	mState = 0;
	mData  = 0;
}

void SetSlot::setSpawned(void* obj)
{
	mState = 2;
	mObjId = *(u16*)((u8*)obj + 0x20);
	mObj   = obj;
}

// 0x8006146C
int SetSlot::canSpawn(int tick)
{
	if (mState == 3) {
		u32 left = mTimer | 0x10000;
		left -= tick;
		left %= 0x10000;
		if ((int)left >= 0x8000) {
			mState = 0;
			mTimer = 0;
			return 1;
		}
	} else if (mState == 0) {
		return 1;
	}
	return 0;
}

int SetSlot::hasNoData()
{
	return _10 == 0;
}

void* SetSlot::getData()
{
	return mData;
}

void SetSlot::setData(void* data)
{
	mData = data;
}

void SetSlot::setDead(void* obj, int timer)
{
	mObj   = 0;
	mTimer = timer;
	mLink  = 0;
}

void SetSlot::setDeadFree(void* obj)
{
	mState = 0;
	setDead(obj, 0);
}

void SetSlot::setDeadDone(void* obj)
{
	mState = 4;
	_10    = 1;
	setDead(obj, 0);
}

void SetSlot::setDeadTimed(void* obj, u16 timer)
{
	mState = 3;
	_10    = 1;
	setDead(obj, (s16)timer);
}

void SetSlot::setDeadForever(void* obj)
{
	mState = 5;
	_10    = 1;
	setDead(obj, 0);
}

void SetSlot::setBlocked()
{
	if (mState == 0 || mState == 4) {
		mState = 1;
	}
}

int SetSlot::takeBlocked()
{
	if (mState == 1) {
		mState = 0;
		return 1;
	}
	return 0;
}

int SetSlot::takeHidden()
{
	if (mState == 4 || mState == 5) {
		mState = 1;
		return 1;
	}
	return 0;
}

int SetSlot::takeAny()
{
	if (mState == 1 || mState == 4 || mState == 5) {
		mState = 0;
		return 1;
	}
	return 0;
}

int SetSlot::checkHidden()
{
	if (mState == 4 || mState == 5) {
		return 1;
	}
	if (mState == 3) {
		mState = 4;
		return 1;
	}
	return 0;
}

void SetSlot::setFinished()
{
	if (mState == 4) {
		mState = 5;
	}
}

void SetSlot::setLinkIndex(int index)
{
	if (mLink != 0) {
		mLink->mIndex = index;
	}
}

void SetSlot::killObject()
{
	if (mState == 2) {
		TObject* obj = findObjById(mObjId);
		if (obj != 0 && obj == mObj) {
			if (mLink != 0) {
				mLink->reset();
			}
			mState = 0;
			obj->mFlags |= 1;
		}
	}
}
