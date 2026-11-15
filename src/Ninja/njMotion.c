#include "Ninja/njMotionInternal.h"

typedef struct GjCameraBase {
	f32 pos[3]; // _00
	f32 vec[3]; // _0C
	s16 roll;   // _18
	s16 ang;    // _1A
} GjCameraBase;

typedef struct GjCameraOut {
	f32* pos;  // _00
	f32* vec;  // _04
	s16* roll; // _08
	s16* ang;  // _0C
} GjCameraOut;

// 0x8020CF0C
// Reads a camera node Each bit of the motion's type says a track is present
void njGetCameraMotion(const GjCameraBase* base, void* motion, const GjCameraOut* out, f32 frame)
{
	u32 type;
	f32* pos  = out->pos;
	f32* vec  = out->vec;
	s16* roll = out->roll;
	s16* ang  = out->ang;

	gjMotionSelectState(0);
	gjMotionBegin(motion, frame);
	type = gjCurMotionState->type;
	if (type & 1) {
		if (gjMotionGetVec(pos) == 0) {
			pos[0] = base->pos[0];
			pos[1] = base->pos[1];
			pos[2] = base->pos[2];
		}
	} else {
		pos[0] = base->pos[0];
		pos[1] = base->pos[1];
		pos[2] = base->pos[2];
	}
	if ((type & 2) || (type & 0x20) || (type & 0x2000)) {
		gjCurMotionState->index++;
	}
	if (type & 4) {
		gjCurMotionState->index++;
	}
	if (type & 0x40) {
		if (gjMotionGetVec(vec) != 0) {
			vec[0] = vec[0] - pos[0];
			vec[1] = vec[1] - pos[1];
			vec[2] = vec[2] - pos[2];
		} else {
			vec[0] = base->vec[0];
			vec[1] = base->vec[1];
			vec[2] = base->vec[2];
		}
	} else if (type & 8) {
		if (gjMotionGetVec(vec) == 0) {
			vec[0] = base->vec[0];
			vec[1] = base->vec[1];
			vec[2] = base->vec[2];
		}
	} else {
		vec[0] = base->vec[0];
		vec[1] = base->vec[1];
		vec[2] = base->vec[2];
	}
	if (type & 0x10) {
		gjCurMotionState->index++;
	}
	if (type & 0x20) {
		gjCurMotionState->index++;
	}
	if (type & 0x80) {
		if (gjMotionGetAngS1(roll) == 0) {
			*roll = base->roll;
		}
	}
	if (type & 0x100) {
		if (gjMotionGetAngS1(ang) == 0) {
			*ang = base->ang;
		}
	} else {
		*ang = base->ang;
	}
}

// 0x8020D168
// Reads the next list as positions or scales
s32 gjMotionGetVec(f32* out)
{
	GjMotionState* s = gjCurMotionState;
	void* keys       = s->list0[s->index];
	if (keys != NULL) {
		s->readVec(keys, s->list1[s->index], s->frame, out);
		gjCurMotionState->index++;
		return 1;
	}
	s->index++;
	return 0;
}

// 0x8020D1E0
// Reads the next list as a single u16-framed angle
s32 gjMotionGetAngS1(s16* out)
{
	GjMotionState* s = gjCurMotionState;
	void* keys       = s->list0[s->index];
	if (keys != NULL) {
		gjInterpAngS1(keys, s->list1[s->index], s->frame, out);
		gjCurMotionState->index++;
		return 1;
	}
	s->index++;
	return 0;
}

// 0x8020D250
// Reads the next list as angles
s32 gjMotionGetAng(s32* out)
{
	GjMotionState* s = gjCurMotionState;
	void* keys       = s->list0[s->index];
	if (keys != NULL) {
		s->readAng(keys, s->list1[s->index], s->frame, out);
		gjCurMotionState->index++;
		return 1;
	}
	s->index++;
	return 0;
}

// 0x8020D2C8
// Reads the next list as u16-framed angles
s32 gjMotionGetAngS(s16* out)
{
	GjMotionState* s = gjCurMotionState;
	void* keys       = s->list0[s->index];
	if (keys != NULL) {
		s->readAngS(keys, s->list1[s->index], s->frame, out);
		gjCurMotionState->index++;
		return 1;
	}
	s->index++;
	return 0;
}

// 0x8020D340
// Reads the next list as quaternions
s32 gjMotionGetQuat(NJS_QUATERNION* out)
{
	GjMotionState* s = gjCurMotionState;
	void* keys       = s->list0[s->index];
	if (keys != NULL) {
		gjInterpQuat(keys, s->list1[s->index], s->frame, out);
		gjCurMotionState->index++;
		return 1;
	}
	s->index++;
	return 0;
}

// 0x8020D3B0
void gjInterpQuat(GjKeyQuat* keys, u32 n, f32 frame, NJS_QUATERNION* out)
{
	f32 t;
	GjKeyQuat* a;
	GjKeyQuat* b;
	gjKeyBracket(keys, 0x14, n, frame, (void**)&a, (void**)&b, &t);
	njInterpolateQuaternion(&a->q, &b->q, t, out);
}

