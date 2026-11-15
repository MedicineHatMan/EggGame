#ifndef _SDKCALLS_H
#define _SDKCALLS_H

#include "Ninja/ninja.h"
#ifdef __cplusplus
#include "core/MathUtil.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif
void GXEnableTexOffsets(GXTexCoordID, GXBool, GXBool);
void GXSetPointSize(u8, GXTexOffset);
s32 GBAGetProcessStatus(s32 chan, u8* percent);
BOOL PADInit();
void AXInit();
void AXRegisterCallback(void (*callback)());
void MIXInit();
void C_QUATMtx(f32*, const f32*);
void GXSetTexCopySrc(u16 left, u16 top, u16 width, u16 height);
void GXCopyTex(void*, GXBool);
void GXInitTlutObj(GXTlutObj*, void*, GXTlutFmt, u16);
void GXLoadTlut(GXTlutObj* obj, u32 name);
void PSVECScale(const f32*, f32*, f32);

void PSMTXConcatArray(const f32* a, const f32* b, f32* ab, u32 count);
void GXInitLightAttn(GXLightObj*, f32, f32, f32, f32, f32, f32);
void GXGetLightAttnK(GXLightObj* obj, f32* k0, f32* k1, f32* k2);
void GXGetLightAttnA(GXLightObj* obj, f32* a0, f32* a1, f32* a2);
void syMemCopy2(void*, const void*, s32);
void PSMTXReorder(const f32* src, f32* dst);
#ifdef __cplusplus
void PSMTXROMultVecArray(const f32* mtx, const Vec3f* in, Vec3f* out, int count);
#endif
void C_MTXFrustum(f32 m[4][4], f32 t, f32 b, f32 l, f32 r, f32 n, f32 f);
void GXPixModeSync();
void GXSetScissorBoxOffset(s32 x, s32 y);
void* memcpy(void*, const void*, u32);
void* memmove(void*, const void*, u32);
int sprintf(char*, const char*, ...);
void OSPanic(const char* file, int line, const char* msg, ...);
extern u8 __lower_map[256];
void GXDrawSphere1(u8 depth);
void GXDrawCylinder(u8 sides);
void GXDrawCube();
void GXSetAlphaUpdate(GXBool);
void GXSetDstAlpha(GXBool, u8);
#ifdef __cplusplus
void PSMTXReflect(f32* mtx, const Vec3f* pos, const Vec3f* normal);
#endif
void qsort(void*, u32, u32, void*);
void PSQUATMultiply(const f32*, const f32*, f32*);
void* __register_global_object(void*, void*, void*);
#ifdef __cplusplus
}
#endif

#endif
