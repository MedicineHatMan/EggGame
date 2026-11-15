#include "stage/SetManager.h"
#include "engine/LightZone.h"
#include "engine/CharaColli.h"
#include "object/ObjAction.h"
#include "io/NinjaFile.h"
#include "camera/TCamera.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "engine/Colli.h"
#include "stage/SetSlot.h"
#include "object/TObjSetLightCollision.h"
struct CharaColli;

class TObjSetLightCollision;

typedef char TObjSetLightCollision_size_check[(sizeof(TObjSetLightCollision) == 0xCC) ? 1 : -1];

// 0x800A21A8
// 1 inside 0.7 of the range falling to 0 at its edge
extern "C" f32 lightZoneFade(f32 dist, f32 range)
{
	f32 edge = range;
	edge *= 0.7f;
	dist -= edge;
	range -= edge;
	if (dist < 0.0f) {
		return 1.0f;
	}
	dist /= range;
	dist = 1.0f - dist;
	if (dist < 0.0f) {
		dist = 0.0f;
	}
	return dist;
}

// 0x800A21E8
ObjLightZone::ObjLightZone()
{
	mOwner = 0;
}

// 0x800A222C
ObjLightZone::~ObjLightZone()
{
}

// 0x800A2284
void ObjLightZone::setOwner(TObject* owner)
{
	mOwner = owner;
}

// 0x800A228C
int ObjLightZone::test(u16 id, int kind, f32* dist)
{
	if (mOwner != 0) {
		return ((TObjSetLightCollision*)mOwner)->test(id, kind, dist);
	}
	return 0;
}

// 0x800A22C0
int ObjLightZone::getSun(u16 id, LightZoneSun* sun)
{
	if (mOwner != 0) {
		return ((TObjSetLightCollision*)mOwner)->getSun(id, sun);
	}
	return 0;
}

// 0x800A22F4
int ObjLightZone::getSpot(u16 id, LightZoneSpot* spot)
{
	if (mOwner != 0) {
		return ((TObjSetLightCollision*)mOwner)->getSpot(id, spot);
	}
	return 0;
}

extern "C" void lightCollisionBuildShape(TObjSetLightCollision* light, int kind, f32 a, f32 b, f32 c);

// 0x800A2328
extern "C" TObject* createTObjSetLightCollision(TObject* parent, SetDesc* desc)
{
	return new TObjSetLightCollision(parent, desc);
}

// 0x800A237C
// get3E leaves the "no record" test in the condition flags
// 1 sets the main default light
extern "C" void prepareTObjSetLightCollision(SetDesc* desc)
{
	SetDescObj* objDesc = (SetDescObj*)desc;
	int kind            = objDesc->get3E();
	if (kind <= 0) {
		return;
	}
	int value = objDesc->getInt(0);
	if (kind == 1) {
		lightSetDefaultMain(value);
	} else {
		lightSetDefaultOther(value);
	}
}

const char* gTObjSetLightCollisionName = "TObjSetLightCollision";

// 0x800A23DC
TObjSetLightCollision::TObjSetLightCollision(TObject* parent, const SetDesc* desc)
    : TObjAction(parent)
{
	_00   = (u32)gTObjSetLightCollisionName;
	mSize = 0xCC;
	setObjId(this, desc->mObjId);
	mLink.get()->attach((SetDesc*)desc);
	mShape = 0;
	initFromDesc((SetDesc*)desc);
	if (mRecord == 0) {
		mShape = colliCreate(1, 1);
		colliSetOwner(mShape, (u16)mObjId);
		colliSetMasks((CharaColli*)mShape, 4, 255, 255, 252);
		lightCollisionBuildShape(this, mKind, mExtent[0], mExtent[1], mExtent[2]);
	}
	mNode.setOwner(this);
}

// 0x800A24D8
TObjSetLightCollision::~TObjSetLightCollision()
{
	if (mShape != 0) {
		colliRetire((CharaColli*)mShape);
		mShape = 0;
	}
	notifySetLinkDeath(mLink.get(), this);
	releaseObjId(this);
}

