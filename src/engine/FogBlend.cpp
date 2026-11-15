#include "stage/Stage.h"
#include "engine/Landscape.h"
#include "engine/FogBlend.h"

extern "C" {
f32 sFogBlend; // 0x802C4DB8

// 0x8004082C
void fogBlendInit()
{
	fogBlendReset();
}

// 0x8004084C
void fogBlendExit()
{
}

// 0x80040850
// Reads the blend again from the landscape
f32 fogBlendUpdate()
{
	sFogBlend = getLandscapeBlend();
	return sFogBlend;
}

// 0x80040878
void fogBlendSet(f32 blend)
{
	if (blend > 1.0f) {
		blend = 1.0f;
	} else if (blend < 0.0f) {
		blend = 0.0f;
	}
	sFogBlend = blend;
}

// 0x800408A4
void fogBlendReset()
{
	fogBlendSet(0.0f);
}

// 0x800408C8
// Blend 0 is day and 1 night
int fogBlendIsDay()
{
	fogBlendUpdate();
	return sFogBlend < 0.7f;
}

// 0x800408FC
// TEneBoss03_VolRock calls this when a rock lands
void fogBlendHit(Vec3f* pos, int, u16 id)
{
}
}
