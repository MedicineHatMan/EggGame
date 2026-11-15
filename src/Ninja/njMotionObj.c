#include "Ninja/njMotionInternal.h"

const NJS_MOTION gjNullMotion = { 0 }; // an empty motion for a null one
GjMotionState gjMotionStates[2];

// 0x8020DE50
// The motion data is a pointer table per list then a count table
void gjMotionSetListCount(s32 nbList)
{
	s32 size = nbList * 4;
	u8* base;
	gjCurMotionState->size  = size;
	gjCurMotionState->total = size + size;
	base                    = gjCurMotionState->base;
	gjCurMotionState->list0 = (void**)base;
	gjCurMotionState->list1 = (u32*)(base + gjCurMotionState->size);
	gjCurMotionState->index = 0;
}

// 0x8020DE94
void njSetNextMotionNodeEx(void)
{
	u8* base = gjCurMotionState->base;
	base += gjCurMotionState->total;
	gjCurMotionState->base  = base;
	gjCurMotionState->list0 = (void**)base;
	gjCurMotionState->list1 = (u32*)(base + gjCurMotionState->size);
	gjCurMotionState->index = 0;
}

// 0x8020DED0
void gjMotionBegin(NJS_MOTION* motion, f32 frame)
{
	s32 fn;
	if (motion == NULL) {
		motion = (NJS_MOTION*)&gjNullMotion;
	}
	fn                        = (motion->inp_fn & 0xC0) >> 6;
	gjCurMotionState->base    = motion->mdata;
	gjCurMotionState->nbFrame = motion->nbFrame;
	gjCurMotionState->type    = motion->type;
	gjCurMotionState->frame   = frame;
	gjCurMotionState->index   = 0;
	gjMotionSetListCount(motion->inp_fn & ~0xC0);
	gjCurMotionState->readVec  = gjVecKeyFuncs[fn];
	gjCurMotionState->readAng  = gjAngKeyFuncs[fn];
	gjCurMotionState->_2C      = gjPosKeyFuncs[0];
	gjCurMotionState->readAngS = gjAngSKeyFuncs[0];
}

// 0x8020DFA8
void gjMotionSelectState(s32 index)
{
	gjCurMotionState = &gjMotionStates[index];
}

// 0x8020DFC0
// The last key at or before the frame
u32 gjKeyFind(const void* keys, u32 size, u32 hi, u32 frame)
{
	u32 lo = 0;
	while (hi - lo > 1) {
		u32 mid = (lo + hi) / 2;
		if (frame >= *(u32*)((u8*)keys + mid * size)) {
			lo = mid;
		} else {
			hi = mid;
		}
	}
	return lo;
}

// 0x8020E000
// The two keys around the frame and how far between them
void gjKeyBracket(void* keys, u32 size, u32 n, f32 frame, void** outA, void** outB, f32* outT)
{
	u32 i  = gjKeyFind(keys, size, n, frame);
	u32* a = (u32*)((u8*)keys + i * size);
	u32* b;
	s32 dt;
	if (i < n - 1) {
		b = (u32*)((u8*)keys + (i + 1) * size);
	} else {
		b = (u32*)keys;
	}
	*outA = a;
	*outB = b;
	dt    = *b - *a;
	if (dt > 0) {
		*outT = (frame - *a) / dt;
	} else {
		*outT = frame - *a;
	}
}

// 0x8020E114
// gjKeyFind with a u16 frame
u32 gjKeyFindS(const void* keys, u32 size, u32 hi, u16 frame)
{
	u32 lo = 0;
	while (hi - lo > 1) {
		u32 mid = (lo + hi) / 2;
		if (frame >= *(u16*)((u8*)keys + mid * size)) {
			lo = mid;
		} else {
			hi = mid;
		}
	}
	return lo;
}

// 0x8020E158
// gjKeyBracket with a u16 frame
void gjKeyBracketS(void* keys, u32 size, u32 n, f32 frame, void** outA, void** outB, f32* outT)
{
	u32 i  = gjKeyFindS(keys, size, n, frame);
	u16* a = (u16*)((u8*)keys + i * size);
	u16* b;
	s16 dt;
	if (i < n - 1) {
		b = (u16*)((u8*)keys + (i + 1) * size);
	} else {
		b = (u16*)keys;
	}
	*outA = a;
	*outB = b;
	dt    = (s16)*b - (s16)*a;
	if (dt > 0) {
		*outT = (frame - *a) / dt;
	} else {
		*outT = frame - *a;
	}
}

// 0x8020E270
void njStartMotionObj(void* motion, f32 frame)
{
	gjMotionSelectState(0);
	gjMotionSetup(motion, frame);
}

