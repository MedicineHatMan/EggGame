#include "stage/SetManager.h"
#include "engine/CharaColli.h"
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "object/TObjPlayer.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "engine/Colli.h"
#include "stage/SetSlot.h"
#include "object/TObjRestartColli.h"
struct CharaColli;

extern "C" {
ColliShape sRestartColliShape[] = {
	{ { 0.0f, 5.0f, 0.0f }, 0x0, 0, 0x0, 5.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

inline void TObjRestartColli::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}

const char* gTObjRestartColliName = "TObjRestartColli";

// 0x80066648
TObjRestartColli::TObjRestartColli(TObject* parent, const SetDesc* desc)
    : TObjAction(parent)
{
	_00 = (u32)gTObjRestartColliName;
	mSize = 0x50;
	setObjId(this, desc->mObjId);
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 0, 255, 254);
	colliCopyShapes(mShape, sRestartColliShape);
	mLink.get()->attach((SetDesc*)desc);
	initFromDesc((SetDesc*)desc);
}

// 0x80066714
TObjRestartColli::~TObjRestartColli()
{
	colliFree(mShape);
	notifySetLinkDeath(mLink.get(), this);
	unregisterId();
}

// 0x8006679C
extern "C" TObject* createTObjRestartColli(TObject* parent, SetDesc* desc)
{
	return new TObjRestartColli(parent, desc);
}

// 0x800667F0
// Int 0 is the restart point int 1 the shape 0 to 2 floats 0 to 2 its size
void TObjRestartColli::initFromDesc(SetDesc* desc)
{
	SetRot rot;

	mRestartId        = desc->getInt(0);
	ColliShape* shape = colliGetShape(mShape, 0);
	desc->getPos(&mPos);
	colliShapeSetPos(shape, &mPos);
	desc->getRot(&rot);
	shape->_0C   = 0;
	shape->mRotY = rot.y;
	shape->_14   = 0;
	u32 type     = desc->getInt(1);
	if ((int)type < 0 || (int)type > 2) {
		type = 0;
	}
	shape->mType  = type;
	shape->mSizeX = desc->getFloat(0);
	shape->mSizeY = desc->getFloat(1);
	shape->mSizeZ = desc->getFloat(2);
}

// 0x80066920
void TObjRestartColli::draw()
{
	if (!isInRange((void*)this, (const Vec3f*)getPos(), 500.0f)) {
		mFlags |= 1;
		return;
	}
	for (ColliContact* contact = mShape->mContacts; contact != 0; contact = contact->mNext) {
		TObjAction* player = (TObjAction*)findPlayer(colliContactOwner(contact));
		if (player != 0) {
			restartPointUse(player->mObjId, mRestartId);
		}
	}
}

// 0x800669B8
void TObjRestartColli::debugPrint(void* arg)
{
}

// 0x800669BC
Vec3f* TObjRestartColli::getPos()
{
	return &mPos;
}

// 0x800669C4
int TObjRestartColli::getKind()
{
	return 3;
}
