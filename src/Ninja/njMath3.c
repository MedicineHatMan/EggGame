#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"

// 0x8020FC34
void njRotateY(f32* m, s32 ang)
{
	NJS_MATRIX r;
	f32* cur;
	if (m == NULL) {
		cur = _gj_matrix_;
	} else {
		cur = m;
	}
	PSMTXRotRad(r, 'y', NJ_ANG_TO_RAD * ang);
	PSMTXConcat(cur, r, cur);
}