// 0x8020E2B8
// Picks the readers from the motion's type 1 position 2 angles 4 scale 0x20 u16-framed angles 0x2000 quaternions
void gjMotionSetup(NJS_MOTION* motion, f32 frame)
{
	u32 type;
	gjMotionBegin(motion, frame);
	type = gjCurMotionState->type;
	if (type & 1) {
		gjCurMotionState->getPos = gjMotionGetVec;
	} else {
		gjCurMotionState->getPos = gjMotionGetVecNone;
	}
	if (type & 2) {
		gjCurMotionState->getAng  = gjMotionGetAng;
		gjCurMotionState->getQuat = NULL;
		gjCurMotionState->getAngS = NULL;
	} else if (type & 0x20) {
		gjCurMotionState->getAng  = NULL;
		gjCurMotionState->getQuat = NULL;
		gjCurMotionState->getAngS = gjMotionGetAngS;
	} else if (type & 0x2000) {
		gjCurMotionState->getAng  = NULL;
		gjCurMotionState->getQuat = gjMotionGetQuat;
		gjCurMotionState->getAngS = NULL;
	} else {
		gjCurMotionState->getAng  = gjMotionGetAngNone;
		gjCurMotionState->getQuat = NULL;
		gjCurMotionState->getAngS = NULL;
	}
	if (type & 4) {
		gjCurMotionState->getScl = gjMotionGetVec;
	} else {
		gjCurMotionState->getScl = gjMotionGetVecNone;
	}
}

// 0x8020E3F8
void njMotionTransformEx(NJS_OBJECT* obj)
{
	f32 pos[3];
	f32 scl[3];
	s32 ang[3];

	NJS_QUATERNION quat;
	s32 type = njGetMotionNodeData(obj, pos, ang, scl, &quat);

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

// 0x8020E484
// Returns the rotation order 0 XYZ 1 ZXY 2 a quaternion
// The node's own rotation for a track the motion lacks a quaternion when the node has one else its angles
#define gjObjAng(obj, out, quat, type)             \
	{                                              \
		u32 flags = (obj)->evalflags;              \
		if (flags & 0x400) {                       \
			(quat)->re    = (obj)->quatRe;         \
			(quat)->im[0] = *(f32*)&(obj)->ang[0]; \
			(quat)->im[1] = *(f32*)&(obj)->ang[1]; \
			(quat)->im[2] = *(f32*)&(obj)->ang[2]; \
			type          = 2;                     \
		} else {                                   \
			(out)[0] = (obj)->ang[0];              \
			(out)[1] = (obj)->ang[1];              \
			(out)[2] = (obj)->ang[2];              \
			if (flags & 0x20) {                    \
				type = 1;                          \
			} else {                               \
				type = 0;                          \
			}                                      \
		}                                          \
	}

s32 njGetMotionNodeData(NJS_OBJECT* obj, f32* pos, s32* ang, f32* scl, NJS_QUATERNION* quat)
{
	s16 angS[3];
	s32 type;
	u32 flags = obj->evalflags;

	if ((flags & 0x40) == 0) {
		if (gjCurMotionState->getPos(pos) == 0) {
			pos[0] = obj->pos[0];
			pos[1] = obj->pos[1];
			pos[2] = obj->pos[2];
		}
		if (gjCurMotionState->getAng != NULL) {
			if (gjCurMotionState->getAng(ang) != 0) {
				if (obj->evalflags & 0x20) {
					type = 1;
				} else {
					type = 0;
				}
			} else {
				gjObjAng(obj, ang, quat, type);
			}
		} else if (gjCurMotionState->getAngS != NULL) {
			if (gjCurMotionState->getAngS(angS) != 0) {
				ang[0] = angS[0];
				ang[1] = angS[1];
				ang[2] = angS[2];
				if (obj->evalflags & 0x20) {
					type = 1;
				} else {
					type = 0;
				}
			} else {
				gjObjAng(obj, ang, quat, type);
			}
		} else if (gjCurMotionState->getQuat(quat) != 0) {
			type = 2;
		} else {
			gjObjAng(obj, ang, quat, type);
		}
		if (gjCurMotionState->getScl(scl) == 0) {
			scl[0] = obj->scl[0];
			scl[1] = obj->scl[1];
			scl[2] = obj->scl[2];
		}
		njSetNextMotionNodeEx();
	} else {
		pos[0] = obj->pos[0];
		pos[1] = obj->pos[1];
		pos[2] = obj->pos[2];
		scl[0] = obj->scl[0];
		scl[1] = obj->scl[1];
		scl[2] = obj->scl[2];
		if (flags & 0x400) {
			quat->re    = obj->quatRe;
			quat->im[0] = *(f32*)&obj->ang[0];
			quat->im[1] = *(f32*)&obj->ang[1];
			quat->im[2] = *(f32*)&obj->ang[2];
			type        = 2;
		} else {
			ang[0] = obj->ang[0];
			ang[1] = obj->ang[1];
			ang[2] = obj->ang[2];
			if (flags & 0x20) {
				type = 1;
			} else {
				type = 0;
			}
		}
	}
	return type;
}

GjKeyFn gjVecKeyFuncs[3] = { (GjKeyFn)gjInterpVec, (GjKeyFn)gjInterpVecSpline, (GjKeyFn)gjInterpVec2 };

GjKeyFn gjAngKeyFuncs[3] = { (GjKeyFn)gjInterpAng, (GjKeyFn)gjInterpAng, (GjKeyFn)gjInterpAng };

GjKeyFn gjPosKeyFuncs[3] = { (GjKeyFn)gjInterpAng1, (GjKeyFn)gjInterpAng1, (GjKeyFn)gjInterpAng1 };

GjKeyFn gjAngSKeyFuncs[3] = { (GjKeyFn)gjInterpAngS, (GjKeyFn)gjInterpAngS, (GjKeyFn)gjInterpAngS };