// 0x8020D410
// Linear
void gjInterpVec(GjKeyVec* keys, u32 n, f32 frame, f32* out)
{
	f32 t;
	GjKeyVec* a;
	GjKeyVec* b;
	gjKeyBracket(keys, 0x10, n, frame, (void**)&a, (void**)&b, &t);
	out[0] = a->v[0] + t * (b->v[0] - a->v[0]);
	out[1] = a->v[1] + t * (b->v[1] - a->v[1]);
	out[2] = a->v[2] + t * (b->v[2] - a->v[2]);
}

// 0x8020D4C0
// Linear with z at zero
void gjInterpVec2(GjKeyVec* keys, u32 n, f32 frame, f32* out)
{
	f32 t;
	GjKeyVec* a;
	GjKeyVec* b;
	gjKeyBracket(keys, 0x10, n, frame, (void**)&a, (void**)&b, &t);
	out[0] = a->v[0] + t * (b->v[0] - a->v[0]);
	out[1] = a->v[1] + t * (b->v[1] - a->v[1]);
	out[2] = 0.0f;
}

// 0x8020D554
// The difference wraps to s16 so an angle takes the short way round
void gjInterpAng1(GjKeyAng1* keys, u32 n, f32 frame, s32* out)
{
	f32 t;
	GjKeyAng1* a;
	GjKeyAng1* b;
	gjKeyBracket(keys, 8, n, frame, (void**)&a, (void**)&b, &t);
	out[0] = (f32)a->v + t * (s16)(b->v - a->v);
}

// 0x8020D5FC
void gjInterpAng(GjKeyAng* keys, u32 n, f32 frame, s32* out)
{
	f32 t;
	GjKeyAng* a;
	GjKeyAng* b;
	gjKeyBracket(keys, 0x10, n, frame, (void**)&a, (void**)&b, &t);
	out[0] = (f32)a->v[0] + t * (s16)(b->v[0] - a->v[0]);
	out[1] = (f32)a->v[1] + t * (s16)(b->v[1] - a->v[1]);
	out[2] = (f32)a->v[2] + t * (s16)(b->v[2] - a->v[2]);
}

// 0x8020D75C
void gjInterpAngS(GjKeyAngS* keys, u32 n, f32 frame, s16* out)
{
	f32 t;
	GjKeyAngS* a;
	GjKeyAngS* b;
	gjKeyBracketS(keys, 8, n, frame, (void**)&a, (void**)&b, &t);
	out[0] = (f32)a->v[0] + t * (s16)(b->v[0] - a->v[0]);
	out[1] = (f32)a->v[1] + t * (s16)(b->v[1] - a->v[1]);
	out[2] = (f32)a->v[2] + t * (s16)(b->v[2] - a->v[2]);
}

// 0x8020D8BC
void gjInterpAngS1(GjKeyAngS1* keys, u32 n, f32 frame, s16* out)
{
	f32 t;
	GjKeyAngS1* a;
	GjKeyAngS1* b;
	gjKeyBracketS(keys, 4, n, frame, (void**)&a, (void**)&b, &t);
	out[0] = (f32)a->v + t * (s16)(b->v - a->v);
}

// 0x8020D964
// Catmull-Rom through the keys wrapping at both ends
void gjInterpVecSpline(GjKeyVec* keys, u32 n, f32 frame, f32* out)
{
	f32 d[2] = { 1.0f, 1.0f };
	u32 i    = gjKeyFind(keys, 0x10, n, frame);
	s32 dt;
	u32 start;
	GjKeyVec* k0;
	GjKeyVec* k1;
	GjKeyVec* k2;
	GjKeyVec* k3;
	f32 tt, t2, t3, t, c0, c1;
	k0    = &keys[(i + n - 1) % n];
	k1    = &keys[i];
	k2    = &keys[(i + 1) % n];
	k3    = &keys[(i + 2) % n];
	start = k1->frame;
	dt    = k2->frame - start;

	d[1]   = (u32)dt;
	t      = (frame - start) / d[dt > 0];
	t2     = t * t;
	t3     = t2 * t;
	c0     = 0.5f * (-t3 + (tt = 2.0f * t2) - t);
	c1     = 1.0f + (1.5f * t3 - 2.5f * t2);
	t      = 0.5f * t + (-1.5f * t3 + tt);
	t2     = 0.5f * (t3 - t2);
	out[0] = c0 * k0->v[0] + c1 * k1->v[0] + t * k2->v[0] + t2 * k3->v[0];
	out[1] = c0 * k0->v[1] + c1 * k1->v[1] + t * k2->v[1] + t2 * k3->v[1];
	out[2] = c0 * k0->v[2] + c1 * k1->v[2] + t * k2->v[2] + t2 * k3->v[2];
}
