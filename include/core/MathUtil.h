#ifndef _CORE_MATHUTIL_H_
#define _CORE_MATHUTIL_H_
#include "Ninja/ninja.h"
#include "stage/Stage.h"
struct HeldQuat;
struct Mtx34;

#include "core/types.h"

struct Vec3f;

extern "C" {
f32 distXZ(const Vec3f*, const Vec3f*);
f32 lerpF(f32, f32, f32);
void mtxSetRotX(f32*, int);
void mtxTranspose(f32*, const f32*);
f32 vecCrossXZ(const Vec3f*, const Vec3f*);
f32 vecDotXZ(const Vec3f*, const Vec3f*);
void vecLerp(f32, const Vec3f*, const Vec3f*, Vec3f*);
}

extern "C" {
void mtxRotateZ(f32*, const f32*, int);
void mtxTranslate(f32*, const f32*, f32, f32, f32);
void mtxIdentity(f32*);
void mtxSetTransV(f32*, const Vec3f*);
void mtxRotateY(f32*, const f32*, int);
void mtxMultVecSR(const f32*, const Vec3f*, Vec3f*);
void mtxConcat(f32*, const f32*, const f32*);
void mtxGetPos(const f32*, Vec3f*);
int vecAngleXZ(const Vec3f*);
void mtxSetRotY(f32*, int);
void mtxConcatQuat(f32*, const f32*, const f32*);
void mtxRotateY(f32*, const f32*, int);
void mtxScale(f32, f32, f32, f32*, const f32*);
void vecFromAngleXZ(f32, Vec3f*, int);
f32 vecLenXZ(const Vec3f*);
}

extern "C" {
void mtxFromQuat(f32*, const f32*);
void mtxMultVec(const f32*, const Vec3f*, Vec3f*);
void mtxToQuat(const f32*, f32*);
void vecRotateY(const Vec3f*, Vec3f*, int);
void mtxTranslateV(f32*, const f32*, const Vec3f*);
void mtxScaleV(f32*, const f32*, const Vec3f*);
void vecReflect(const Vec3f*, const Vec3f*, Vec3f*);
void mtxRotateX(f32*, const f32*, int);
void mtxRotateZYX(f32*, const f32*, const int*);
}

extern "C" {
f32 vecNormalizeXZ(const Vec3f*, Vec3f*);
int angleToXZ(const Vec3f*, const Vec3f*);
void mtxSetScale(f32*, f32, f32, f32);
void mtxSetTrans(f32*, f32, f32, f32);
int mtxInverse(f32*, const f32*);
}

extern "C" {
void mtxSetScaleV(f32*, const Vec3f*);
}

struct HeldQuat {
	f32 w;
	Vec3f v;
};

struct Mtx34 {
	f32 m[12];
};

extern "C" {
extern void* njSinTable;
extern const f32 mathOne;
extern int gjLastRetrace;
}

/** The smallest length a vector is normalised from */
extern const f32 gUnitVectorMin;

extern "C" {
double fabs(double);
f32 fabsf(f32);
}

#endif
