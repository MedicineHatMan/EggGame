#include "Dolphin/mtx.h"
#include <math.h>

asm void PSQUATMultiply(const register Quaternion* p, const register Quaternion* q, register Quaternion* pq)
{
#ifdef __MWERKS__ // clang-format off
	nofralloc
	psq_l    f0, 0x0(p), 0, 0
	psq_l    f1, 0x8(p), 0, 0
	psq_l    f2, 0x0(q), 0, 0
	ps_neg   f5, f0
	psq_l    f3, 0x8(q), 0, 0
	ps_neg   f6, f1
	ps_merge01 f4, f5, f0
	ps_muls0 f7, f1, f2
	ps_muls0 f5, f5, f2
	ps_merge01 f1, f6, f1
	ps_muls1 f8, f4, f2
	ps_madds0 f7, f4, f3, f7
	ps_muls1 f2, f1, f2
	ps_madds0 f5, f1, f3, f5
	ps_madds1 f8, f6, f3, f8
	ps_merge10 f7, f7, f7
	ps_madds1 f2, f0, f3, f2
	ps_merge10 f5, f5, f5
	ps_add   f7, f7, f2
	psq_st   f7, 0x0(pq), 0, 0
	ps_sub   f5, f5, f8
	psq_st   f5, 0x8(pq), 0, 0
	blr
#endif // clang-format on
}

// MSL's sqrtf y is read back on the x > 0 path and x itself is returned otherwise which is why the DOL keeps s in x's
// register f4
static inline f32 QUATSqrtf(f32 x)
{
	static const f64 _half  = .5;
	static const f64 _three = 3.0;
	vf32 y;
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

/** The rotation in m as a quaternion */
// 0x801DABE0
void C_QUATMtx(Quaternion* r, const Mtx m)
{
	f32 tr, s;
	s32 i, j, k;
	s32 nxt[3] = { 1, 2, 0 };
	f32 q[3];

	tr = m[0][0] + m[1][1] + m[2][2];
	if (tr > 0.0f) {
		s    = QUATSqrtf(tr + 1.0f);
		r->w = s * 0.5f;
		s    = 0.5f / s;
		r->x = (m[2][1] - m[1][2]) * s;
		r->y = (m[0][2] - m[2][0]) * s;
		r->z = (m[1][0] - m[0][1]) * s;
	} else {
		i = 0;
		if (m[1][1] > m[0][0]) {
			i = 1;
		}
		if (m[2][2] > m[i][i]) {
			i = 2;
		}
		j    = nxt[i];
		k    = nxt[j];
		s    = QUATSqrtf((m[i][i] - (m[j][j] + m[k][k])) + 1.0f);
		q[i] = s * 0.5f;
		if (s != 0.0f) {
			s = 0.5f / s;
		}
		r->w = (m[k][j] - m[j][k]) * s;
		q[j] = (m[i][j] + m[j][i]) * s;
		q[k] = (m[i][k] + m[k][i]) * s;
		r->x = q[0];
		r->y = q[1];
		r->z = q[2];
	}
}
