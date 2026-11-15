#ifndef _NINJA_NJINTERNAL_H
#define _NINJA_NJINTERNAL_H

#include "Ninja/ninja.h"
#include "stl/stdarg.h"

s32 sprintf(char* out, const char* fmt, ...);
s32 vsprintf(char* out, const char* fmt, va_list args);
char* strcat(char* dst, const char* src);
int strcmp(const char* a, const char* b);
void GXInitTlutObj(void* obj, void* lut, s32 fmt, u16 count);
void GXLoadTlut(void* obj, u32 name);
u32 PADRead(void* status);
void strupr(char* s);
char* strcpy(char* dst, const char* src);
u32 strlen(const char* s);

// gjActor.c
void gjCalcActorModel(GjModel* model);
void gjShapeBlendVerts(GjModel* model, void* a, f32 t, void* b, f32 rest);

// gjDraw.c
s32 _gjDrawModel(GjModel* model);
void gjDrawListAdd(NJS_OBJECT* obj, const f32* m);
void gjDrawListBegin(s32 (*f)(GjModel*));
void gjDrawListFlush(void);
void gjDrawMotionTree(NJS_OBJECT* obj);

// gjPrim.c
void gjMtxPush(void);

// njMotion.c
void gjKeyBracket(void* keys, u32 size, u32 n, f32 frame, void** outA, void** outB, f32* outT);

// njPrint.c
void gjColorFromF32(const f32* in, GXColor* out);
void gjColorToF32(const GXColor* in, f32* out);
s32 gjMotionGetAngNone(s32* out);
s32 gjMotionGetVecNone(f32* out);

// njTexture.c
void gjTexClearErr(void);
s32 gjTexHeapReady(void);
s32 gjTexLoadSlot(s32 slot, const void* src);
s32 gjTexLookup(u32 gbix, s32 bank, u32 format, s32* out);
void gjTexMakePath(char* out, const char* name, const char* ext);
s32 gjTexReleaseSlot(s32 slot);
u8* gvrFindImage(GjTexDesc* desc, u32* gbix, u8* p);

// data
extern f32 _nj_constant_material_[4];
extern s32 _nj_control_3d_flag_;
extern s32 gDrawAlphaRef;
extern NJS_TEXERRSTAT gTexErrStat;
extern GjMotionState* gjCurMotionState;
extern GjDrawCalls* gjDrawCalls;
extern u32 gjLightsOn;
extern s32 gjTexHeadersUsed;
extern u8* gjTexHeap;
extern void (*gjObjectCallback)(NJS_OBJECT*);
extern void (*gjMeshParamFuncs[11])(u32);
extern OSHeapHandle gjTexHeapHandle;
extern u32 gjTexHeapSize;
extern GjTexMem* gjTexSlots;
extern s32 gjVertsAreFloat;
extern f32 njProjection[10];
extern f32 njProjection2[10];
extern void* njSinTable;

#endif
