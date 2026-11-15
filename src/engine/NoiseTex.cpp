#include "engine/NoiseTex.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "stl/math.h"
#include "Dolphin/OS/OSCache.h"
#include "core/Mem.h"
#include "stage/SetActor.h"
#include "engine/DrawTask.h"
#include "object/ObjAction.h"
#include "stage/TStageOrange.h"
#include "camera/TCamera.h"
#include "Ninja/ninja.h"
#include "object/ObjClassName.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCameraCtrl.h"
#include "core/MathUtil.h"
#include "stage/SetSlot.h"
#include "core/VecMath.h"
#include "object/TObjFractalCloud.h"

#define B  0x100
#define BM 0xff

#define RANDOM() ((int)(32768.0f * ((1.0f / 32768.0f) * (f32)rand())))

static int sNoisePerm[B + B + 2];       // 0x80282948
static f32 sNoiseGrad3[B + B + 2][3];   // 0x80283150
static f32 sNoiseGrad2[B + B + 2][2];   // 0x80284968
static f32 sNoiseGrad1[B + B + 2];      // 0x80285978
static int sNoiseReady;                 // 0x802C5748

// The usual inline sqrtf three Newton steps on frsqrte
static inline f32 sqrtfInline(f32 x)
{
	static const f64 _half  = .5;
	static const f64 _three = 3.0;
	volatile f32 y;
	if (x > 0.0f) {
		f64 guess = __frsqrte((f64)x);
		guess     = _half * guess * (_three - guess * guess * x);
		guess     = _half * guess * (_three - guess * guess * x);
		guess     = _half * guess * (_three - guess * guess * x);
		y         = (f32)(x * guess);
		return y;
	}
	return x;
}

static inline void normalize2(f32 v[2])
{
	f32 s = sqrtfInline(v[0] * v[0] + v[1] * v[1]);
	v[0] = v[0] / s;
	v[1] = v[1] / s;
}

static inline void normalize3(f32 v[3])
{
	f32 s = sqrtfInline(v[0] * v[0] + v[1] * v[1] + v[2] * v[3]); // v[3] not v[2] the game's typo
	v[0] = v[0] / s;
	v[1] = v[1] / s;
	v[2] = v[2] / s;
}

// 0x801477A4
void noiseInit()
{
	int i, j, k;

	if (sNoiseReady != 0) {
		return;
	}

	for (i = 0; i < B; i++) {
		sNoisePerm[i]  = i;
		sNoiseGrad1[i] = (f32)((RANDOM() % (B + B)) - B) / B;

		for (j = 0; j < 2; j++) {
			sNoiseGrad2[i][j] = (f32)((RANDOM() % (B + B)) - B) / B;
		}
		normalize2(sNoiseGrad2[i]);

		for (j = 0; j < 3; j++) {
			sNoiseGrad3[i][j] = (f32)((RANDOM() % (B + B)) - B) / B;
		}
		normalize3(sNoiseGrad3[i]);
	}

	while (--i) {
		k             = sNoisePerm[i];
		sNoisePerm[i] = sNoisePerm[j = RANDOM() % B];
		sNoisePerm[j] = k;
	}

	for (i = 0; i < B + 2; i++) {
		sNoisePerm[B + i]  = sNoisePerm[i];
		sNoiseGrad1[B + i] = sNoiseGrad1[i];
		for (j = 0; j < 2; j++) {
			sNoiseGrad2[B + i][j] = sNoiseGrad2[i][j];
		}
		for (j = 0; j < 3; j++) {
			sNoiseGrad3[B + i][j] = sNoiseGrad3[i][j];
		}
	}

	sNoiseReady = 1;
}

#define s_curve(t)     (t * t * (3.0f - 2.0f * t))
#define lerp(t, a, b)  (a + t * (b - a))
#define at2(rx, ry, q) (rx * q[0] + ry * q[1])

// Not in the DOL the linker stripped it
f32 noiseCurve(f32 t)
{
	return s_curve(t);
}

// 0x80147C14
f32 noise2(f32 x, f32 y)
{
	int bx0, bx1, by0, by1, b00, b10, b01, b11;
	f32 rx0, rx1, ry0, ry1, *q, sx, sy, a, b, u, v;
	int i, j;

	bx0 = (int)x & BM;
	bx1 = (bx0 + 1) & BM;
	rx0 = x - (int)x;
	rx1 = rx0 - 1.0f;

	by0 = (int)y & BM;
	by1 = (by0 + 1) & BM;
	ry0 = y - (int)y;
	ry1 = ry0 - 1.0f;

	i = sNoisePerm[bx0];
	j = sNoisePerm[bx1];

	b00 = sNoisePerm[i + by0];
	b10 = sNoisePerm[j + by0];
	b01 = sNoisePerm[i + by1];
	b11 = sNoisePerm[j + by1];

	sx = s_curve(rx0);
	sy = s_curve(ry0);

	q = sNoiseGrad2[b00];
	u = at2(rx0, ry0, q);
	q = sNoiseGrad2[b10];
	v = at2(rx1, ry0, q);
	a = lerp(sx, u, v);

	q = sNoiseGrad2[b01];
	u = at2(rx0, ry1, q);
	q = sNoiseGrad2[b11];
	v = at2(rx1, ry1, q);
	b = lerp(sx, u, v);

	return lerp(sy, a, b);
}
