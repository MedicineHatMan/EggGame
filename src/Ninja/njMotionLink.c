#include "Ninja/njMotionInternal.h"

f32 gjLinkRate; // the blend between the two linked motions

// 0x8020DB78
void njStartMotionLink(const NJS_MOTION_LINK* link, f32 rate)
{
	gjMotionSelectState(1);
	gjMotionSetup(link->motion[1], link->frame[1]);
	gjMotionSelectState(0);
	gjMotionSetup(link->motion[0], link->frame[0]);
	gjLinkRate = rate;
}

// 0x8020DBE0
void njSetNextMotionLinkNode(void)
{
	gjMotionSelectState(1);
	njSetNextMotionNodeEx();
	gjMotionSelectState(0);
	njSetNextMotionNodeEx();
}

// 0x8020DC14
// Reads both motions and blends them
s32 njGetMotionLinkNodeData(NJS_OBJECT* obj, f32* pos, s32* ang, f32* scl, NJS_QUATERNION* quat)
{
	f32 pos0[3];
	f32 scl0[3];
	s32 ang0[3];
	NJS_QUATERNION quat0;
	f32 pos1[3];
	f32 scl1[3];
	s32 ang1[3];
	NJS_QUATERNION quat1;
	f32 rest;
	f32 rate;
	s32 type1;
	s32 type0;
	rate = gjLinkRate;
	rest = 1.0f - rate;

	gjMotionSelectState(1);
	type1 = njGetMotionNodeData(obj, pos1, ang1, scl1, &quat1);
	gjMotionSelectState(0);
	type0 = njGetMotionNodeData(obj, pos0, ang0, scl0, &quat0);
	if (type0 != 2) {
		if (type0 == 0) {
			njXYZAngleToQuaternion(ang0, &quat0);
		} else {
			njZXYAngleToQuaternion(ang0, &quat0);
		}
	}
	if (type1 != 2) {
		if (type1 == 0) {
			njXYZAngleToQuaternion(ang1, &quat1);
		} else {
			njZXYAngleToQuaternion(ang1, &quat1);
		}
	}
	pos[0] = pos0[0] * rest + pos1[0] * rate;
	pos[1] = pos0[1] * rest + pos1[1] * rate;
	pos[2] = pos0[2] * rest + pos1[2] * rate;
	scl[0] = scl0[0] * rest + scl1[0] * rate;
	scl[1] = scl0[1] * rest + scl1[1] * rate;
	scl[2] = scl0[2] * rest + scl1[2] * rate;
	njInterpolateQuaternion(&quat0, &quat1, rate, quat);
	return 2;
}

// 0x8020DDC4
void njMotionLinkTransformEx(NJS_OBJECT* obj)
{
	f32 pos[3];
	f32 scl[3];
	s32 ang[3];

	NJS_QUATERNION quat;
	s32 type = njGetMotionLinkNodeData(obj, pos, ang, scl, &quat);

	njTranslateEx(pos);
	switch (type) {
	case 0:
		njRotateEx(ang, 0);
		break;
	case 1:
		njRotateEx(ang, 1);
		break;
	default:
		njQuaternionEx(&quat);
		break;
	}
	njScaleEx(scl);
}
