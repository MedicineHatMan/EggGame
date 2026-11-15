#include "stage/Stage.h"
#include "engine/LightZone.h"
#include "object/ObjAction.h"
#include "engine/TFogCtrl.h"
#include "camera/TCameraCtrl.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "object/yellow/TObjYellowSand.h"
#include "Ninja/ninja.h"
#include "core/PlacementNew.h"
#include "engine/FogBlend.h"
#include "object/yellow/TObjYellowDoubleFog.h"
#include "core/TMainTask.h"
struct TObjAction;

typedef char TObjYellowDoubleFog_size_check[(sizeof(TObjYellowDoubleFog) == 0x30) ? 1 : -1];

#define sInstance sYellowDoubleFogObj

static inline f32 randUnit()
{
	return 3.0517578e-05f * (f32)rand();
}

static inline f32 randSigned()
{
	return 2.0f * (3.0517578e-05f * rand() - 0.5f);
}

// 0x8017D98C
extern "C" void initYellowDoubleFog()
{
	sInstance = 0;
	fogSetDefaultMain(0x24);
	fogSetDefaultOther(0x10);
	lightSetDefaultMain(8);
	lightSetDefaultOther(9);
}

// 0x8017D9D0
extern "C" void termYellowDoubleFog()
{
	if (sInstance != 0) {
		sInstance->mFlags |= 1;
		sInstance = 0;
	}
}

// 0x8017D9F4
extern "C" void createYellowDoubleFog()
{
	if (sInstance == 0) {
		sInstance = new TObjYellowDoubleFog(gTL_02);
	}
}

// 0x8017DA40
extern "C" void triggerYellowDoubleFog()
{
	if (sInstance != 0) {
		sInstance->trigger();
	}
}

// 0x8017DA6C
void TObjYellowDoubleFog::trigger()
{
	mTriggers++;
}

const char* gTObjYellowDoubleFogName = "TObjYellowDoubleFog";

// 0x8017DA7C
TObjYellowDoubleFog::TObjYellowDoubleFog(TObject* parent)
    : TObject(parent)
{
	_00        = (u32)gTObjYellowDoubleFogName;
	mSize      = 0x30;
	mState     = 0;
	mBoardTimer = 0;
	mTriggers  = 0;
	mStormTimer = 0;
}

// 0x8017DADC
TObjYellowDoubleFog::~TObjYellowDoubleFog()
{
	if (sInstance == this) {
		sInstance = 0;
	}
}

// 0x8017DB50
void TObjYellowDoubleFog::setFog(f32 rate, int low, int high)
{
	fogSetMain(low);
	fogSetOther(high);
	if (fogBlendIsDay() != 0) {
		fogRequest(0, low, 2, 10000, rate);
	} else {
		fogRequest(0, high, 2, 10000, rate);
	}
}

// 0x8017DBD8
void TObjYellowDoubleFog::draw()
{
	Vec3f cam;
	camGetAt(0, &cam);

	switch (mState) {
	case 0:
		if (cam.y >= 650.0f) {
			mState = 1;
			setFog(1.0f, 0x24, 0x10);
		} else {
			mState = 2;
			setFog(1.0f, 6, 6);
		}
		break;
	case 1:
		if (cam.y < 600.0f) {
			mState = 2;
			setFog(0.01f, 6, 6);
		}
		break;
	case 2:
		if (cam.y >= 700.0f) {
			mState = 1;
			setFog(0.01f, 0x24, 0x10);
		}
		break;
	}

	Vec3f cam2;
	camGetAt(0, &cam2);
	f32 blend = cam2.y - 590.0f;
	blend /= 60.0f;

	Vec3f dir;
	u32 lightColor;
	u32 ambient;

	LightParam* light = getLightParam(8);
	lightParamBlend(blend, 6, 0x24, &dir, &lightColor, &ambient);
	light->mDir     = dir;
	light->mColor   = lightColor;
	light->mAmbient = ambient;

	light = getLightParam(9);
	lightParamBlend(blend, 6, 0x10, &dir, &lightColor, &ambient);
	light->mDir     = dir;
	light->mColor   = lightColor;
	light->mAmbient = ambient;

	if (cam.y < 800.0f) {
		updateSandBoardTimer(&mBoardTimer, 691.0f);
	}

	if (mTriggers > 0) {
		mStormTimer--;
		if (mStormTimer <= 0) {
			mStormTimer = 90;
			TObjAction* player = (TObjAction*)findObjById(0);
			if (player != 0) {
				Vec3f start;
				Vec3f dir;
				Vec3f pos;
				pos = *player->getPos();
				pos.y = 693.0f;

				int angle = 65536.0f * (3.0517578e-05f * rand());
				njSinCos(angle, &dir.x, &dir.z);
				dir.y = 0.0f;
				gjMulAddVector(-400.0f, &pos, &pos, &dir);

				f32 sn;
				f32 cs;
				angle = 65536.0f * randUnit();
				njSinCos(angle, &sn, &cs);

				start = pos;
				f32 reach = 100.0f * randUnit();
				start.x += reach * sn;
				start.z += reach * cs;

				f32 side = randSigned() * 300.0f;
				start.x += side * cs;
				start.z -= side * sn;
				dir.y = 0.0f;

				gjScaleVector(0.5f + 0.5f * randUnit(), &dir, &dir);
				createSandStorm(gTL_05, &start, &dir);
			}
		}
	}
	mTriggers = 0;
}
