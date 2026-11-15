#include "stage/SetManager.h"
#include "object/TObjFogCollision.h"
#include "engine/TFogCtrl.h"
#include "camera/TCameraCtrl.h"
#include "engine/GroundQuery.h"
#include "camera/TCamera.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "object/ObjAction.h"
#include "stage/SetSlot.h"
#include "io/NinjaFile.h"
#include "core/TMainTask.h"
#include "Ninja/ninja.h"
struct SetLink;

// TObjFogCollision Classes 0x1 and 0x2C share the constructor
// 0x1 passes 1050 and has a prepare
// 0x8004F640-0x8004FDDC Vtable 0x80255BB0
// The record int 0 the fog id int 1 the shape 0 cylinder 1 box int 2 the default 0 off 1 on by day 2 on by night int 3
// the order

const char* gTObjFogCollisionName = "TObjFogCollision"; // what the object list prints

enum FogShape {
	FOGSHAPE_Cylinder = 0,
	FOGSHAPE_Box      = 1,
};

enum FogDefault {
	FOGDEFAULT_Off     = 0,
	FOGDEFAULT_OnDay   = 1,
	FOGDEFAULT_OnNight = 2,
};

typedef char TObjFogCollision_size_check[(sizeof(TObjFogCollision) == 0xB0) ? 1 : -1];

// 0x8004F640
// class 0x1
extern "C" TObject* createTObjFogCollision(TObject* parent, SetDesc* desc)
{
	parent = gTL_TIMESTOP;
	return new TObjFogCollision(parent, desc, 1050.0f);
}

// 0x8004F698
// class 0x2C
extern "C" TObject* createTObjFogCollision2C(TObject* parent, SetDesc* desc)
{
	parent = gTL_TIMESTOP;
	return new TObjFogCollision(parent, desc, 100050.0f);
}

// 0x8004F6F0
// On by day stores the record's first int at gFogMain on by night at gFogOther
extern "C" void prepareTObjFogCollision(SetDesc* desc)
{
	int kind = fogCollisionGetDefault(desc);
	if (kind == FOGDEFAULT_Off) {
		return;
	}
	int value = desc->getInt(0);
	if (kind == FOGDEFAULT_OnDay) {
		fogSetDefaultMain(value);
	} else if (kind == FOGDEFAULT_OnNight) {
		fogSetDefaultOther(value);
	}
}

// 0x8004F758
TObjFogCollision::TObjFogCollision(TObject* parent, const SetDesc* desc, f32 value)
    : TObjAction(parent)
{
	_00   = (u32)gTObjFogCollisionName;
	mSize = 0xB0;
	setObjId(this, desc->mObjId);
	mLink.get()->attach((SetDesc*)desc);
	mFar     = value;
	mShape   = FOGSHAPE_Cylinder;
	mSize3.x = 20.0f;
	mSize3.y = 20.0f;
	mSize3.z = 20.0f;
	initFromDesc((SetDesc*)desc);
}

// 0x8004F818
TObjFogCollision::~TObjFogCollision()
{
	notifySetLinkDeath((SetLink*)&mLink, (void*)this);
	releaseObjId(this);
}

// 0x8004F8C0
void TObjFogCollision::initFromDesc(SetDesc* desc)
{
	desc->getPos(&mXform.mPos);
	desc->getRot(&mXform.mRot);
	mDefault   = fogCollisionGetDefault(desc);
	mFogId     = desc->getInt(0);
	mRate      = desc->getFloat(0);
	mOrder     = desc->getInt(3);
	int shape  = desc->getInt(1);
	f32 width  = desc->getFloat(1);
	f32 height = desc->getFloat(2);
	f32 depth  = desc->getFloat(3);
	setShape(shape, width, height, depth);
}

// 0x8004FA18
// The record's third int kept to 0 1 or 2
extern "C" int fogCollisionGetDefault(SetDesc* desc)
{
	int kind = desc->getInt(2);
	if (kind < FOGDEFAULT_Off) {
		return FOGDEFAULT_Off;
	}
	return kind > FOGDEFAULT_OnNight ? FOGDEFAULT_OnNight : kind;
}

// 0x8004FA6C
// Nothing at least 20 across
void TObjFogCollision::setShape(int shape, f32 width, f32 height, f32 depth)
{
	if (mDefault != FOGDEFAULT_Off) {
		return;
	}
	int kind;
	if (shape < FOGSHAPE_Cylinder) {
		kind = FOGSHAPE_Cylinder;
	} else {
		kind = shape > FOGSHAPE_Box ? FOGSHAPE_Box : shape;
	}
	f32 w = width < 20.0f ? 20.0f : width;
	if (height < 20.0f) {
		height = 20.0f;
	}
	f32 d  = depth < 20.0f ? 20.0f : depth;
	mShape = kind;
	height *= 0.5f;
	mCenter = mXform.mPos;
	mCenter.y += height;
	mSize3.x = w;
	mSize3.y = height;
	mSize3.z = d;
}

// 0x8004FB10
void TObjFogCollision::draw()
{
	if (debugInfoActive()) {
		((void (*)(int, int, f32, int, int))fogRequest)(0, mFogId, mRate, 1, mOrder);
	}
	if (!isInRange(this, &mXform.mPos, mFar)) {
		mFlags |= 1;
		return;
	}
	if (mDefault != FOGDEFAULT_Off) {
		return;
	}
	for (int view = 0; view < 4; view++) {
		Vec3f cam;
		camGetAt(view, &cam);
		cam.y += 10.0f;
		int inside;
		if (mShape == FOGSHAPE_Cylinder) {
			inside = colliCylinderContains(&cam, &mCenter, mSize3.x, mSize3.y);
		} else {
			inside = colliBoxContains(&cam, &mCenter, mXform.mRot.y, mSize3.x, mSize3.y, mSize3.z);
		}
		if (inside) {
			((void (*)(int, int, f32, int, int))fogRequest)(view, mFogId, mRate, 1, mOrder);
		}
	}
}

// 0x8004FC2C
void TObjFogCollision::debugPrint(void* arg)
{
	njPrintC(0xA0013, ":PARAM IDX");
	njPrintC(0xA0014, ":COL TYPE");
	switch (mDefault) {
	case FOGDEFAULT_Off:
		njPrintC(0xA0015, ":DEFAULT OFF");
		break;
	case FOGDEFAULT_OnDay:
		njPrintC(0xA0015, ":DEFAULT ON DAY");
		break;
	case FOGDEFAULT_OnNight:
		njPrintC(0xA0015, ":DEFAULT ON NIGHT");
		break;
	}
	njPrintC(0xA0016, ":LEVEL");
	njPrint(0xA0017, ":LINK   %7.2f", mRate);
	f32 width  = mSize3.x;
	f32 height = mSize3.y;
	f32 depth  = mSize3.z;
	if (mShape == FOGSHAPE_Cylinder) {
		njPrint(0xA0018, ":RAD    %7.2f", width);
		njPrint(0xA0019, ":HEIGHT %7.2f", height);
	} else {
		njPrint(0xA0018, ":WIDTH  %7.2f", width);
		njPrint(0xA0019, ":HEIGHT %7.2f", height);
		njPrint(0xA001A, ":DEPTH  %7.2f", depth);
	}
}
