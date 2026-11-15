#include "engine/LightZone.h"
#include "io/ResFile.h"
#include "io/Resource.h"
#include "engine/Color.h"
#include "engine/LightSet.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "core/TSceneManager.h"
#include "animal/RacerMover.h"
#include "core/SdkCalls.h"
struct SceneRecord;

/** set_light_param.bin. @size 0x30 and more */

// 0x80243EB8
// GXDistAttnFn by LightParam::mDistFn
static const int sDistFns[] = { 0, 1, 2, 3 };
// 0x80243EC8
// GXSpotFn by LightParam::mSpotFn
static const int sSpotFns[] = { 0, 1, 2, 3, 4, 5, 6 };

static inline void set3(f32* out, f32 a, f32 b, f32 c)
{
	out[0] = a;
	out[1] = b;
	out[2] = c;
}

// Stripped nothing calls it but its 0.0f and 1.0f come first in the pool
static void lightUnit(f32* out)
{
	set3(out, 0.0f, 1.0f, 0.0f);
}

// 0x800A14C0
static SceneLights* getSceneLights()
{
	TStage* stage = getCurrentStage();
	return stage != 0 ? stage->mLights : 0;
}

// 0x800A14F4
static LightGroup* getLightGroup()
{
	SceneRecord* record = (SceneRecord*)getSceneRecord();
	if (record == 0) {
		return 0;
	}
	return record->mLights;
}

// 0x800A1528
void LightZoneSun::init()
{
	mBlend   = 0.0f;
	mFlags   = 0;
	mColor   = 0xFFFFFFFF;
	mAmbient = 0xFFFFFFFF;
}

// 0x800A1548
int LightZoneSun::isOn() const
{
	return lightFlagsOn(mFlags);
}

// 0x800A156C
void LightZoneSpot::init()
{
	mPos.x = 0.0f;
	mPos.y = 0.0f;
	mPos.z = 0.0f;
	mDir.x = 0.0f;
	mDir.y = -1.0f;
	mDir.z = 0.0f;
	mColor = 0xFFFFFFFF;
	mA[0]  = 1.0f;
	mA[1]  = 0.0f;
	mA[2]  = 0.0f;
	mK[0]  = 1.0f;
	mK[1]  = 0.0f;
	mK[2]  = 0.0f;
}

// 0x800A15B4
LightZone::LightZone()
{
	mPrev = 0;
	mNext = 0;
	if (sLightZones == 0) {
		sLightZones = this;
		return;
	}
	LightZone* last;
	for (last = sLightZones; last->mNext != 0; last = last->mNext) { }
	last->mNext = this;
	mPrev       = last;
}

// 0x800A1604
LightZone::~LightZone()
{
	if (mNext != 0) {
		mNext->mPrev = mPrev;
	}
	if (mPrev != 0) {
		mPrev->mNext = mNext;
	}
	if (sLightZones == this) {
		sLightZones = mNext;
	}
}

// 0x800A168C
extern "C" void lightZoneInit()
{
	loadSizedResource("set_light_param.bin", &sLightParamFile);
	sLightParams = (LightParam*)sLightParamFile->mData;
	if (sLightZoneCtrl == 0) {
		sLightZoneCtrl = new LightZoneCtrl;
	}
}

// 0x800A16EC
extern "C" void lightSetDefaultMain(int param)
{
	lightSetFrom(0, param);
}

// 0x800A1714
extern "C" void lightSetDefaultOther(int param)
{
	lightSetTo(0, param);
}

// 0x800A173C
extern "C" void lightZoneExit()
{
	if (sLightZoneCtrl != 0) {
		delete sLightZoneCtrl;
		sLightZoneCtrl = 0;
	}
	if (sLightZoneA != 0) {
		delete sLightZoneA;
		sLightZoneA = 0;
	}
	if (sLightZoneB != 0) {
		delete sLightZoneB;
		sLightZoneB = 0;
	}
	releaseResource(sLightParamFile);
	sLightParamFile = 0;
	sLightParams    = 0;
}

