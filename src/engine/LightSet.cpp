#include "engine/LightZone.h"
#include "engine/Light.h"
#include "stage/Stage.h"
#include "core/dolphin.h"
#include "object/TObjSceneLightCollision.h"
#include "engine/LightSet.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "core/TSceneManager.h"
#include "animal/RacerMover.h"
struct SceneRecord;

// The four blended light slots Slot 0 drives the stage's sun and ambient colour

// names inferred from function

/** @size 0x5C */
struct LightSlot {
	LightSlot() { }

	int mFrom;        // _00 light param indices to blend between
	int mTo;          // _04
	Vec3f mDir;       // _08
	u32 mColor;       // _14
	u32 mAmbient;     // _18
	GXLightObj mObj;  // _1C
};

static LightSlot* sLightSlots;
static u32 sLightMask;
static int sLightSel;
static int sLightLast;

// 0x801A3A00
void lightSetCreate()
{
	LightSlot* slot;
	sLightSlots = new LightSlot[4];
	for (int i = 0; i < 4; i++) {
		slot = &sLightSlots[i];
		slot->mFrom     = 0;
		slot->mTo       = 0;
		slot->mDir.x    = 0.5f;
		slot->mDir.y    = -1.0f;
		slot->mDir.z    = 0.0f;
		if (njUnitVector(&slot->mDir) < gUnitVectorMin) {
			slot->mDir = gVecZero;
		}
		slot->mColor   = 0xFFFFFFFF;
		slot->mAmbient = 0x808080FF;
	}
	sLightMask = 1;
	sLightSel  = 0;
	sLightLast = 0;
}

// 0x801A3B30
void lightSetDestroy()
{
	if (sLightSlots != 0) {
		delete[] sLightSlots;
		sLightSlots = 0;
	}
}
// I don't have the enums for this yet 0x801A3B6C
int lightSetIsValid(LightSlot* slots, int slot, int param)
{
	if (slots == 0) {
		return 0;
	}
	if (slot < 0 || slot >= 4) {
		return 0;
	}
	if (param < 0 || slot >= 0x80) {
		return 0;
	}
	return 1;
}

// 0x801A3BB4
void lightSetFrom(int slot, int param)
{
	if (lightSetIsValid(sLightSlots, slot, param)) {
		sLightSlots[slot].mFrom = param;
	}
}

// 0x801A3C04
void lightSetTo(int slot, int param)
{
	if (lightSetIsValid(sLightSlots, slot, param)) {
		sLightSlots[slot].mTo = param;
	}
}

// 0x801A3C58
void lightSetBlend(f32 t)
{
	if (sLightSlots != 0) {
		if (t < 0.0f) {
			t = 0.0f;
		} else if (t > 1.0f) {
			t = 1.0f;
		}
		for (int i = 0; i < 4; i++) {
			LightSlot* slot = &sLightSlots[i];
			lightParamBlend(t, slot->mFrom, slot->mTo, &slot->mDir, &slot->mColor, &slot->mAmbient);
		}
		LightSlot* first;
		SceneLights* scene;
		sceneLightsSetSunDir(scene = getCurrentStage()->mLights, &(first = sLightSlots)->mDir);
		sceneLightsSetSunColor(scene, &first->mColor);
		sceneLightsSetAmbient(scene, &first->mAmbient);
		sLightMask = scene->mSun.mGxId;
	}
}

// 0x801A3D34
void lightSetLoad()
{
	if (sLightSlots != 0) {
		lightGroupLoad(getSceneRecord()->mLights);
		for (int i = 1; i < 4; i++) {
			LightSlot* slot = &sLightSlots[i];
			lightInitDirectional(&slot->mObj, _gj_matrix_, &slot->mDir, &slot->mColor);
		}
		GXLoadLightObjImm(&sLightSlots[1].mObj, GX_LIGHT5); // GX_LIGHT5
		GXLoadLightObjImm(&sLightSlots[2].mObj, GX_LIGHT6); // GX_LIGHT6
		GXLoadLightObjImm(&sLightSlots[3].mObj, GX_LIGHT7); // GX_LIGHT7
		sLightSel  = 0;
		sLightLast = 0;
	}
}

// 0x801A3DE0
// Which slot the next object draws with
void lightSetSelect(int slot)
{
	if (slot >= 0 && slot < 4) {
		sLightSel = slot;
	}
}

// 0x801A3DF8
// Sets the ambient color for the selected slot and gives back its GX light mask
u32 lightSetApply()
{
	if (sLightSel != sLightLast && sLightSlots != 0) {
		u32 c = ((u32*)&sLightSlots[sLightSel])[6];
		GXColor color;
		color.r = c >> 24;
		color.g = c >> 16;
		color.b = c >> 8;
		color.a = c;
		GXSetChanAmbColor(GX_COLOR0A0, color);
	}
	// todo enum
	u32 mask = 1;
	if (sLightSel == 0) {
		mask = sLightMask;
	} else if (sLightSel == 1) {
		mask = 0x20;
	} else if (sLightSel == 2) {
		mask = 0x40;
	} else if (sLightSel == 3) {
		mask = 0x80;
	}
	sLightLast = *(volatile int*)&sLightSel; // read again after the tests
	sLightSel  = 0;
	return mask;
}

// 0x801A3EC0
int lightSetFindArea(Vec3f* pos)
{
	return lightAreaFind(pos);
}
