#include "core/types.h"
#include "Ninja/ninja.h"
#include "core/MslMath.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/ctype_api.h"
#include "stl/math.h"
#include "egg/EggCreature.h"

#define FP_NAN       1
#define FP_INFINITE  2
#define FP_ZERO      3
#define FP_NORMAL    4
#define FP_SUBNORMAL 5

static inline int FP_CLASS(f32 x)
{
	switch ((*(s32*)&x) & 0x7F800000) {
	case 0x7F800000:
		if ((*(s32*)&x) & 0x007FFFFF) {
			return FP_NAN;
		}
		return FP_INFINITE;
	case 0:
		if ((*(s32*)&x) & 0x007FFFFF) {
			return FP_SUBNORMAL;
		}
		return FP_ZERO;
	}
	return FP_NORMAL;
}

// MSL's toupper
static inline int toupper(int c)
{
	return (c == -1 ? -1 : (int)__upper_map[(u8)c]);
}

// 0x8020C808
double acos(double x)
{
	return __ieee754_acos(x);
}

// 0x8020C828
double asin(double x)
{
	return __ieee754_asin(x);
}

// 0x8020C848
double atan2(double y, double x)
{
	return __ieee754_atan2(y, x);
}

// 0x8020C868
double log(double x)
{
	return __ieee754_log(x);
}

// 0x8020C888
double log10(double x)
{
	return __ieee754_log10(x);
}

// 0x8020C8A8
double pow(double x, double y)
{
	return __ieee754_pow(x, y);
}

// 0x8020C8C8
// Splits x into its whole part kept in whole and the fraction it returns
f32 modff(f32 x, f32* whole)
{
	f32 frac;
	s32 i;
	f32 y = x;

	switch (FP_CLASS(y)) {
	case FP_NAN:
	case FP_ZERO:
		*whole = y;
		return x;
	case FP_INFINITE:
		*whole = y;
		return 0.0f;
	case FP_SUBNORMAL:
		*whole = 0.0f;
		return x;
	}
	i    = (s32)y;
	frac = y - (f32)i;
	if (*(s32*)&frac == 0 || (*(s32*)&x & 0x7F800000) >= 0x4B800000) {
		*whole = y;
		return 0.0f;
	}
	*whole = (f32)i;
	return frac;
}

// 0x8020C9F8
// Three Newton steps on frsqrte
f32 sqrtf(f32 x)
{
	volatile f32 y;
	if (x > 0.0f) {
		f64 guess = __frsqrte((f64)x);
		guess     = 0.5 * guess * (3.0 - guess * guess * x);
		guess     = 0.5 * guess * (3.0 - guess * guess * x);
		guess     = 0.5 * guess * (3.0 - guess * guess * x);
		y         = (f32)(x * guess);
		return y;
	}
	return x;
}

// 0x8020CA5C
f32 powf(f32 x, f32 y)
{
	return (f32)pow(x, y);
}

// 0x8020CA80
f32 tanf(f32 x)
{
	return (f32)tan(x);
}

// 0x8020CAA4
f32 sinf(f32 x)
{
	return (f32)sin(x);
}

// 0x8020CAC8
f32 cosf(f32 x)
{
	return (f32)cos(x);
}

// 0x8020CAEC
// Upper-cases str in place njTexture uses it on texture names
char* strupr(char* str)
{
	for (char* s = str; *s != 0; s++) {
		*s = toupper(*s);
	}
	return str;
}
