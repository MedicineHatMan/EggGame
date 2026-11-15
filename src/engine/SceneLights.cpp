#include "stage/Stage.h"

#include "engine/SceneLights.h"

extern "C" {

// 0x8004D1D0
void stageLightsInit()
{
	if (sStageLights == 0) {
		sStageLights = new TStageLights;
	}
}

// 0x8004D218
void stageLightsExit()
{
	if (sStageLights != 0) {
		delete sStageLights;
		sStageLights = 0;
	}
}
}

// 0x8004D260
TStageLights::TStageLights()
    : TObject(0)
{
	mLights = new SceneLights;
}

// 0x8004D2CC
TStageLights::~TStageLights()
{
	delete mLights;
}

// 0x8004D3C8
// Only the sun lights the diffuse channel
SceneLights::SceneLights()
{
	mSun.mFlags = 2;
	mSet.add(&mSun);
	mSet.add(&mAmbient);
}

extern "C" {
// 0x8004D428
void sceneLightsSetSunDir(SceneLights* scene, const Vec3f* dir)
{
	f32 y              = dir->y;
	scene->mSun.mDir.x = dir->x;
	f32 z              = dir->z;
	scene->mSun.mDir.y = y;
	scene->mSun.mDir.z = z;
}

// 0x8004D444
void sceneLightsSetSunColor(SceneLights* scene, u32* color)
{
	scene->mSun.mColor = *color;
}

// 0x8004D450
void sceneLightsSetAmbient(SceneLights* scene, u32* color)
{
	scene->mAmbient.mColor = *color;
}
}