// 0x800A257C
// The three ints are kept in tenths
void TObjSetLightCollision::initFromDesc(SetDesc* desc)
{
	SetRot rot;

	desc->getPos(&mPos);
	desc->getRot(&rot);
	mRotY      = rot.y;
	mRotX      = rot.x;
	mRotZ      = rot.z;
	mParam     = desc->getInt(0);
	mRecord    = ((SetDescObj*)desc)->get3E();
	mTune[0]   = (f32)desc->getInt(1) * 0.1f;
	mTune[1]   = (f32)desc->getInt(2) * 0.1f;
	mTune[2]   = (f32)desc->getInt(3) * 0.1f;
	mKind      = (int)desc->getFloat(0);
	mExtent[0] = desc->getFloat(1);
	mExtent[1] = desc->getFloat(2);
	mExtent[2] = desc->getFloat(3);
	int kind   = mKind;
	if (kind < 0) {
		kind = 0;
	} else if (kind > 2) {
		kind = 2;
	}
	mKind      = kind;
	mExtent[0] = mExtent[0] < 2.0f ? 2.0f : mExtent[0];
	mExtent[1] = mExtent[1] < 2.0f ? 2.0f : mExtent[1];
	mExtent[2] = mExtent[2] < 2.0f ? 2.0f : mExtent[2];
	lightCollisionBuildShape(this, mKind, mExtent[0], mExtent[1], mExtent[2]);
}

// 0x800A27E4
// Kind 0 1 and 2 pick the shape
extern "C" void lightCollisionBuildShape(TObjSetLightCollision* light, int kind, f32 a, f32 b, f32 c)
{
	f32* partY;
	void* part;

	if (light->mShape == 0) {
		return;
	}
	part = colliGetShape((CharaColli*)light->mShape, 0);
	if (part == 0) {
		return;
	}
	switch (kind) {
	case 0:
		colliShapeMakeSphere((ColliShape*)part, (const Vec3f*)&light->mPos, 0, a);
		break;
	case 1:
		colliShapeMakeCylinder((ColliShape*)part, (const Vec3f*)&light->mPos, 1, a, b);
		partY = (f32*)((u8*)part + 4);
		*partY -= 0.3f * b;
		break;
	case 2:
		colliShapeMakeBox((ColliShape*)part, (const Vec3f*)&light->mPos, light->mRotY, a, b, c, 1);
		partY = (f32*)((u8*)part + 4);
		*partY -= 0.3f * b;
		break;
	}
	colliRefreshBounds((CharaColli*)light->mShape);
}

// 0x800A28FC
void TObjSetLightCollision::updateLight()
{
	LightParam* param = getLightParam(mParam);

	if (param != 0) {
		mLightPos = mPos;
		njAddVector(&mLightPos, (Vec3f*)mTune);
		mBase = mLightPos;
		gjPushUnitMatrix();
		gjRotateY(mRotZ);
		gjRotateX(mRotX);
		njCalcVectorEx(&param->mDir, &mDir);
		njPopMatrixEx();
		if (param->isPlain()) {
			lightFarPoint(&mLightPos, &mDir, &mLightPos);
		}
		mColor = param->mColor;
		param->getDistAttn(&mDistK[0], &mDistK[1], &mDistK[2]);
		param->getSpotAttn(&mSpotA[0], &mSpotA[1], &mSpotA[2]);
	}
}

// 0x800A29E4
// The param when there is a shape and the param has this type
LightParam* TObjSetLightCollision::getParam(int type)
{
	LightParam* param;

	if (mShape == 0) {
		return 0;
	}
	param = getLightParam(mParam);
	if (param == 0) {
		return 0;
	}
	if (param->mType != type) {
		return 0;
	}
	return param;
}

// 0x800A2A48
// Whether object id touches the zone and how far it is from the light
int TObjSetLightCollision::test(u16 id, int kind, f32* dist)
{
	ColliContact* contact;
	TObjAction* obj;
	Vec3f d;

	if (getParam(kind) == 0) {
		return 0;
	}
	if (mShape == 0) {
		return 0;
	}
	for (contact = ((CharaColli*)mShape)->mContacts; contact != 0; contact = contact->mNext) {
		if (id == (u16)colliContactOwner(contact)) {
			obj = findObjById((u16)id);
			if (obj != 0) {
				d = *obj->getPos();
				njSubVector(&d, &mBase);
				*dist = njScalor(&d);
				return 1;
			}
		}
	}
	return 0;
}

