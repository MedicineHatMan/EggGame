#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"

extern GXLightObj gjLightObjs[8];
extern f32 gjLight0Dist; // how far away light 0 is

u32 gjLightMasks[5] = {
	0x00000001, 0x00000002, 0x00000004, 0x00000008, 0x00000010,
}; // a mask for each light

void gjCheckLightId(s32 id);

// 0x8021647C
void gjCheckLightId(s32 id)
{
	if (id < 0 || id > 5) {
		OSPanic("gjLight.c", 0x12, "illegal Light ID.");
	}
}

// 0x802164C4
void gjSetAmbient(f32 r, f32 g, f32 b)
{
	GXColor c;
	c.r = 255.0f * r;
	c.g = 255.0f * g;
	c.b = 255.0f * b;
	c.a = 0xFF;
	GXSetChanAmbColor(4, c);
}

// 0x8021653C
void gjSetLightSwitch(s32 id, s32 on)
{
	gjCheckLightId(id);
	if (on == 0) {
		gjLightsOn &= ~gjLightMasks[id];
	} else {
		gjLightsOn |= gjLightMasks[id];
	}
}

// 0x802165B4
void gjSetLightColor(s32 id, f32 r, f32 g, f32 b)
{
	GXColor c;
	gjCheckLightId(id);
	c.r = 255.0f * r;
	c.g = 255.0f * g;
	c.b = 255.0f * b;
	c.a = 0xFF;
	GXInitLightColor(&gjLightObjs[id], c);
}

// 0x80216684
// Light 0 is a far point that stands in for a direction
void gjSetLightVector(s32 id, f32 x, f32 y, f32 z)
{
	gjCheckLightId(id);
	if (id == 0) {
		GXInitLightPos(&gjLightObjs[id], gjLight0Dist * -x, gjLight0Dist * -y, gjLight0Dist * -z);
	} else {
		GXInitLightDir(&gjLightObjs[id], x, y, z);
	}
}

// 0x80216748
void gjLoadLight(s32 id)
{
	gjCheckLightId(id);
	GXLoadLightObjImm(&gjLightObjs[id], gjLightMasks[id]);
}
