#include "stage/SetManager.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "io/NinjaFile.h"
#include "camera/TCameraCtrl.h"
#include "camera/TCamera.h"
#include "engine/Color.h"
#include "object/TObjLauncher.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "object/ObjAction.h"
#include "core/MathUtil.h"
#include "object/TObjEggRing.h"
#include "stage/SetSlot.h"
#include "core/VecMath.h"
#include "object/TObjWindColli.h"
#include "core/TMainTask.h"

extern "C" {
const char* gTObjWindColliName = "TObjWindColli";
}

const char* sWindTypeNames[3] = { "DEFAULT", "SPHERE", "BOX" };
TWindCtrl* gWindCtrl;

enum {
	WIND_DEFAULT = 0,
	WIND_SPHERE  = 1,
	WIND_BOX     = 2,
};

typedef char TWindCtrl_size_check[(sizeof(TWindCtrl) == 0x2C) ? 1 : -1];

typedef char TObjWindColli_size_check[(sizeof(TObjWindColli) == 0x80) ? 1 : -1];

// 0x800F39C8
extern "C" TObject* createTObjWindColli(TObject* parent, SetDesc* desc)
{
	return new TObjWindColli(parent, desc);
}

// 0x800F3A1C
TObjWindColli::TObjWindColli(TObject* parent, const SetDesc* desc)
    : TObjAction(parent)
{
	_00   = (u32)gTObjWindColliName;
	mSize = 0x80;
	setObjId(this, desc->mObjId);
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
	mLink.get()->attach((SetDesc*)desc);
	initFromDesc((SetDesc*)desc);
}

// 0x800F3AC0
TObjWindColli::~TObjWindColli()
{
	removeDrawTask(mDraw);
	notifySetLinkDeath(mLink.get(), this);
	unregisterId();
}

// 0x800F3B48
// The wind of a record float 0 along z turned by int 2 about y and int 1 about x
extern "C" void windDirFromDesc(SetDesc* desc, Vec3f* out)
{
	f32 mtx[12];
	Vec3f v;
	mtxSetRotY(mtx, (int)(182.04445f * (f32)desc->getInt(2)));
	mtxRotateX(mtx, mtx, (int)(182.04445f * (f32)desc->getInt(1)));
	v.x = 0.0f;
	v.y = 0.0f;
	v.z = desc->getFloat(0);
	mtxMultVecSR(mtx, &v, out);
}

// 0x800F3C4C
// Whether a point in the shape's space is inside the unit sphere or box
extern "C" BOOL windShapeContains(int type, Vec3f* pos)
{
	switch (type) {
	case WIND_SPHERE:
		return njScalor2(pos) <= 1.0f;
	case WIND_BOX: {
		u8 inside = FALSE;
		u8 inXY   = (f32)__fabs(pos->x) <= 1.0f && (f32)__fabs(pos->y) <= 1.0f;
		if (inXY && (f32)__fabs(pos->z) <= 1.0f) {
			inside = TRUE;
		}
		return inside;
	}
	default:
		return FALSE;
	}
}

// 0x800F3D14
void TObjWindColli::initFromDesc(SetDesc* desc)
{
	Vec3f pos;
	SetRot rot;
	f32 mtx[12];
	int type = desc->getInt(0);
	if (type < 0 || type >= 3) {
		type = WIND_DEFAULT;
	}
	mType = type;
	desc->getPos(&pos);
	desc->getRot(&rot);
	mtxSetTransV(mtx, &pos);
	mtxRotateZYX((f32*)mtx, (const f32*)mtx, (const int*)&rot);
	switch (mType) {
	case WIND_DEFAULT:
		mtxCopy(mMatrix, mtx);
		break;
	case WIND_SPHERE:
		mtxScale(desc->getFloat(1), desc->getFloat(1), desc->getFloat(1), mMatrix, mtx);
		break;
	case WIND_BOX:
		mtxScale(desc->getFloat(1), desc->getFloat(2), desc->getFloat(3), mMatrix, mtx);
		break;
	}
	windDirFromDesc(desc, &mWind);
	if (mType == WIND_DEFAULT && gWindCtrl != 0) {
		gWindCtrl->setWind(&mWind);
	}
}

