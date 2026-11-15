#include "object/ObjAction.h"
#include "engine/DrawTask.h"
#include "engine/CharaColli.h"
#include "object/TObjBlast.h"
#include "engine/Colli.h"
#include "camera/TCamera.h"
#include "core/PlacementNew.h"
struct CharaColli;

class TObjBlast;

typedef char TObjBlast_size_check[(sizeof(TObjBlast) == 0x58) ? 1 : -1];

const char* gTObjBlastName = "TObjBlast";

// 0x8002E3A4
TObjBlast::TObjBlast(TObject* parent, const Vec3f* pos, f32 radius, f32 grow, int power)
    : TObjAction(parent)
{
	setObjId(this, 0xFFFF);
	_00     = (u32)gTObjBlastName;
	mSize   = 0x58;
	mPos    = *pos;
	mRadius = radius;
	mGrow   = mRadius * grow;
	mPower  = power;
	mFrame  = 0;
	build();
}

// 0x8002E46C
TObjBlast::~TObjBlast()
{
	removeDrawTask(mDraw);
	colliFree((CharaColli*)mShape);
	releaseObjId(this);
}

// 0x8002E4E4
void TObjBlast::build()
{
	ColliShape shape;

	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	colliShapeInit(&shape);
	shape.mType = 0;
	shape.mSizeX = mRadius;
	shape.mFlags |= 8;
	mShape       = 0;
	mShape       = colliCreate(1, 1);
	colliCopyShapes(mShape, &shape);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 0x80, 0, 0xFF, 0);
	colliPlaceShapes(mShape, &mPos, &shape);
	mAct = 0;
}

// 0x8002E5A4
void TObjBlast::grow()
{
	ColliShape* shape;

	mFrame = mFrame + 1;
	if (mFrame == 30) {
		colliDisable(mShape);
	} else if (mFrame == 75) {
		mAct = 1;
		return;
	}
	mRadius = mRadius + mGrow;
	mGrow *= 0.85f;
	colliGetShape(mShape, 0)->mSizeX = mRadius;
}

// 0x8002E648
void TObjBlast::draw()
{
	switch (mAct) {
	case 0:
		grow();
		break;
	case 1:
		mFlags |= 1;
		break;
	}
}

// 0x8002E694
void TObjBlast::vfn_10()
{
}

// 0x8002E698
void TObjBlast::render()
{
}

// 0x8002E69C
int TObjBlast::vfn_48(TObjAction* other)
{
	if (other->getKind() == 1) {
		return 0;
	}
	return mPower;
}

// 0x8002E6EC
int TObjBlast::onHit(const TObjAction*, int, int* result)
{
	HitInfo* hit = (HitInfo*)result;

	resetHitInfo(hit);
	hit->mDamage = mPower;
	hit->mFlags.setBit(0xD);
	return 1;
}

// 0x8002E738
int TObjBlast::getKind()
{
	return 3;
}

// 0x8002E740
Vec3f* TObjBlast::getPos()
{
	return &mPos;
}