// 0x800A17B8
LightZoneCtrl::LightZoneCtrl()
{
	mSun.init();
	mSpot.init();
	mLight.mFlags = 2;
	mApplied      = 0;
	mSavedColor   = 0xFFFFFFFF;
	mSavedDir.x   = 0.0f;
	mSavedDir.y   = -1.0f;
	mSavedDir.z   = 0.0f;
	mSavedAmbient = 0;
	mId           = 0xFFFF;
}

// 0x800A1838
LightZoneCtrl::~LightZoneCtrl()
{
}

// 0x800A18A0
extern "C" LightParam* getLightParam(int param)
{
	if (param < 0 || param >= 0x80) {
		return 0;
	}
	return &sLightParams[param];
}

// 0x800A18C8
LightZone* LightZoneCtrl::find(u16 id, int kind)
{
	f32 dist = 0.0f;
	f32 best = 100000.0f;
	LightZone* zone;
	LightZone* found = 0;
	for (zone = sLightZones; zone != 0; zone = zone->mNext) {
		if (zone->test(id, kind, &dist) != 0 && dist < best) {
			best  = dist;
			found = zone;
		}
	}
	return found;
}

// 0x800A1968
void LightZoneCtrl::begin(u16 id)
{
	apply(id);
	load(0);
}

// 0x800A19A0
int LightZoneCtrl::apply(u16 id)
{
	mApplied = 0;
	applySun(id);
	applySpot(id);
	return mApplied != 0;
}

// 0x800A19F4
void LightZoneCtrl::load(int force)
{
	if (mApplied != 0 || force != 0) {
		LightGroup* group = getLightGroup();
		if (group != 0) {
			lightGroupLoad(group);
		}
	}
}

// 0x800A1A34
void LightZoneCtrl::applySun(u16 id)
{
	Vec3f dir;
	u32 color;

	mId                = id;
	SceneLights* scene = getSceneLights();
	if (scene == 0) {
		return;
	}
	LightZone* zone = find(id, 0);
	if (zone == 0) {
		return;
	}
	zone->getSun(id, &mSun);
	mSavedColor   = scene->mSun.mColor;
	mSavedDir     = scene->mSun.mDir;
	mSavedAmbient = scene->mAmbient.mColor;
	f32 t         = mSun.mBlend;
	if (mSun.isOn() != 0) {
		if (t >= 1.0f) {
			sceneLightsSetSunColor(scene, &mSun.mColor);
			sceneLightsSetSunDir(scene, &mSun.mDir);
		} else {
			lerpColor(t, &color, &mSavedColor, &mSun.mColor);
			vecLerp(t, &mSavedDir, &mSun.mDir, &dir);
			sceneLightsSetSunColor(scene, &color);
			sceneLightsSetSunDir(scene, &dir);
		}
	}
	if (mSun.mFlags & 4) {
		if (t >= 1.0f) {
			sceneLightsSetAmbient(scene, &mSun.mAmbient);
		} else {
			lerpColor(t, &color, &mSavedAmbient, &mSun.mAmbient);
			sceneLightsSetAmbient(scene, &color);
		}
	}
	mApplied |= 1;
}

// 0x800A1BB0
void LightZoneCtrl::applySpot(u16 id)
{
	LightGroup* group = getLightGroup();
	if (group == 0) {
		return;
	}
	LightZone* zone = find(id, 1);
	if (zone == 0) {
		return;
	}
	zone->getSpot(id, &mSpot);
	mLight.mPos   = mSpot.mPos;
	mLight.mDir   = mSpot.mDir;
	mLight.mColor = mSpot.mColor;
	set3(mLight.mA, mSpot.mA[0], mSpot.mA[1], mSpot.mA[2]);
	set3(mLight.mK, mSpot.mK[0], mSpot.mK[1], mSpot.mK[2]);
	group->add(&mLight);
	mApplied |= 2;
}