// 0x800F3F1C
void TObjWindColli::debugPrint(void*)
{
	njPrint(0xA0013, ":TYPE %s", sWindTypeNames[mType]);
	njPrintC(0xA0014, ":VEL ANG X");
	njPrintC(0xA0015, ":VEL ANG Y");
	njPrintC(0xA0017, "VEL");
	switch (mType) {
	case WIND_SPHERE:
		njPrintC(0xA0018, "RADIUS");
		break;
	case WIND_BOX:
		njPrintC(0xA0018, "SCALE X");
		njPrintC(0xA0019, "SCALE Y");
		njPrintC(0xA001A, "SCALE Z");
		break;
	}
}

// 0x800F4004
// The set class's prepare a DEFAULT record sets the stage's wind before anything spawns
extern "C" void prepareTObjWindColli(SetDesc* desc)
{
	if (desc->getInt(0) == WIND_DEFAULT) {
		windDirFromDesc(desc, &gWindDefault);
	}
}

// 0x800F4058
void TObjWindColli::draw()
{
	Vec3f pos;
	Vec3f at;
	f32 inv[12];
	if (debugEditActive() != 0) {
		return;
	}
	mtxGetPos(mMatrix, &pos);
	if (!isInRange(this, &pos, 500.0f)) {
		mFlags |= 1;
	} else if (mType == WIND_DEFAULT) {
		setLinkDone(mLink.get());
		mFlags |= 1;
	} else if (gWindCtrl != 0) {
		camGetAt(0, &at);
		if (mtxInverse(inv, mMatrix)) {
			mtxMultVec(inv, &at, &at);
			windShapeContains(mType, &at);
		}
	}
}

// 0x800F412C
void TObjWindColli::vfn_10()
{
}

// 0x800F4130
// Debug a red line along the wind then the shape
void TObjWindColli::render()
{
	Vec3f pos;
	Vec3f dir;
	f32 mtx[12];
	if (debugEditActive() == 0) {
		return;
	}
	mtxGetPos(mMatrix, &pos);
	if (0.0f == vecNormalize(&dir, &mWind)) {
		sWindLine[1].x = 0.0f;
		sWindLine[1].y = 0.0f;
		sWindLine[1].z = 20.0f;
	} else {
		f32 len        = 20.0f;
		sWindLine[1].x = dir.x * len;
		sWindLine[1].y = dir.y * len;
		sWindLine[1].z = dir.z * len;
	}
	njPushMatrixEx();
	njTranslateEx(&pos);
	njDrawLineExStart(0.5f, (gColorRed << 24) | (gColorRed >> 8), 0);
	njDrawLine3DExSetList((const f32*)sWindLine, 2);
	njDrawLineExEnd();
	njPopMatrixEx();
	if (mType != WIND_DEFAULT) {
		mtxConcat(mtx, gjGetMatrix(), mMatrix);
		switch (mType) {
		case WIND_SPHERE:
			debugWireSphere(mtx, &gColorBlack);
			break;
		case WIND_BOX:
			debugWireBox(mtx, &gColorBlack);
			break;
		}
	}
}

const char* gTWindCtrlName = "TWindCtrl";

// 0x800F4268
TWindCtrl::TWindCtrl(TObject* parent)
    : TObject(parent)
{
	_00   = (u32)gTWindCtrlName;
	mSize = 0x2C;
}

// 0x800F42B4
TWindCtrl::~TWindCtrl()
{
}

// 0x800F4310
void TWindCtrl::draw()
{
}

// 0x800F4314
void TWindCtrl::getWind(Vec3f* out) const
{
	out->x = mWind.x;
	out->y = mWind.y;
	out->z = mWind.z;
}

// 0x800F4330
extern "C" void windCreate()
{
	TWindCtrl* wind = new TWindCtrl(gTL_06);
	gWindCtrl       = wind;
	wind->mWind.x   = gWindDefault.x;
	wind->mWind.y   = gWindDefault.y;
	wind->mWind.z   = gWindDefault.z;
}

// 0x800F438C
extern "C" void windGet(Vec3f* out)
{
	if (gWindCtrl != 0) {
		gWindCtrl->getWind(out);
	}
}

// 0x800F43D0
// The static init of this file an empty blr in .ctors
struct WindColliStatic {
	WindColliStatic() { }
};
static WindColliStatic sWindColliStatic;
