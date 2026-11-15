#include "object/TObjFallThing.h"
#include "object/red/TObjRedDHeadInvokeArea.h"
#include "engine/DrawTask.h"
#include "object/ObjScript.h"
#include "object/red/TObjRedDinoHead.h"
#include "engine/CharaColli.h"
#include "enemy/ObjEnemy.h"
#include "object/ObjArchive.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "engine/Colli.h"
#include "core/Mem.h"
#include "core/SdkCalls.h"
#include "Ninja/ninja.h"
#include "object/red/AreaListImpl.h"
struct ColliShape;
struct CharaColli;
struct ColliContact;

extern "C" {

// 0x802C4048
const char* gTObjRedDHeadInvokeAreaName = "TObjRedDHeadInvokeArea";
}

typedef char TObjRedDHeadInvokeArea_size_check[(sizeof(TObjRedDHeadInvokeArea) == 0x90) ? 1 : -1];

// 0x800C7034
// is this file's __sinit
AreaList gRedAreaList;

static void linkArea(AreaList* list, RedAreaLink* link);
static void unlinkArea(AreaList* list, RedAreaLink* link);

// 0x800C66F8
extern "C" TObject* createTObjRedDHeadInvokeArea(TObject* parent, SetDesc* desc)
{
	return new TObjRedDHeadInvokeArea(parent, desc);
}

// 0x800C674C
TObjRedDHeadInvokeArea::TObjRedDHeadInvokeArea(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	mLink.mNext  = 0;
	mLink.mPrev  = 0;
	mLink.mList  = 0;
	_00          = (u32)gTObjRedDHeadInvokeAreaName;
	mSize        = 0x90;
	mLink.mOwner = this;
	linkArea(&gRedAreaList, &mLink);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	makeColli();
	initFromDesc(desc);
	setup();
}

// 0x800C6810
TObjRedDHeadInvokeArea::~TObjRedDHeadInvokeArea()
{
	freeShape();
	removeDrawTask(mDrawTask);
	unlinkArea(&gRedAreaList, &mLink);
}

// 0x800C688C
void TObjRedDHeadInvokeArea::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mGroupId = (s16)desc->getInt(0);

	mColType = (s16)desc->getInt(1);
	mColType = mColType < 0 ? 0 : (mColType > 1 ? 1 : mColType);
	mRadius  = desc->getFloat(0);
	mHeight  = desc->getFloat(1);
	makeShape();
}

// 0x802C404C
// the name of each collision type for debugPrint
__declspec(section ".sdata") char* sRedDHeadColTypeNames[] = {
	"CYLINDER",
	"SPHERE",
	NULL,
};

// 0x800C695C
void TObjRedDHeadInvokeArea::debugPrint(void*)
{
	njPrint(0xA0013, ":GROUP ID");
	njPrint(0xA0014, ":COL_TYPE=%s", sRedDHeadColTypeNames[mColType]);
	njPrint(0xA0017, ":COL_RAD");
	njPrint(0xA0018, ":COL_HEIGHT");
}

// 0x800C69EC
static inline s16 headGroup(void* head)
{
	return ((TObjRedDinoHead*)head)->mGroupId;
}

void TObjRedDHeadInvokeArea::setup()
{
	mInside = 0;
	mTimer  = 0;
	mClosed = 0;
	for (RedHeadLink* link = gRedHeadList.mTail; link != 0; link = link->mNext) {
		if (mGroupId == headGroup(link->mOwner) && ((TObjRedDinoHead*)link->mOwner)->switchIsClear() == 0) {
			shut();
		}
	}
}

// 0x800C6A68
void TObjRedDHeadInvokeArea::draw()
{
	if (isInRange(this, &mPos, 900.0f) == 0) {
		mFlags |= 1;
		return;
	}
	if (objDebugEnabled(this) != 0) {
		placeShape();
	}
	if (mClosed != 0) {
		return;
	}

	ColliContact* touch;
	int inside = 0;
	for (touch = mShape->mContacts; touch != 0; touch = touch->mNext) {
		ColliShape* shape = colliGetShape(touch->mOther, touch->mOtherShape);
		if ((shape->mFlags & 0x10) == 0 && findObjById(colliContactOwner(touch)) != 0) {
			inside = 1;
		}
	}
	if (mInside != inside) {
		RedHeadList* list = &gRedHeadList;
		if (inside != 0) {
			for (RedHeadLink* link = list->mTail; link != 0; link = link->mNext) {
				if (mGroupId == headGroup(link->mOwner)) {
					((TObjRedDinoHead*)link->mOwner)->open();
				}
			}
		} else {
			for (RedHeadLink* link = list->mTail; link != 0; link = link->mNext) {
				if (mGroupId == headGroup(link->mOwner)) {
					((TObjRedDinoHead*)link->mOwner)->close();
				}
			}
		}
		mInside = inside;
	}
	if (inside != 0 && (mTimer & 0x2F) == 0) {
		drop();
	}
	mTimer += 1;
}

