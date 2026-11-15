#include "stage/SetManager.h"
#include "engine/LightSet.h"
#include "io/NinjaFile.h"
#include "Ninja/ninja.h"
#include "stl/math.h"
#include "object/TObjSceneLightCollision.h"
#include "camera/TCamera.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "object/ObjAction.h"
#include "stage/SetSlot.h"
struct SetLink;

/**
 * One light area. Shape 0 is a sphere of mRadius, 1 a cylinder of mRadius and mHeight, 2 a box of mRadius by mDepth
 * turned by mRotY and mHeight high.
 * @size 0x28
 */
struct LightArea {
	~LightArea() {}
	LightArea* mNext; // _00
	int mLight;       // _04 0 to 3 lightAreaFind gives one more
	Vec3f mPos;       // _08
	int mRotY;        // _14
	int mShape;       // _18
	f32 mRadius;      // _1C
	f32 mHeight;      // _20
	f32 mDepth;       // _24
};

LightArea* sLightAreas; // 0x802C5A28

// 0x801A519C
// A record with _3E set int 0 and 1 are slot 1's from and to 2 and 3 slot 2's and slot 3's
static void lightAreaSetSlots(SetDesc* desc)
{
	lightSetFrom(1, desc->getInt(0));
	lightSetTo(1, desc->getInt(1));
	lightSetFrom(2, desc->getInt(2));
	lightSetTo(2, desc->getInt(3));
	lightSetFrom(3, desc->getInt(2));
	lightSetTo(3, desc->getInt(3));
}

// 0x801A5298
static void lightAreaRead(SetDesc* desc, LightArea* area)
{
	SetRot rot;
	int light;
	int shape;

desc->getPos(&area->mPos);
	desc->getRot(&rot);
	area->mRotY   = rot.y;
	area->mLight  = desc->getInt(0);
	area->mShape  = (int)desc->getFloat(0);
	area->mRadius = desc->getFloat(1);
	area->mHeight = desc->getFloat(2);
	area->mDepth  = desc->getFloat(3);

	light = area->mLight;
	if (light < 0) {
		light = 0;
	} else if (light > 3) {
		light = 3;
	}
	area->mLight = light;
	shape = area->mShape;
	if (shape < 0) {
		shape = 0;
	} else if (shape > 2) {
		shape = 2;
	}
	area->mShape = shape;
	area->mRadius = area->mRadius < 10.0f ? 10.0f : area->mRadius;
	area->mHeight = area->mHeight < 10.0f ? 10.0f : area->mHeight;
	area->mDepth  = area->mDepth < 10.0f ? 10.0f : area->mDepth;
	if (area->mShape != 0) {
		area->mPos.y += area->mHeight;
	}
}

// 0x801A5448
void lightAreaInit()
{
	sLightAreas = NULL;
}

// 0x801A5454
void lightAreaExit()
{
	LightArea* area = sLightAreas;
	while (area != NULL) {
		LightArea* next = area->mNext;
		delete area;
		area = next;
	}
	sLightAreas = NULL;
}

// 0x801A54A4
// The light of the first area pos is in plus one or 0 for none
int lightAreaFind(Vec3f* pos)
{
	f32 x = pos->x;
	f32 y = pos->y;
	f32 z = pos->z;
	LightArea* area;
	int light = -1;

	for (area = sLightAreas; area != NULL; area = area->mNext) {
		f32 dx = x - area->mPos.x;
		f32 dy = y - area->mPos.y;
		f32 dz = z - area->mPos.z;
		f32 r;
		switch (area->mShape) {
		case 0:
			r = area->mRadius;
			r *= r;
			if (dz * dz + (dy * dy + dx * dx) <= r) {
				light = area->mLight;
			}
			break;
		case 1:
			if ((f32)__fabs(dy) < 5.0f + area->mHeight) {
				r = area->mRadius;
				r *= r;
				if (dz * dz + dx * dx <= r) {
					light = area->mLight;
				}
			}
			break;
		case 2:
			if ((f32)__fabs(dy) < 5.0f + area->mHeight) {
				if (area->mRotY != 0) {
					f32 s, c, t;
					njSinCos(-area->mRotY, &s, &c);
					t  = dx;
					dx = c * dx + s * dz;
					dz = c * dz - s * t;
				}
				if ((f32)__fabs(dx) < area->mRadius && (f32)__fabs(dz) < area->mDepth) {
					light = area->mLight;
				}
			}
			break;
		}
		if (light != -1) {
			break;
		}
	}
	if (light == -1) {
		return 0;
	}
	return light + 1;
}

// 0x801A5680
extern "C" TObject* createTObjSceneLightCollision(TObject* parent, SetDesc* desc)
{
	return new TObjSceneLightCollision(parent, desc);
}

// 0x801A56D4
extern "C" void prepareTObjSceneLightCollision(SetDesc* desc)
{
	if (((SetDescObj*)desc)->get3E() != 0) {
		lightAreaSetSlots(desc);
		return;
	}
	LightArea* area = new LightArea;
	area->mNext = NULL;
	lightAreaRead(desc, area);
	if (sLightAreas == NULL) {
		sLightAreas = area;
		return;
	}
	LightArea* tail = sLightAreas;
	while (tail->mNext != NULL) {
		tail = tail->mNext;
	}
	tail->mNext = area;
}

const char* gTObjSceneLightCollisionName = "TObjSceneLightCollision";

// 0x801A5764
TObjSceneLightCollision::TObjSceneLightCollision(TObject* parent, const SetDesc* desc)
    : TObjAction(parent)
{
	_00 = (u32)gTObjSceneLightCollisionName;
	mSize = 0x48;
	setObjId(this, desc->mObjId);
	((SetLink*)&mLink)->attach((SetDesc*)desc);
	initFromDesc((SetDesc*)desc);
}

// 0x801A57EC
TObjSceneLightCollision::~TObjSceneLightCollision()
{
	notifySetLinkDeath((SetLink*)&mLink, (void*)this);
	releaseObjId(this);
}

// 0x801A586C
void TObjSceneLightCollision::initFromDesc(SetDesc* desc)
{
	desc->getPos(&mPos);
}

// 0x801A58A4
void TObjSceneLightCollision::draw()
{
	if (debugEditActive() != 0) {
		((void (*)(void*))debugInfoActive)(this);
	}
	if (isInRange(this, &mPos, 500.0f) == 0) {
		mFlags |= 1;
	}
}

// 0x801A5904
void TObjSceneLightCollision::debugPrint(void*)
{
	njPrintC(0xA000B, ":COL ANG Y");
}

// 0x801A5934
int TObjSceneLightCollision::getKind()
{
	return 3;
}

// 0x801A593C
Vec3f* TObjSceneLightCollision::getPos()
{
	return &mPos;
}
