#ifndef _NINJA_GJPRIM_H
#define _NINJA_GJPRIM_H

#include "Ninja/ninja.h"

typedef struct NJS_TEXTURE_VTX {
	f32 x, y, z; // _00
	f32 u, v;    // _0C
	u32 col;     // _14
} NJS_TEXTURE_VTX;

typedef struct NJS_POLYGON_VTX {
	f32 x, y, z;
	u32 col;
} NJS_POLYGON_VTX;

// One shape motion that's running
typedef struct GjShapeState {
	u8* base;     // _00
	s32 size;     // _04 of the first half
	s32 total;    // _08
	void** list0; // _0C
	void** list1; // _10
	f32 frame;    // _14
	void* _18;    // _18
	s32 index;    // _1C
	s32 type;     // _20
} GjShapeState;

typedef struct GjShape {
	u8* base;  // _00
	void* _04; // _04
	u16 type;  // _08
	u16 nodes; // _0A
} GjShape;

typedef struct GjKey {
	u32 _00;
	void* data; // _04
} GjKey;

void gjSetTexture(void);
void njDrawTexture3DExStart(s32 trans);
void njDrawTexture3DExSetData(const NJS_TEXTURE_VTX* v, s32 count);
void njDrawTexture3DExEnd(void);
void gjVtxFmtColor(void);
void gjVtxFmtColorTex(void);
void gjVtxFmtColorSpecTex(void);
void gjSetNumChannels(s32 n);
void gjPrimZOpaque(void);
void gjPrimZTrans(void);
void gjPrimTevPass(void);
void gjPrimTevStages(s32 n);
void gjPrimNoTexGen(void);
void gjPrimOneTexGen(void);
void gjPrimTexCoordGen(s32 dst, s32 func, s32 src, s32 mtx);
void gjDrawLinkObj(NJS_OBJECT* obj);
void gjDrawLinkTree(NJS_OBJECT* obj);
void gjShapeBegin(GjShape* shape, f32 frame);
void gjShapeSelectState(s32 index);
void gjDrawShapeTree(NJS_OBJECT* obj);
void gjShapeApplyModel(void* model);
void gjShapeBlendKeys(void* a, void* b, void* model, f32 frame);
void gjShapeNextList(void);
void gjPrimGXEnd(void);
void gjPrimGXTexCoord2f32(f32 u, f32 v);
void gjPrimGXColor1u32(u32 col);
void gjPrimGXPosition3f32(f32 x, f32 y, f32 z);
void gjPrimGXEndB(void);
void gjPrimGXColor1u32B(u32 col);
void gjPrimGXPosition3f32B(f32 x, f32 y, f32 z);
void gjPrimGXEndC(void);
void gjPrimGXTexCoord2f32C(f32 u, f32 v);
void gjPrimGXColor1u32C(u32 col);
void gjPrimGXPosition3f32C(f32 x, f32 y, f32 z);
void gjPrimGXEndD(void);
void gjPrimGXColor1u32D(u32 col);
void gjPrimGXPosition3f32D(f32 x, f32 y, f32 z);
void gjLineModeSet2D(void);
void gjLineModeSet3D(void);

extern GXTexObj gjTexObj0;
extern GXTexObj gjTexObj1;
extern s32 gjLineMode;
extern u32 gjLineColor;
extern GjShapeState* gjCurShapeState;
extern GjShapeState gjShapeStates[2];

#endif
