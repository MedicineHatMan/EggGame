#ifndef _NINJA_NINJA_H
#define _NINJA_NINJA_H
struct Vec3f;

#include "core/types.h"
#include "Dolphin/GX/GXTypes.h"
#include "Dolphin/os.h"
#include "Ninja/njtypes.h"

// Sega's Ninja library and its Ginja layer for GameCube

#ifdef __cplusplus
extern "C" {
#endif

typedef f32 NJS_MATRIX[12];
typedef NJS_MATRIX* NJS_MATRIX_PTR;

typedef struct NJS_VECTOR {
	f32 x;
	f32 y;
	f32 z;
} NJS_VECTOR, NJS_POINT3;

typedef struct NJS_POINT2 {
	f32 x;
	f32 y;
} NJS_POINT2;

// A screen quad for njDrawQuadTexture its corners then the texture coordinates at them
typedef struct NJS_QUAD_TEXTURE {
	f32 x1, y1, x2, y2; // _00
	f32 u1, v1, u2, v2; // _10
} NJS_QUAD_TEXTURE;

typedef struct NJS_QUATERNION {
	f32 re;
	f32 im[3];
} NJS_QUATERNION;

// The SDK's order with the real part last
typedef struct GXQuaternion {
	f32 x;
	f32 y;
	f32 z;
	f32 w;
} GXQuaternion;

typedef struct GXViewport {
	f32 v[6];
} GXViewport;

typedef struct GjTexInfo {
	s32 format;  // _00
	u32 _04;     // _04
	u32 _08;     // _08
	s32 width;   // _0C
	s32 height;  // _10
	u32 _14;     // _14
	u32 flags;   // _18 0x80000000 mipmapped 0x8000 palette
	void* image; // _1C
} GjTexInfo;

typedef struct GjTexHeader {
	void* texaddr;  // _00 or the global index once loaded
	GjTexInfo info; // _04
	s32 count;      // _24
} GjTexHeader;

// What njSetTextureInfo fills: a GjTexHeader without the count. @size 0x24
typedef struct NJS_TEXINFO {
	void* texaddr;  // _00
	GjTexInfo info; // _04
} NJS_TEXINFO;

typedef struct GjTexMem {
	u32 _00;          // _00
	u32 _04;          // _04
	s32 tlut;         // _08 the palette bank
	GjTexHeader* hdr; // _0C
	s32 count;        // _10 users of this slot
	u32 _14;          // _14
} GjTexMem;

struct GjModel;

typedef struct NJS_OBJECT {
	u32 evalflags;              // _00
	struct GjModel* model;      // _04
	f32 pos[3];                 // _08
	s32 ang[3];                 // _14
	f32 scl[3];                 // _20
	struct NJS_OBJECT* child;   // _2C
	struct NJS_OBJECT* sibling; // _30
	f32 quatRe;                 // _34 only with evalflags 0x400
} NJS_OBJECT;

/** A Ninja motion the per-node keys how many frames and which kinds of key each node has */
typedef struct NJS_MOTION {
	void* mdata; // _00
	u32 nbFrame; // _04
	u16 type;    // _08
	u16 inp_fn;  // _0A the node's lists and 0xC0 for how it reads them
} NJS_MOTION;

typedef struct GjActor {
	NJS_OBJECT* object; // _00
	u8* verts;          // _04
	u32 size;           // _08 of the vertices
	u32 flags;          // _0C 1 keeps float vertices
	u8 _10[0x10];       // _10
} GjActor;

typedef struct GjMesh {
	void* params; // _00
	s32 nbParam;  // _04
	void* list;   // _08 the display list
	u32 size;     // _0C
} GjMesh;

typedef struct GjModel {
	void* verts;       // _00
	void* vtxList;     // _04 see gjCalcActor
	GjMesh* opaque;    // _08
	GjMesh* trans;     // _0C
	u16 nbOpaque;      // _10
	u16 nbTrans;       // _12
	NJS_POINT3 center; // _14 of the bounding sphere
	f32 r;             // _20 the bounding radius
} GjModel;

// Reads one list of keys at a frame
typedef void (*GjKeyFn)(void* keys, u32 n, f32 frame, void* out);

// The running motion gjMotionSelectState picks which one of these
typedef struct GjMotionState {
	u8* base;                            // _00
	s32 size;                            // _04
	s32 total;                           // _08
	void** list0;                        // _0C
	u32* list1;                          // _10
	f32 frame;                           // _14
	u32 nbFrame;                         // _18
	s32 index;                           // _1C
	u32 type;                            // _20
	GjKeyFn readVec;                     // _24
	GjKeyFn readAng;                     // _28
	GjKeyFn _2C;                         // _2C
	GjKeyFn readAngS;                    // _30
	s32 (*getPos)(f32* out);             // _34
	s32 (*getAng)(s32* out);             // _38
	s32 (*getScl)(f32* out);             // _3C
	s32 (*getQuat)(NJS_QUATERNION* out); // _40
	s32 (*getAngS)(s16* out);            // _44
} GjMotionState;

typedef struct GjDrawCalls {
	u8 _00[0xC];
	void (*drawModel)(void*); // _0C
} GjDrawCalls;

// One of six sets of render state
typedef struct NJS_CONTEXT {
	u32 _00;              // _00
	u32 mode;             // _04 0xE0000000 z compare 0x18000000 culling 0x04000000 z write off
	GjTexInfo* tex;       // _08
	u32 blend;            // _0C 0xE0000000 source 0x1C000000 destination 0x18000 clamp
	u32 _10;              // _10
	s32 _14;              // _14
	s32 _18;              // _18 the texture palette
	u32 _1C;              // _1C
	NJS_TEXLIST* texlist; // _20
	u32 _24;              // _24
} NJS_CONTEXT;

// The current perspective see gjSetPerspective
typedef struct GjProjection {
	f32 mtx[16]; // _00
	s32 type;    // _40
	f32 n;       // _44
	f32 f;       // _48
	f32 aspect;  // _4C
	f32 _50;     // _50 half the screen height over the tangent
	f32 _54;     // _54 its inverse
} GjProjection;

#define GX_FIFO_F32 (*(volatile f32*)0xCC008000)
#define GX_FIFO_U32 (*(volatile u32*)0xCC008000)
#define GX_FIFO_S16 (*(volatile s16*)0xCC008000)

// The SDK's vertex inlines Passing values loads all of them before the first write
static inline void GXPosition3f32(f32 x, f32 y, f32 z)
{
	GX_FIFO_F32 = x;
	GX_FIFO_F32 = y;
	GX_FIFO_F32 = z;
}

static inline void GXNormal3f32(f32 x, f32 y, f32 z)
{
	GX_FIFO_F32 = x;
	GX_FIFO_F32 = y;
	GX_FIFO_F32 = z;
}

static inline void GXTexCoord2f32(f32 s, f32 t)
{
	GX_FIFO_F32 = s;
	GX_FIFO_F32 = t;
}

static inline void GXColor1u32(u32 c)
{
	GX_FIFO_U32 = c;
}

// Ninja colours are ARGB GX wants RGBA
#define ARGB_TO_RGBA(c) ((((c) & 0xFFFFFF) << 8) | (((c) & 0xFF000000) >> 24))

// The last error
typedef struct NJS_TEXERRSTAT {
	s32 texno; // _00
	s32 n;     // _04
	s32 err;   // _08
	s32 _0C;   // _0C
	s32 _10;   // _10
	s32 _14;   // _14
	s32 _18;   // _18
	s32 _1C;   // _1C
} NJS_TEXERRSTAT;

// What a GVR file's GVRT chunk starts with
typedef struct GjTexDesc {
	u32 format; // _00 the low byte is the format 0x100 mipmapped 0x200 palette 0x800 flag 0x40000000
	u16 width;  // _04
	u16 height; // _06
} GjTexDesc;

// 2 pi / 65536 The DOL's constant is a float pi taken to double 0x3F1921FB00000000
#define NJ_PI         3.141592f
#define NJ_ANG_TO_RAD (NJ_PI / 32768.0)

// ---- Globals
extern f32* _gj_matrix_;      // the current matrix inside the stack below
extern f32* gjMatrixStackEnd; // the last matrix of the stack
extern f32* gjMatrixStack;    // the first matrix of the stack
extern NJS_CONTEXT* _nj_current_ctx_;
extern GXRenderModeObj* _gj_rmode_;
extern GjProjection gProjection;
extern const s32 gjBlendFactorTable[8]; // context blend factors to GX ones

// ---- SDK
void C_MTXLightFrustum(f32 m[3][4], f32 t, f32 b, f32 l, f32 r, f32 n, f32 scaleS, f32 scaleT, f32 transS, f32 transT);
void PSMTXIdentity(f32* m);
void PSMTXCopy(const f32* src, f32* dst);
void PSMTXConcat(const f32* a, const f32* b, f32* ab);
u32 PSMTXInverse(const f32* src, f32* dst);
void PSMTXTranspose(const f32* src, f32* dst);
void PSMTXRotRad(f32* m, char axis, f32 rad);
void PSMTXTrans(f32* m, f32 x, f32 y, f32 z);
void PSMTXScale(f32* m, f32 x, f32 y, f32 z);
void PSMTXQuat(f32* m, const GXQuaternion* q);
void C_MTXLookAt(f32* m, const NJS_POINT3* pos, const NJS_VECTOR* up, const NJS_POINT3* at);
void C_MTXPerspective(f32* m, f32 fovy, f32 aspect, f32 n, f32 f);
void C_MTXOrtho(f32* m, f32 t, f32 b, f32 l, f32 r, f32 n, f32 f);
void PSMTXMultVec(const f32* m, const NJS_POINT3* src, NJS_POINT3* dst);
void PSMTXMultVecArray(const f32* m, const NJS_POINT3* src, NJS_POINT3* dst, u32 count);
void PSMTXMultVecSR(const f32* m, const NJS_VECTOR* src, NJS_VECTOR* dst);
void PSMTXMultVecArraySR(const f32* m, const NJS_VECTOR* src, NJS_VECTOR* dst, u32 count);
void GXGetProjectionv(f32* p);
void GXGetViewportv(f32* v);
void GXProject(f32 x, f32 y, f32 z, const f32* mtx, const f32* pm, const f32* vp, f32* sx, f32* sy, f32* sz);

void GXSetProjection(const f32* m, int type);
void GXLoadTexMtxImm(const f32* m, u32 id, int type);
void GXLoadPosMtxImm(const f32* m, u32 id);
void GXLoadNrmMtxImm(const f32* m, u32 id);
void GXSetCurrentMtx(u32 id);
void GXSetBlendMode(int type, int src, int dst, int op);
void GXSetCullMode(int mode);
void GXSetArray(int attr, const void* base, u8 stride);
u32 GXGetTexBufferSize(u16 width, u16 height, u32 format, u8 mipmap, u8 maxLod);
void GXSetZMode(int enable, int func, int update);
void GXSetColorUpdate(int enable);
void GXSetViewport(f32 x, f32 y, f32 w, f32 h, f32 n, f32 f);
void GXSetViewportJitter(f32 x, f32 y, f32 w, f32 h, f32 n, f32 f, u32 field);
void GXSetScissor(u32 x, u32 y, u32 w, u32 h);
void GXSetDispCopySrc(u16 x, u16 y, u16 w, u16 h);
void GXSetDispCopyDst(u16 w, u16 h);
u32 GXSetDispCopyYScale(f32 scale);
void GXSetCopyFilter(u8 aa, const u8 pattern[12][2], int vf, const u8 vfilter[7]);
void GXSetPixelFmt(int pix, int z);
void GXCopyDisp(void* dst, int clear);
void GXDrawDone(void);
void GXInvalidateVtxCache(void);
void GXInvalidateTexAll(void);
void* GXInit(void* base, u32 size);
void GXInitTlutRegion(GXTlutRegion* region, u32 addr, int size);
GXTlutRegion* (*GXSetTlutRegionCallback(GXTlutRegion* (*f)(u32)))(u32); // gives back the old one
void GXAdjustForOverscan(GXRenderModeObj*, GXRenderModeObj*, u16, u16);
void GXBegin(int type, int fmt, u16 count);
void GXInitLightAttnK(GXLightObj* lt, f32 k0, f32 k1, f32 k2);
void GXInitLightColor(GXLightObj* lt, GXColor c);
void GXInitLightPos(GXLightObj* lt, f32 x, f32 y, f32 z);
void GXInitLightDir(GXLightObj* lt, f32 x, f32 y, f32 z);
void GXLoadLightObjImm(const GXLightObj* lt, u32 id);
void GXSetChanAmbColor(int chan, GXColor c);
void GXSetFog(int type, f32 start, f32 end, f32 n, f32 f, GXColor c);
void GXSetNumChans(u8 n);
void GXSetChanCtrl(int chan, u8 enable, int amb, int mat, u32 lights, int diff, int attn);
void GXSetNumTevStages(u8 n);
void GXSetTevOrder(int stage, int coord, int map, int color);
void GXSetTevOp(int stage, int mode);
void GXSetTevColorIn(int stage, int a, int b, int c, int d);
void GXSetTevAlphaIn(int stage, int a, int b, int c, int d);
void GXSetTevColorOp(int stage, int op, int bias, int scale, int clamp, int out);
void GXSetTevAlphaOp(int stage, int op, int bias, int scale, int clamp, int out);
void GXSetAlphaCompare(int a, u8 ref0, int op, int b, u8 ref1);
void GXSetZCompLoc(int before);
void GXSetNumTexGens(u8 n);
void GXSetTexCoordGen2(int dst, int func, int src, u32 mtx, int norm, u32 post);
void GXSetLineWidth(u8 width, int offset);
void GXClearVtxDesc(void);
void GXSetVtxDesc(int attr, int type);
void GXSetVtxAttrFmt(int fmt, int attr, int cnt, int type, u8 frac);
void GXInitTexObj(GXTexObj* obj, void* image, u16 w, u16 h, int fmt, int s, int t, u8 mipmap);
void GXInitTexObjCI(GXTexObj* obj, void* image, u16 w, u16 h, int fmt, int s, int t, u8 mipmap, u32 tlut);
void GXLoadTexObj(GXTexObj* obj, int id);
void GXInitTexObjLOD(GXTexObj* obj, int minFilt, int magFilt, f32 minLod, f32 maxLod, f32 bias, u8 clamp, u8 edge, int aniso);

void LCEnable(void);

void* memset(void* dst, int value, u32 size);
double sin(double x);
double cos(double x);
f32 sqrtf(f32 x);

// ---- Ninja
void syMemCopy(void* dst, const void* src, s32 size);
f32 njSin(s32 ang);
f32 njCos(s32 ang);
f32 njFraction(f32 n);
void njSinCos(s32 ang, f32* s, f32* c);
void njRotateX(f32* m, s32 ang);
void njRotateY(f32* m, s32 ang);
void njRotateZ(f32* m, s32 ang);
void __nj_unit_matrix(f32* m);
void __internal_njTranslate(f32* m, f32 x, f32 y, f32 z);
void njUnitMatrix(f32* m);
void syMemSet(void* dst, u8 value, u32 size);
void gjStartVertex2D(void);
void gjStartVertex3D(void);
void gjStartVertexNonTexOpaque(void);
void gjStartVertexNonTexTrans(void);
void gjStartVertexTexOpaque(void);
void gjStartVertexTexTrans(void);
void gjStartVertexTexOpaqueEx(void);
void gjStartVertexTexTransEx(void);
void gjEndVertex(void);
void gjLoadTextureNum(s32 num, s32 a, s32 b, s32 c);
void gjSetPolygonCullingMode(void);
void gjSetZBufferMode(s32 update);
void gjGXSetBlendMode(s32 enable);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
void C_MTXLightPerspective(f32* mtx, f32 fovY, f32 aspect, f32 scaleS, f32 scaleT, f32 transS, f32 transT);
void GXInitFogAdjTable(GXFogAdjTable* table, u16 width, const f32* projMtx);
void GXInitLightDistAttn(GXLightObj* obj, f32 refDist, f32 refBrightness, int distFunc);
void GXSetChanMatColor(int channel, GXColor color);
void GXSetCopyClear(GXColor clearColor, u32 clearZ);
void GXSetIndTexOrder(int stage, int texCoord, int texMap);
void GXSetNumIndStages(u8 stageCount);
void GXSetTevColor(int reg, GXColor color);
void GXSetTevDirect(int stage);
void GXSetTevKColor(int id, GXColor color);
void GXSetTevKColorSel(int stage, int sel);
void GXSetTexCopyDst(u16 width, u16 height, int format, GXBool useMIPmap);
void PSMTXRotTrig(f32* mtx, char axis, f32 sinA, f32 cosA);
void PSMTXTransApply(const f32* src, f32* dest, f32 xT, f32 yT, f32 zT);
void GXCallDisplayList(void* list, u32 numBytes);
void GXInitLightSpot(GXLightObj* obj, f32 cutoff, int spotFunc);
void GXSetIndTexCoordScale(int stage, int sScale, int tScale);
void GXSetIndTexMtx(int mtx, const f32* offsets, s8 scale);
void GXSetTevIndWarp(int tevStage, int indStage, GXBool isSignedOffset, GXBool isReplaceMode, int mtx);
void GXSetTevKAlphaSel(int stage, int sel);
void GXGetCullMode(int* mode);
void GXSetFogRangeAdj(GXBool doEnable, u16 center, GXFogAdjTable* table);
#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
extern "C" {
void gjTranslate(f32, f32, f32);
void gjRotateX(s32);
void gjRotateZ(int);
f32 njScalor(Vec3f*);
s32 njSetTextureNum(s32);
s32 njPushMatrixEx();
s32 njPopMatrixEx();
void njTranslateEx(const Vec3f*);
void njMultiMatrix(f32*, f32*);
void njGetMatrix(f32*);
void gjRotateY(s32);
void gjScaleVector(f32, Vec3f*, const Vec3f*);
void gjAddVector(Vec3f*, const Vec3f*, const Vec3f*);
void njAddVector(Vec3f*, const Vec3f*);
void gjSetNumPostTexGens(int);
void gjSetNumPostTevStages(int);
void gjDrawActor(GjActor*);
void gjSubVector(Vec3f*, const Vec3f*, const Vec3f*);
f32 njUnitVector(Vec3f*);
void njUnitRotPortion(f32*);
void njCreateQuaternion(s32, f32, f32, f32, NJS_QUATERNION*);
void njSubVector(Vec3f*, const Vec3f*);
void gjDrawObject(NJS_OBJECT*);
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
void GXSetDrawSync(u16 token);
u16 GXReadDrawSync(void);
void GXPeekZ(u16 x, u16 y, u32* z);
#ifdef __cplusplus
}
#endif

/* The game's view of these C++ only because the middleware's own .c files declare them themselves */
#ifdef __cplusplus
extern "C" {
struct GjSysParams;
struct GjShape;
struct NJS_LINE;
struct NJS_POLYGON_VTX;
struct NJS_QUAD_TEXTURE_EX;
struct NJS_TEXTURE_VTX;
struct GjCameraBase;
struct GjCameraOut;
struct GjPrintEntry;
struct GjPaletteFile;
struct GjPalette;
struct NJS_MOTION_LINK;
typedef void (*GjStageCallback)(s32 first);
s32 _gjDrawModel(GjModel*);
s32 gjTexListDecode(NJS_TEXLIST*, u32, u32*, s32*);
int gjTexFindFree(GjTexMem**, GjTexHeader**);
void gjTexListSetSlot(NJS_TEXLIST*, s32, s32);
s32 gjTexListBind(NJS_TEXLIST*, s32, u32, s32, const GjTexDesc*, s32*);
s32 gjTexSurfaceSetup(GjTexInfo*, int, int, int);
u32 gjTexCalcSize(int, int, u32);
u32 gjTexDescSize(GjTexDesc*);
void gjSetBlendOverride(s32, s32, s32, s32);
void njInitDevice();
void syPadInit(int, int, int);
void njMiscInit(int, int);
u32 gjActorSize(NJS_OBJECT*);
void gjBeforeRender();
void gjBeginCalcActor(GjActor*);
void gjBeginDrawModel();
void gjBuildActorVertices(GjActor*);
s32 gjCalcActor(GjModel*);
u32 gjCalcFrameBufferSize(GXRenderModeObj*);
void gjChangeDrawObjIF(void* const*, u32);
u8* gjCreateActor(NJS_OBJECT*, u8*);
u8* gjCreateActorEx(NJS_OBJECT*, u8*, int);
void gjDisableFog();
void gjDoneRender();
void gjDrawModel(GjModel*);
void gjDrawMotion(NJS_OBJECT*, void*, f32);
void gjDrawMotionLink(NJS_OBJECT*, void*, f32);
void gjEnableFog();
void gjEndCalcActor(GjActor*);
void gjEndDrawModel();
int gjGetMotionNodeData(NJS_OBJECT*, f32*, s32*, f32*, f32*);
int gjGetPostTexGenStartID();
void gjInitMatrix(s32, int);
void gjInitSystem(GjSysParams*);
void gjLoadLight(s32);
void gjLoadModelView(s32);
void gjLoadNrmMatrix(s32);
void gjLoadPosMatrix(s32);
void gjLoadTexture(NJS_TEXLIST*, int, int, int, int);
void gjMotionActor(f32, GjActor*, void*);
void gjMotionLinkActor(GjActor*, void*, f32);
void gjMulAddVector(f32, Vec3f*, const Vec3f*, const Vec3f*);
void gjPrintFlush();
void gjPushUnitMatrix();
void gjSetAmbient(f32, f32, f32);
void gjSetFog();
void gjSetFogColor(u32);
void gjSetFogCurve(u32);
void gjSetFogRange(f32, f32);
void gjSetLightColor(int, f32, f32, f32);
void gjSetLightSwitch(s32, s32);
void gjSetLightVector(int, f32, f32, f32);
void gjSetMotionCallback(void (*f)(NJS_OBJECT*));
void gjSetNumPostChans(int);
void gjSetPaletteData(int, u16, int, void*);
void gjSetPerspective(s32, f32, f32, f32);
void gjSetPostChanCallback(GjStageCallback);
void gjSetRenderMode(s32);
void gjSetTexture();
void gjShapeActor(GjActor*, void*, GjShape*, f32);
void gjSwapBuffers();
void njCalcPoint(const f32*, const Vec3f*, Vec3f*);
void njCalcPointEx(const Vec3f*, const Vec3f*);
void njCalcPoints(f32*, const Vec3f*, Vec3f*, int);
s32 njCalcScreen(const Vec3f*, const f32*, f32*);
void njCalcVector(f32*, const Vec3f*, Vec3f*);
void njCalcVectorEx(const Vec3f*, Vec3f*);
void njCalcVectors(f32*, const Vec3f*, Vec3f*, int);
void njCameraExLookAt(const Vec3f*, const Vec3f*, const Vec3f*);
void njColorBlendingMode(s32, s32);
void njCreateQuaternionV(const Vec3f*, int, NJS_QUATERNION*);
f32 njDistanceP2L(const Vec3f*, const NJS_LINE*, Vec3f*);
f32 njDistanceP2P(const Vec3f*, const Vec3f*);
void njDrawLine2DExSetList(const f32*, f32, int);
void njDrawLine3DExSetList(const f32*, int);
void njDrawLine3DExSetStrip(const Vec3f*, int);
void njDrawLineExEnd();
void njDrawLineExStart(f32, u32, int);
void njDrawPolygon2DExEnd();
void njDrawPolygon2DExSetData(const NJS_POLYGON_VTX*, s32);
void njDrawPolygon2DExStart(s32);
void njDrawPolygon3DExEnd();
void njDrawPolygon3DExSetData(const NJS_POLYGON_VTX*, s32);
void njDrawPolygon3DExStart(s32);
void njDrawQuadTexture(const NJS_QUAD_TEXTURE*, f32);
void njDrawQuadTextureEx(const NJS_QUAD_TEXTURE_EX*);
void njDrawTexture2DExEnd();
void njDrawTexture2DExSetData(const NJS_TEXTURE_VTX*, int);
void njDrawTexture2DExStart(s32);
void njDrawTexture3DExEnd();
void njDrawTexture3DExSetData(const NJS_TEXTURE_VTX*, int);
void njDrawTexture3DExStart(s32);
void njGetCameraMotion(const GjCameraBase*, void*, const GjCameraOut*, f32);
NJS_TEXLIST* njGetCurrentTexList();
int njGetMotionLinkNodeData(NJS_OBJECT*, f32*, s32*, f32*, NJS_QUATERNION*);
int njGetMotionNodeData(NJS_OBJECT*, f32*, s32*, f32*, NJS_QUATERNION*);
void njGetTranslation(f32*, Vec3f*);
void njInitCameraEx();
void njInitPrint(GjPrintEntry*, int, int);
void njInitTextureEx(void*, s32, void*, s32);
f32 njInnerProduct(const Vec3f*, const Vec3f*);
void njInterpolateQuaternion(const NJS_QUATERNION*, const NJS_QUATERNION*, NJS_QUATERNION*, f32);
void njInvertMatrix(f32*);
s32 njLoadPaletteData(const GjPaletteFile*, GjPalette*);
s32 njLoadTexture(NJS_TEXLIST*);
void njMotionLinkTransformEx(NJS_OBJECT*);
void njMotionTransformEx(NJS_OBJECT*);
void njMultiQuaternion(const NJS_QUATERNION*, const NJS_QUATERNION*, NJS_QUATERNION*);
f32 njOuterProduct(const Vec3f*, const Vec3f*, Vec3f*);
void njPolygonCullingMode(u32);
void njPrint(s32, const char*, ...);
void njPrintC(s32, const char*);
void njPrintColor(u32);
void njPrintD(s32, s32, s32);
void njPrintF(s32, s32, f32);
void njPrintSize(u16);
void njProjectScreen(f32*, const Vec3f*, NJS_POINT2*);
s32 njPushMatrix(const f32*);
void njQuadTextureEnd();
void njQuadTextureStart(s32);
void njQuaternionEx(const NJS_QUATERNION*);
void njQuaternionEx2(f32, const Vec3f*);
s32 njReleaseTexture(NJS_TEXLIST*);
void njRotateEx(const s32*, s32);
void njRotateXCameraEx(s32);
void njRotateZXY(f32*, s32, s32, s32);
void njScale(f32*, f32, f32, f32);
void njScaleEx(const Vec3f*);
f32 njScalor2(const Vec3f*);
void njSetConstantMaterial(const f32*);
void njSetMatrix(f32*, f32*);
void njSetNextMotionLinkNode();
void njSetNextMotionNodeEx();
void njSetQuadTexture(s32, u32);
void njSetQuadTextureColor(u32);
void njSetTextureInfo(NJS_TEXINFO*, void*, s32, s32, s32);
void njSetTextureName(NJS_TEXNAME*, void*, void*, u32);
void njStartMotionLink(const NJS_MOTION_LINK*, f32);
void njStartMotionObj(void*, f32);
void njTextureClampMode(u32);
void njTranslate(f32*, f32, f32, f32);
void njTranslateCameraEx(f32, f32, f32);
void njXYZAngleToQuaternion(const s32*, NJS_QUATERNION*);
void njZBufferMode(s32);
void njZWriteMode(s32);
void njZXYAngleToQuaternion(const s32*, NJS_QUATERNION*);
void* syCalloc(u32, u32);
void syFree(void*);
void* syMalloc(u32);
void syMallocInit(void*, u32);
void syMallocStat(s32*, s32*);
void syMemSet4(void*, u32, s32);
u32 syTmrCountToMicro(u32);
u32 syTmrDiffCount(u32, u32);
u32 syTmrGetCount();
}
#endif

#endif