// 0x800C6BCC
void TObjRedDHeadInvokeArea::drop()
{
	static FallSpawn spawn;

	spawn.reset();
	spawn.mType    = FALL_SHATTER;
	spawn.mModel   = (NJS_OBJECT*)getGeObjModels()[9];
	spawn.mTexList = getGeObjTextures()[9];
	spawn.mPos     = mPos;
	spawn.mPos.y += 200.0f;
	spawn.mPos.x += mRadius * (2.0f * (3.0517578e-05f * (f32)rand() - 0.5f));
	spawn.mPos.z += mRadius * (2.0f * (3.0517578e-05f * (f32)rand() - 0.5f));
	spawn.mRadius       = 4.0f;
	spawn.mDamage       = 2;
	spawn.mParticleBank = 1;
	spawn.mParticleId   = 0;
	spawn.mPieceModels  = (NJS_OBJECT**)getGeObjModels();
	spawn.mPieceFirst   = 10;
	spawn.mPieceCount   = 9;
	spawn.mPieceTexList = getGeObjTextures()[9];
	spawn.mLandSe       = 0x220;
	spawnFallThing(this, &spawn);
}

// 0x800C6D9C
void TObjRedDHeadInvokeArea::shut()
{
	colliDisable(mShape);
	mClosed = 1;
}

// 0x800C6DD4
void TObjRedDHeadInvokeArea::makeColli()
{
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 0xFF, 0xFF, 0xFE);
}

// 0x800C6E30
void TObjRedDHeadInvokeArea::makeShape()
{
	ColliShape shape;

	colliShapeInit(&shape);
	shape.mPos   = mPos;
	shape.mType  = (mColType != 0) ? COLLISHAPE_SPHERE : COLLISHAPE_CYLINDER;
	shape.mSizeX = mRadius;
	if (shape.mType == COLLISHAPE_CYLINDER) {
		shape.mSizeY = 0.5f * mHeight;
		shape.mPos.y += shape.mSizeY;
	}
	shape.mFlags |= 0x10;
	colliCopyShapes(mShape, &shape);
}

// 0x800C6ED4
void TObjRedDHeadInvokeArea::placeShape()
{
	ColliShape shape;

	shape.mPos.x = 0.0f;
	shape.mPos.y = 0.5f * mHeight;
	shape.mPos.z = shape.mPos.x;
	colliPlaceShapes(mShape, &mPos, &shape);
}

// 0x800C6F1C
void TObjRedDHeadInvokeArea::freeShape()
{
	colliRetire(mShape);
	mShape = 0;
}

// 0x800C6F54
void TObjRedDHeadInvokeArea::vfn_10()
{
}

// 0x800C6F58
void TObjRedDHeadInvokeArea::render()
{
	if (objDebugEnabled(this) != 0) {
		colliDrawShapes(mShape);
	}
}

// 0x800C6F94
static void linkArea(AreaList* list, RedAreaLink* link)
{
	if (list->mTail == 0) {
		list->mHead = link;
		list->mTail = link;
	} else {
		link->mPrev        = list->mHead;
		list->mHead->mNext = link;
		list->mHead        = link;
	}
	link->mList = list;
	list->mCount += 1;
}

// 0x800C6FD4
static void unlinkArea(AreaList* list, RedAreaLink* link)
{
	if (link->mPrev != 0) {
		link->mPrev->mNext = link->mNext;
	} else {
		list->mTail = link->mNext;
	}
	if (link->mNext != 0) {
		link->mNext->mPrev = link->mPrev;
	} else {
		list->mHead = link->mPrev;
	}
	link->mNext = 0;
	link->mPrev = 0;
	link->mList = 0;
	list->mCount -= 1;
}
