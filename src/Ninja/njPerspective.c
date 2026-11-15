#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"

GjProjection gProjection;

void gjSetPerspective(s32 ang, f32 aspect, f32 n, f32 f)
{
	ang *= 0.75f;
	C_MTXPerspective(gProjection.mtx, 0.0054931640625 * ang, aspect, n, f);
	GXSetProjection(gProjection.mtx, 0);
	gProjection.type   = 0;
	gProjection.n      = n;
	gProjection.f      = f;
	gProjection.aspect = aspect;
	gProjection._50    = 0.5f * ((f32)_gj_rmode_->efbHeight * gProjection.mtx[5]);
	gProjection._54    = 1.0f / gProjection._50;
	njProjection2[6]   = gProjection._50;
	njProjection2[7]   = aspect / 1.3333334f;
	njProjection2[8]   = 1.0f;
	njProjection2[9]   = 1.0f / aspect;
	njProjection2[0]   = njProjection2[7] * njProjection2[6];
	njProjection2[1]   = -(njProjection2[8] * njProjection2[6]);
	njProjection2[4]   = 1.0f / njProjection2[0];
	njProjection2[5]   = 1.0f / njProjection2[1];
	njProjection[0]    = f;
	njProjection[1]    = n;
}
