#ifndef _NJMOTIONINTERNAL_H
#define _NJMOTIONINTERNAL_H

#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"

typedef struct NJS_MOTION_LINK {
	void* motion[2]; // _00
	f32 frame[2];    // _08
} NJS_MOTION_LINK;

s32 njGetMotionNodeData(NJS_OBJECT* obj, f32* pos, s32* ang, f32* scl, NJS_QUATERNION* quat);
s32 njGetMotionLinkNodeData(NJS_OBJECT* obj, f32* pos, s32* ang, f32* scl, NJS_QUATERNION* quat);
void njSetNextMotionNodeEx(void);

// The keys gjKeyBracket finds them by a u32 frame and gjKeyBracketS by a u16 one
typedef struct GjKeyVec {
	u32 frame; // _00
	f32 v[3];  // _04
} GjKeyVec;

typedef struct GjKeyQuat {
	u32 frame;        // _00
	NJS_QUATERNION q; // _04
} GjKeyQuat;

typedef struct GjKeyAng {
	u32 frame; // _00
	s32 v[3];  // _04
} GjKeyAng;

typedef struct GjKeyAng1 {
	u32 frame; // _00
	s32 v;     // _04
} GjKeyAng1;

typedef struct GjKeyAngS {
	u16 frame; // _00
	s16 v[3];  // _02
} GjKeyAngS;

typedef struct GjKeyAngS1 {
	u16 frame; // _00
	s16 v;     // _02
} GjKeyAngS1;

extern const NJS_MOTION gjNullMotion;
extern GjKeyFn gjVecKeyFuncs[3];
extern GjKeyFn gjAngKeyFuncs[3];
extern GjKeyFn gjPosKeyFuncs[3];
extern GjKeyFn gjAngSKeyFuncs[3];
extern GjMotionState gjMotionStates[2];
extern f32 gjLinkRate;

s32 gjMotionGetVec(f32* out);
s32 gjMotionGetAngS1(s16* out);
s32 gjMotionGetAng(s32* out);
s32 gjMotionGetAngS(s16* out);
s32 gjMotionGetQuat(NJS_QUATERNION* out);
void gjInterpQuat(GjKeyQuat* keys, u32 n, f32 frame, NJS_QUATERNION* out);
void gjInterpVec(GjKeyVec* keys, u32 n, f32 frame, f32* out);
void gjInterpVec2(GjKeyVec* keys, u32 n, f32 frame, f32* out);
void gjInterpAng1(GjKeyAng1* keys, u32 n, f32 frame, s32* out);
void gjInterpAng(GjKeyAng* keys, u32 n, f32 frame, s32* out);
void gjInterpAngS(GjKeyAngS* keys, u32 n, f32 frame, s16* out);
void gjInterpAngS1(GjKeyAngS1* keys, u32 n, f32 frame, s16* out);
void gjInterpVecSpline(GjKeyVec* keys, u32 n, f32 frame, f32* out);
void gjMotionSetListCount(s32 nbList);
void gjMotionBegin(NJS_MOTION* motion, f32 frame);
void gjMotionSelectState(s32 index);
u32 gjKeyFind(const void* keys, u32 size, u32 hi, u32 frame);
void gjKeyBracket(void* keys, u32 size, u32 n, f32 frame, void** outA, void** outB, f32* outT);
u32 gjKeyFindS(const void* keys, u32 size, u32 hi, u16 frame);
void gjKeyBracketS(void* keys, u32 size, u32 n, f32 frame, void** outA, void** outB, f32* outT);
void gjMotionSetup(NJS_MOTION* motion, f32 frame);

#endif