// 0x800A1C98
int LightZoneCtrl::restore()
{
	if (mApplied == 0) {
		return 0;
	}
	SceneLights* scene = getSceneLights();
	if (scene == 0) {
		return 0;
	}
	if (mApplied & 1) {
		sceneLightsSetSunDir(scene, &mSavedDir);
		sceneLightsSetSunColor(scene, &mSavedColor);
		sceneLightsSetAmbient(scene, &mSavedAmbient);
	}
	if (mApplied & 2) {
		LightGroup* group = getSceneRecord()->mLights;
		if (group != 0) {
			group->remove(&mLight);
		}
	}
	mApplied = 0;
	return 1;
}

// 0x800A1D44
void LightZoneCtrl::end(u16 id)
{
	restore();
	load(1);
}

// 0x800A1D7C
extern "C" void lightZoneBegin(u16 id)
{
	if (sLightZoneCtrl != 0) {
		sLightZoneCtrl->begin(id);
	}
}

// 0x800A1DB0
extern "C" void lightZoneEnd(u16 id)
{
	if (sLightZoneCtrl != 0) {
		sLightZoneCtrl->end(id);
	}
}

// 0x800A1DE4
extern "C" int lightFlagsOn(u32)
{
	return 1;
}

// 0x800A1DEC
int LightParam::hasDistAttn()
{
	return mDistFn != 0;
}

// 0x800A1E00
void LightParam::getDistAttn(f32* k0, f32* k1, f32* k2)
{
	if (hasDistAttn()) {
		GXLightObj obj;
		GXInitLightDistAttn(&obj, mRefDist, mRefBright, sDistFns[mDistFn]);
		GXGetLightAttnK(&obj, k0, k1, k2);
	} else {
		*k0 = 1.0f;
		*k1 = 0.0f;
		*k2 = 0.0f;
	}
}

// 0x800A1E90
int LightParam::hasSpot()
{
	return mSpotFn != 0;
}

// 0x800A1EA4
void LightParam::getSpotAttn(f32* a0, f32* a1, f32* a2)
{
	if (hasSpot()) {
		GXLightObj obj;
		GXInitLightSpot(&obj, mCutoff, sSpotFns[mSpotFn]);
		GXGetLightAttnA(&obj, a0, a1, a2);
	} else {
		*a0 = 1.0f;
		*a1 = 0.0f;
		*a2 = 0.0f;
	}
}

// 0x800A1F30
int LightParam::isPlain()
{
	bool plain = false;
	if (!hasDistAttn() && !hasSpot()) {
		plain = true;
	}
	return plain;
}

// 0x800A1F80
extern "C" void lightFarPoint(const Vec3f* pos, const Vec3f* dir, Vec3f* out)
{
	gjMulAddVector(-100000.0f, out, pos, dir);
}

// 0x800A1FB8
// Straight down white sun and grey ambient
extern "C" void lightParamGetDefault(Vec3f* dir, u32* color, u32* ambient)
{
	dir->x = 0.0f;
	dir->y = 1.0f;
	dir->z = 0.0f;
	if (njUnitVector(dir) < gUnitVectorMin) {
		*dir = gVecZero;
	}
	*color   = 0xFFFFFFFF;
	*ambient = 0x808080FF;
}

// 0x800A203C
extern "C" void lightParamGet(int param, Vec3f* dir, u32* color, u32* ambient)
{
	LightParam* p = getLightParam(param);
	if (p != 0) {
		*dir     = p->mDir;
		*color   = p->mColor;
		*ambient = p->mAmbient;
	} else {
		lightParamGetDefault(dir, color, ambient);
	}
}

// 0x800A20B4
extern "C" void lightParamBlend(f32 t, int from, int to, Vec3f* dir, u32* color, u32* ambient)
{
	if (t <= 0.0f) {
		lightParamGet(from, dir, color, ambient);
	} else if (t >= 1.0f) {
		lightParamGet(to, dir, color, ambient);
	} else {
		Vec3f dir0;
		Vec3f dir1;
		u32 color0;
		u32 color1;
		u32 ambient0;
		u32 ambient1;
		lightParamGet(from, &dir0, &color0, &ambient0);
		lightParamGet(to, &dir1, &color1, &ambient1);
		vecLerp(t, &dir0, &dir1, dir);
		lerpColor(t, color, &color0, &color1);
		lerpColor(t, ambient, &ambient0, &ambient1);
	}
}
