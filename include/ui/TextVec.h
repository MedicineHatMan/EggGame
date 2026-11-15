#ifndef UI_TEXTVEC_H
#define UI_TEXTVEC_H

#include "stage/Stage.h"
#include "stage/SetManager.h"

struct TextVec : public Vec3f {
	inline TextVec() { }
};

inline TextVec textVec(f32 x, f32 y, f32 z)
{
	TextVec v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

// A file-scope vector the static initializer stores straight into with no temporary
struct InitVec : public Vec3f {
	inline InitVec(f32 x, f32 y, f32 z) { set(x, y, z); }
};

struct TextRot : public SetRot {
	inline TextRot() { }
};

inline TextRot textRot(int x, int y, int z)
{
	TextRot r;
	r.x = x;
	r.y = y;
	r.z = z;
	return r;
}

#endif