// 0x800A2B2C
// The sun for object id the blend fades out toward the shape's edge
int TObjSetLightCollision::getSun(u16 id, LightZoneSun* sun)
{
	LightParam* param = getParam(0);
	ColliContact* contact;
	TObjAction* obj;
	Vec3f at;
	Vec3f d;
	f32 blend;
	f64 a;
	f32 f;

	if (param == 0) {
		return 0;
	}
	if (mShape == 0) {
		return 0;
	}
	for (contact = ((CharaColli*)mShape)->mContacts; contact != 0; contact = contact->mNext) {
		if (id == (u16)colliContactOwner(contact)) {
			obj = findObjById((u16)id);
			if (obj == 0) {
				break;
			}
			at = *obj->getPos();
			d  = mPos;
			njSubVector(&d, &at);
			switch (mKind) {
			case 0:
				blend = lightZoneFade(njScalor(&d), mExtent[0]);
				break;
			case 1:
				d.y   = 0.7f * mExtent[1] + d.y;
				blend = lightZoneFade(vecLenXZ(&d), mExtent[0]);
				a     = __fabs(d.y);
				f     = lightZoneFade(a, mExtent[1]);
				if (blend > f) {
					blend = f;
				}
				break;
			case 2:
				d.y = 0.7f * mExtent[1] + d.y;
				if (mRotY != 0) {
					vecRotateY(&d, &d, -mRotY);
				}
				a     = __fabs(d.x);
				blend = lightZoneFade(a, mExtent[0]);
				a     = __fabs(d.y);
				f     = lightZoneFade(a, mExtent[1]);
				if (blend > f) {
					blend = f;
				}
				a = __fabs(d.z);
				f = lightZoneFade(a, mExtent[2]);
				if (blend > f) {
					blend = f;
				}
				break;
			}
			sun->mBlend   = blend;
			sun->mFlags   = param->mFlags;
			sun->mColor   = param->mColor;
			sun->mAmbient = param->mAmbient;
			sun->mDir     = mDir;
			return 1;
		}
	}
	return 0;
}

// 0x800A2D74
// The spot light when the first contact is object id
int TObjSetLightCollision::getSpot(u16 id, LightZoneSpot* spot)
{
	ColliContact* contact;

	if (getParam(1) == 0) {
		return 0;
	}
	if (mShape == 0) {
		return 0;
	}
	for (contact = ((CharaColli*)mShape)->mContacts; contact != 0;) {
		if (id != (u16)colliContactOwner(contact)) {
			break;
		}
		spot->mPos   = mLightPos;
		spot->mDir   = mDir;
		spot->mColor = mColor;
		spot->mA[0]  = mSpotA[0];
		spot->mA[1]  = mSpotA[1];
		spot->mA[2]  = mSpotA[2];
		spot->mK[0]  = mDistK[0];
		spot->mK[1]  = mDistK[1];
		spot->mK[2]  = mDistK[2];
		return 1;
	}
	return 0;
}

// 0x800A2E68
void TObjSetLightCollision::draw()
{
	if (debugEditActive() != 0) {
		((void (*)(void*))debugInfoActive)(this);
		updateLight();
	}
	if (isInRange(this, &mPos, 500.0f) == 0) {
		mFlags |= 1;
	}
}

// 0x800A2ED0
// The lines are the set-file labels
void TObjSetLightCollision::debugPrint(void* arg)
{
	ColliShape* shape;

	njPrintC(0xA000A, ":DIR ANG X");
	njPrintC(0xA000B, ":COL ANG Y");
	njPrintC(0xA000C, ":DIR ANG Y");
	switch (mRecord) {
	case 0:
		njPrintC(0xA000F, ":DEFAULT OFF");
		break;
	case 1:
		njPrintC(0xA000F, ":DEFAULT ON");
		break;
	case 2:
	default:
		njPrintC(0xA000F, ":DEFAULT NIGHT ON");
		break;
	}
	njPrintC(0xA0013, ":PARAM IDX");
	njPrint(0xA0014, ":OFS X  %3.2f", mTune[0]);
	njPrint(0xA0015, ":OFS Y  %3.2f", mTune[1]);
	njPrint(0xA0016, ":OFS Z  %3.2f", mTune[2]);
	if (mShape != 0 && (shape = (ColliShape*)colliGetShape(mShape, 0)) != 0) {
		switch (shape->mType) {
		case 0:
			njPrint(0xA0017, ":COL SPHERE");
			njPrint(0xA0018, ":RAD");
			break;
		case 2:
			njPrint(0xA0017, ":COL CYLINDER");
			njPrint(0xA0018, ":RAD");
			njPrint(0xA0019, ":HEIGHT");
			break;
		case 1:
			njPrint(0xA0017, ":COL CUBE");
			njPrint(0xA0018, ":WIDTH");
			njPrint(0xA0019, ":HEIGHT");
			njPrint(0xA001A, ":DEPTH");
			break;
		}
	}
}

// 0x800A30E0
int TObjSetLightCollision::getKind()
{
	return 3;
}

// 0x800A30E8
Vec3f* TObjSetLightCollision::getPos()
{
	return &mPos;
}

// 0x800A30F0
// The class's set-up and tear-down both empty
extern "C" void lightCollisionInit()
{
}

// 0x800A30F4
extern "C" void lightCollisionExit()
{
}
