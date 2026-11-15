#ifndef _ENGINE_SCENELIGHTS_H_
#define _ENGINE_SCENELIGHTS_H_
#include "stage/Stage.h"
struct TStageLights;

#include "core/types.h"

struct SceneLights;
struct Vec3f;

extern "C" {
void sceneLightsSetAmbient(SceneLights*, u32*);
void sceneLightsSetSunColor(SceneLights*, u32*);
void sceneLightsSetSunDir(SceneLights*, const Vec3f*);
}

/**
 * Holds a SceneLights and two more light sets. Vtable 0x80255A20.
 * @size 0x4C
 */
class TStageLights : public TObject {
public:
	TStageLights();          // 0x8004D260
	virtual ~TStageLights(); // 0x8004D2CC

	SceneLights* mLights; // _20
	LightSet mSetA;       // _24
	LightSet mSetB;       // _38
};

extern "C" {
extern TStageLights* sStageLights;
}

#endif
