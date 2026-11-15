#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"

typedef struct GjDrawState {
	u8 _00[8];         // _00
	s32 _08[11];       // _08
	s32 _34[11];       // _34
	u8 _60[0xC];       // _60
	s32 _6C;           // _6C
	u8 _70[8];         // _70
	s32 _78;           // _78
	u8 numChans;       // _7C
	u8 numTexGens;     // _7D
	u8 numTevStages;   // _7E
	u8 preChans;       // _7F
	u8 preTexGens;     // _80
	u8 preTevStages;   // _81
	u8 postChans;      // _82
	u8 postTexGens;    // _83
	u8 postTevStages;  // _84
	u8 totalChans;     // _85
	u8 totalTexGens;   // _86
	u8 totalTevStages; // _87
} GjDrawState;

typedef void (*GjStageCallback)(s32 first);

GjDrawState gjDrawState;
extern void* gjDrawFuncs[8]; // the current draw functions
extern void* gjDefaultDrawFuncs[8]; // the default ones
extern GjStageCallback gjPreStageCb0;
extern GjStageCallback gjPreStageCb1;
extern GjStageCallback gjPreStageCb2;
extern GjStageCallback gjPostStageCb0;
extern GjStageCallback gjPostStageCb1;
extern GjStageCallback gjPostStageCb2;
extern s32 gjDrawPass; // 1 draw opaque meshes 2 draw transparent ones

void gjDrawListBegin(s32 (*f)(GjModel*));
void gjDrawListFlush(void);
s32 _gjDrawModel(GjModel* model);
void gjBeginDrawModel(void);
void gjEndDrawModel(void);
void gjLoadModelView(s32 id);

// One parameter of a mesh
typedef struct GjMeshParam {
	u8 type;   // _00
	u32 value; // _04
} GjMeshParam;

// One vertex array of a model
typedef struct GjVtxArray {
	u8 type;    // _00
	u8 stride;  // _01
	u8 _02[6];
	void* data; // _08
	u8 _0C[4];
} GjVtxArray;

// The calls gjChangeDrawObjIF can replace as they get used here
#define DRAW_IF_CHANCOLOR ((void (*)(s32, GXColor))gjDrawFuncs[1])
#define DRAW_IF_TEVORDER ((void (*)(s32, s32, s32, s32))gjDrawFuncs[3])
#define DRAW_IF_TEXGEN ((void (*)(s32, s32, s32, s32))gjDrawFuncs[4])
#define DRAW_IF_CHANCTRL ((void (*)(s32, s32, s32, s32, u32))gjDrawFuncs[5])
#define DRAW_IF_BLEND ((void (*)(s32, s32, s32, s32))gjDrawFuncs[6])
#define DRAW_IF_TEXTURE ((void (*)(s32, s32, s32, s32))gjDrawFuncs[7])

void gjDrawListAdd(NJS_OBJECT* obj, const f32* m);
void gjDrawListDraw(u32 count);
void gjDrawTree(NJS_OBJECT* obj);
void gjMeshRunParams(GjMesh* mesh);
void gjSetVtxArrays(GjVtxArray* v);
void gjModelCheckState(GjModel* model);
void gjSetBlendOverride(s32 a, s32 b, s32 c, s32 d);
u32 gjVtxDescField(u32 value, s32 index);
void gjParamVtxDesc(u32 value);
void gjSetChanFlags(s32 chan, u32 flags);
void gjParamDiffuse(u32 col);
void gjParamStubA(u32 value);
void gjParamStubB(u32 value);
void gjParamBlend(u32 value);
void gjParamVtxFmt(u32 value);
void gjParamTexture(u32 value);
void gjParamTevOrder(u32 value);
void gjParamTexGen(u32 value);
void gjSetChanCtrl(s32 chan, u8 enable, s32 amb, s32 mat, u32 lights);
void gjSetChanMatColor(s32 chan, GXColor c);
void gjSetTevStage(s32 stage, s32 coord, s32 map, s32 color);
void gjSetAlphaRef(u8 ref);
void gjSetTexGen(s32 dst, s32 func, s32 src, u32 mtx);
void gjDrawMotionTree(NJS_OBJECT* obj);
void gjDrawMotionObj(NJS_OBJECT* obj);
void gjSetBlendMode(u32 flags);

// By parameter type
#define MESH_PARAM(f) (void (*)(u32))f
void (*gjMeshParamFuncs[11])(u32) = {
	MESH_PARAM(gjParamVtxFmt), MESH_PARAM(gjParamVtxDesc), MESH_PARAM(gjSetBlendMode), 0,
	MESH_PARAM(gjParamBlend), MESH_PARAM(gjParamDiffuse), MESH_PARAM(gjParamStubA), MESH_PARAM(gjParamStubB),
	MESH_PARAM(gjParamTexture), MESH_PARAM(gjParamTevOrder), MESH_PARAM(gjParamTexGen),
};

void* gjDrawFuncs[8] = { 0, gjSetChanMatColor, 0, gjSetTevStage, gjSetTexGen, gjSetChanCtrl, GXSetBlendMode, gjLoadTextureNum };

void* gjDefaultDrawFuncs[8] = { 0, gjSetChanMatColor, 0, gjSetTevStage, gjSetTexGen, gjSetChanCtrl, GXSetBlendMode, gjLoadTextureNum };

u32 gjVtxAttrTable[] = {
	0x00000000, 0x00000009, 0x0000000A, 0x0000000B, 0x0000000C, 0x0000000D, 0x0000000E, 0x0000000F,
	0x00000010, 0x00000011, 0x00000012, 0x00000013, 0x00000014, 0x000000FF,
};
u32 gjTexCoordTable[] = {
	0x00000000, 0x00000001, 0x00000002, 0x00000003, 0x00000004, 0x00000005, 0x00000006, 0x00000007,
	0x000000FF,
};
u32 gjTexMapTable[] = {
	0x00000000, 0x00000001, 0x00000002, 0x00000003, 0x00000004, 0x00000005, 0x00000006, 0x00000007,
	0x000000FF,
};
u32 gjTexCompCntTable[] = {
	0x00000000, 0x00000001, 0x00000000, 0x00000001, 0x00000002, 0x00000000, 0x00000001, 0x00000000,
	0x00000001,
};
u32 gjTexCoordTypeTable[] = {
	0x00000000, 0x00000001, 0x00000002, 0x00000003, 0x00000004, 0x00000000, 0x00000001, 0x00000002,
	0x00000003, 0x00000004, 0x00000005,
};
u32 gjPosMtxTable[] = {
	0x0000001E, 0x00000021, 0x00000024, 0x00000027, 0x0000002A, 0x0000002D, 0x00000030, 0x00000033,
	0x00000036, 0x00000039, 0x0000003C,
};
u32 gjBlendOverride = 0x250F;    // a blend value that overrides the meshes'

// 0x80211CF4
// Walks the tree Flags 1 2 and 4 skip the translation rotation and scale and 7 skips the matrix too
// 8 skips the model and 0x10 the children
void gjDrawTree(NJS_OBJECT* obj)
{
	do {
		u32 flags = obj->evalflags;
		if ((flags & 7) == 7) {
			if ((flags & 8) == 0) {
				gjDrawListAdd(obj, _gj_matrix_);
			}
			if ((flags & 0x10) == 0) {
				gjDrawTree(obj->child);
			}
		} else {
			njPushMatrixEx();
			if ((flags & 1) == 0) {
				njTranslateEx(obj->pos);
			}
			if ((flags & 2) == 0) {
				if (flags & 0x400) {
					njQuaternionEx2(obj->quatRe, obj->ang);
				} else {
					njRotateEx(obj->ang, flags & 0x20);
				}
			}
			if ((flags & 4) == 0) {
				njScaleEx(obj->scl);
			}
			if ((flags & 8) == 0) {
				gjDrawListAdd(obj, _gj_matrix_);
			}
			if ((flags & 0x10) == 0) {
				gjDrawTree(obj->child);
			}
			njPopMatrixEx();
		}
		obj = obj->sibling;
	} while (obj != NULL);
}

// 0x80211E00
void gjDrawObject(NJS_OBJECT* obj)
{
	NJS_MATRIX scale;
	NJS_MATRIX trans;

	if ((u32)(_nj_control_3d_flag_ & 0x100000) != 0) {
		PSMTXScale(scale, -0.5f, -0.5f, 0.0f);
		PSMTXTrans(trans, 0.5f, 0.5f, 1.0f);
		PSMTXConcat(trans, scale, scale);
		GXLoadTexMtxImm(scale, 0x40, 0);
	}
	gjBeginDrawModel();
	gjDrawListBegin(_gjDrawModel);
	gjDrawTree(obj);
	gjEndDrawModel();
	gjDrawListFlush();
}

// 0x80211EA0
void gjDrawModel(GjModel* model)
{
	gjLoadModelView(0);
	gjBeginDrawModel();
	_gjDrawModel(model);
	gjEndDrawModel();
}

// 0x80211EE0
// Runs a mesh's parameters skipping any that wouldn't change anything
#pragma opt_common_subs off
void gjMeshRunParams(GjMesh* mesh)
{
	u32 type;
	u32 diff;
	s32* changed;
	s32* last;
	GjMeshParam* p;
	s32 n;
	u32 value;

	p = mesh->params;
	n = mesh->nbParam;

	while (n > 0) {
		type = p->type;
		value = p->value;
		last = &gjDrawState._08[type];
		diff = value ^ *last;
		changed = &gjDrawState._34[type];
		diff |= *changed;

		*changed = diff;
		if (diff != 0) {
			gjMeshParamFuncs[type](value);
		}
		*last = value;
		*changed = 0;
		n--;
		p++;
	}
}
#pragma opt_common_subs reset

// 0x80211F88
void gjSetVtxArrays(GjVtxArray* v)
{
	while (v->type != 0xFF) {
		GXSetArray(gjVtxAttrTable[v->type], v->data, v->stride);
		v++;
	}
}

// 0x80211FE8
void gjModelCheckState(GjModel* model)
{
	if (((u32)gjDrawState._6C ^ (u32)_nj_control_3d_flag_) & 0x80000) {
		gjDrawState._6C = _nj_control_3d_flag_;
		gjDrawState._34[2] |= 0xF00F3;
		gjDrawState._34[9] = -1;
		gjDrawState._34[10] = -1;
	}
}

// 0x8021202C
s32 _gjDrawModel(GjModel* model)
{
	GjMesh* mesh;
	s32 n;

	gjModelCheckState(model);
	if (model->verts != NULL) {
		gjSetVtxArrays(model->verts);
	}
	if (gjDrawPass & 1) {
		n = model->nbOpaque;
		mesh = model->opaque;
		while (n > 0) {
			gjMeshRunParams(mesh);
			GXCallDisplayList(mesh->list, mesh->size);
			n--;
			mesh++;
		}
	}
	if (gjDrawPass & 2) {
		n = model->nbTrans;
		mesh = model->trans;
		while (n > 0) {
			gjMeshRunParams(mesh);
			GXCallDisplayList(mesh->list, mesh->size);
			n--;
			mesh++;
		}
	}
	return 0;
}

// 0x802120FC
void gjBeginDrawModel(void)
{
	s32 i;
	for (i = 0; i < 11; i++) {
		gjDrawState._08[i] = 0;
		gjDrawState._34[i] = -1;
	}
	gjDrawState._6C = _nj_control_3d_flag_;
	gjDrawState._78 = -1;
}

// 0x802121B4
void gjEndDrawModel(void)
{
}

// 0x802121B8
void gjLoadModelView(s32 id)
{
	NJS_MATRIX scale;
	NJS_MATRIX trans;
	NJS_MATRIX m;

	GXLoadPosMtxImm(_gj_matrix_, id);
	GXLoadNrmMtxImm(_gj_matrix_, id);
	if ((u32)_nj_control_3d_flag_ & 0x100000) {
		PSMTXScale(scale, -0.5f, -0.5f, 0.0f);
		PSMTXTrans(trans, 0.5f, 0.5f, 1.0f);
		PSMTXInverse(_gj_matrix_, m);
		PSMTXTranspose(m, m);
		PSMTXConcat(scale, _gj_matrix_, m);
		PSMTXConcat(trans, m, m);
		GXLoadTexMtxImm(m, 0x2A, 0);
	}
}

// 0x80212278
void gjSetRenderMode(s32 mode)
{
	gjDrawPass = mode;
}

#define CHANGE_DRAW_IF(bit, index)                       \
	if (mask & (bit)) {                                  \
		if (funcs[index] == NULL) {                      \
			gjDrawFuncs[index] = gjDefaultDrawFuncs[index];   \
		} else {                                         \
			gjDrawFuncs[index] = funcs[index];          \
		}                                                \
	}

// 0x80212280
// Each bit of the mask replaces one function or puts the default back if the new one is null
void gjChangeDrawObjIF(void* const* funcs, u32 mask)
{
	CHANGE_DRAW_IF(1, 0)
	CHANGE_DRAW_IF(2, 1)
	CHANGE_DRAW_IF(4, 2)
	CHANGE_DRAW_IF(8, 6)
	CHANGE_DRAW_IF(0x10, 7)
	CHANGE_DRAW_IF(0x20, 3)
	CHANGE_DRAW_IF(0x40, 4)
	CHANGE_DRAW_IF(0x80, 5)
}

// 0x80212484
void gjSetNumPostChans(int n)
{
	gjDrawState.postChans = (u8)n;
}

// 0x80212498
void gjSetNumPostTexGens(int n)
{
	gjDrawState.postTexGens = (u8)n;
}

// 0x802124AC
void gjSetNumPostTevStages(int n)
{
	gjDrawState.postTevStages = (u8)n;
}

// 0x802124C0
void gjSetPostChanCallback(GjStageCallback f)
{
	gjPostStageCb0 = f;
}

// 0x802124C8
void gjSetPostTexGenCallback(GjStageCallback f)
{
	gjPostStageCb1 = f;
}

// 0x802124D0
void gjSetPostTevStageCallback(GjStageCallback f)
{
	gjPostStageCb2 = f;
}

// 0x802124D8
u8 gjGetPostTexGenStartID(void)
{
	return gjDrawState.numTexGens + gjDrawState.preTexGens;
}

// 0x802124F4
void gjSetBlendOverride(s32 a, s32 b, s32 c, s32 d)
{
	gjBlendOverride = a << 14 | b << 11 | c << 8 | d;
}

// 0x80212514
// Two bits per attribute
u32 gjVtxDescField(u32 value, s32 index)
{
	return (value >> (index * 2)) & 3;
}

// 0x80212524
void gjParamVtxDesc(u32 value)
{
	GXClearVtxDesc();
	GXSetVtxDesc(9, gjVtxDescField(value, 1));
	GXSetVtxDesc(10, gjVtxDescField(value, 2));
	GXSetVtxDesc(11, gjVtxDescField(value, 3));
	GXSetVtxDesc(12, gjVtxDescField(value, 4));
	GXSetVtxDesc(13, gjVtxDescField(value, 5));
}

// 0x802125C8
void gjSetChanFlags(s32 chan, u32 flags)
{
	s32 enable;
	s32 amb;
	s32 mat;

	enable = (flags & 0x100) ? 0 : 1;
	amb = (flags & 0x1000) ? 1 : 0;
	mat = ((u32)_nj_control_3d_flag_ & 0x30) ? 0 : ((flags & 0x800) ? 1 : 0);
	DRAW_IF_CHANCTRL(chan + 4, enable, amb, mat, gjLightsOn);
}

// 0x80212664
// flags is the mesh's 3 number of channels 0xF0 number of texture coordinates 0xF0000 number of stages 0x8000 no
// culling 0x2000 transparent and 0x4000 punch through
void gjSetBlendMode(u32 flags)
{
	u32 changed = gjDrawState._34[2];
	s32 count;

	if (changed & 0x8000) {
		if (flags & 0x8000) {
			GXSetCullMode(0);
		} else {
			gjSetPolygonCullingMode();
		}
	}
	if (changed & 0x6000) {
		if (flags & 0x2000) {
			if (flags & 0x4000) {
				gjSetAlphaRef(0);
				GXSetZCompLoc(0);
				gjSetZBufferMode(0);
			} else {
				gjSetAlphaRef((u8)gDrawAlphaRef);
				GXSetZCompLoc(0);
				gjSetZBufferMode(1);
			}
		} else {
			GXSetZCompLoc(1);
			gjSetZBufferMode(1);
		}
	}
	if (changed & 0x1903) {
		s32 i;
		s32 n;
		s32 pre;
		s32 post;
		n = flags & 3;
		if ((u32)_nj_control_3d_flag_ & 0x80000) {
			pre = gjDrawState.preChans;
			post = gjDrawState.postChans;
			gjDrawState.numChans = n;
			count = pre + n;
			gjDrawState.totalChans = post + count;
			GXSetNumChans(post + count);
			if (pre != 0 && gjPreStageCb0 != NULL) {
				gjPreStageCb0(0);
			}
			for (i = 0; i < n; i++) {
				gjSetChanFlags(pre + i, flags);
			}
			if (post != 0 && gjPostStageCb0 != NULL) {
				gjPostStageCb0((u8)count);
			}
		} else {
			GXSetNumChans(n);
			for (count = 0; count < n; count++) {
				gjSetChanFlags(count, flags);
			}
		}
	}
	if (changed & 0xF0) {
		s32 pre;
		s32 post;
		u8 gens = (flags & 0xF0) >> 4;
		if ((u32)_nj_control_3d_flag_ & 0x80000) {
			pre = gjDrawState.preTexGens;
			post = gjDrawState.postTexGens;
			gjDrawState.numTexGens = gens;
			count = pre + gens;
			gjDrawState.totalTexGens = post + count;
			GXSetNumTexGens(post + count);
			if (pre != 0 && gjPreStageCb1 != NULL) {
				gjPreStageCb1(0);
			}
			if (post != 0 && gjPostStageCb1 != NULL) {
				gjPostStageCb1((u8)count);
			}
		} else {
			GXSetNumTexGens(gens);
		}
		gjDrawState._34[10] = -1;
	}
	if ((u32)_nj_control_3d_flag_ & 0x80000) {
		s32 n;
		s32 pre;
		s32 post;
		n = (flags & 0xF0000) >> 16;
		pre = gjDrawState.preTevStages;
		post = gjDrawState.postTevStages;
		if (changed & 0xF0000) {
			gjDrawState.numTevStages = n;
			gjDrawState.totalTevStages = pre + n + post;
			GXSetNumTevStages(pre + n + post);
			gjDrawState._34[9] = -1;
		}
		if (changed & 0xF00F3) {
			if (pre != 0 && gjPreStageCb2 != NULL) {
				gjPreStageCb2(0);
			}
			if (post != 0 && gjPostStageCb2 != NULL) {
				gjPostStageCb2((u8)(pre + n));
			}
		}
	} else if (changed & 0xF0000) {
		GXSetNumTevStages((flags & 0xF0000) >> 16);
		gjDrawState._34[9] = -1;
	}
}

// 0x8021299C
// 0x10 uses the ambient colour alone 0x20 adds it
void gjParamDiffuse(u32 col)
{
	GXColor c;
	f32 f[4];
	u32 mode = _nj_control_3d_flag_;

	if (mode & 0x10) {
		gjColorFromF32(_nj_constant_material_, &c);
	} else if (mode & 0x20) {
		c.r = (col & 0xFF000000) >> 24;
		c.g = (col & 0xFF0000) >> 16;
		c.b = (col & 0xFF00) >> 8;
		c.a = col & 0xFF;
		gjColorToF32(&c, f);
		f[0] = f[0] + _nj_constant_material_[0];
		f[1] = f[1] + _nj_constant_material_[1];
		f[2] = f[2] + _nj_constant_material_[2];
		f[3] = f[3] + _nj_constant_material_[3];
		gjColorFromF32(f, &c);
	} else {
		c.r = (col & 0xFF000000) >> 24;
		c.g = (col & 0xFF0000) >> 16;
		c.b = (col & 0xFF00) >> 8;
		c.a = col & 0xFF;
	}
	DRAW_IF_CHANCOLOR(4, c);
}

// 0x80212AD8
void gjParamStubA(u32 value)
{
}

// 0x80212ADC
void gjParamStubB(u32 value)
{
}

// 0x80212AE0
void gjParamBlend(u32 value)
{
	if ((u32)_nj_control_3d_flag_ & 0x8000) {
		value = gjBlendOverride;
	}
	DRAW_IF_BLEND((value & 0xC000) >> 14, (value & 0x3800) >> 11, (value & 0x700) >> 8, value & 0xFF);
}

// 0x80212B44
// With float vertices positions and normals are always floats
void gjParamVtxFmt(u32 value)
{
	s32 attr = gjVtxAttrTable[(value & 0xFF0000) >> 16];
	u32 cnt = gjTexCompCntTable[(value & 0xF000) >> 12];
	u32 type = gjTexCoordTypeTable[(value & 0xF00) >> 8];
	u8 frac = value & 0xFF;

	if (gjVertsAreFloat != 0 && (attr == 9 || attr == 10)) {
		type = 4;
	}
	GXSetVtxAttrFmt(0, attr, cnt, type, frac);
}

// 0x80212BD8
void gjParamTexture(u32 value)
{
	DRAW_IF_TEXTURE(value & 0x1FFF, (value & 0xC0000) >> 18, (value & 0x30000) >> 16, (value & 0x6000) >> 13);
}

// 0x80212C28
void gjParamTevOrder(u32 value)
{
	s32 stage = (value & 0xF000) >> 12;
	s32 coord = gjTexCoordTable[(value & 0xF00) >> 8];
	s32 map = gjTexMapTable[(value & 0xF0) >> 4];
	s32 color = value & 0xF;

	if ((u32)_nj_control_3d_flag_ & 0x80000) {
		stage += gjDrawState.preTevStages;
	}
	DRAW_IF_TEVORDER(stage, coord, map, color);
}

// 0x80212CB8
void gjParamTexGen(u32 value)
{
	s32 dst = gjTexCoordTable[(value & 0xF0000) >> 16];
	s32 func = (value & 0xF000) >> 12;
	s32 src = (value & 0xFF0) >> 4;
	s32 mtx = gjPosMtxTable[value & 0xF];

	if ((u32)_nj_control_3d_flag_ & 0x80000) {
		dst += gjDrawState.preTexGens;
	}
	DRAW_IF_TEXGEN(dst, func, src, mtx);
}

// 0x80212D48
// Colour and alpha channels are set separately
void gjSetChanCtrl(s32 chan, u8 enable, s32 amb, s32 mat, u32 lights)
{
	s32 color;
	s32 alpha;

	if (chan == 4) {
		color = 0;
		alpha = 2;
	} else if (chan == 5) {
		color = 1;
		alpha = 3;
	} else {
		color = 0;
		alpha = 2;
	}
	GXSetChanCtrl(color, enable, amb, mat, lights, 2, 2);
	GXSetChanCtrl(alpha, 0, amb, mat, lights, 2, 2);
}

// 0x80212DF8
void gjSetChanMatColor(s32 chan, GXColor c)
{
	GXSetChanMatColor(4, c);
}

// 0x80212E28
void gjSetTevStage(s32 stage, s32 coord, s32 map, s32 color)
{
	if (map == 0xFF) {
		GXSetTevOp(stage, 4);
	} else {
		GXSetTevOp(stage, 0);
	}
	GXSetTevOrder(stage, coord, map, color);
}

// 0x80212EA4
void gjSetAlphaRef(u8 ref)
{
	GXSetAlphaCompare(4, ref, 0, 7, 0);
}

// 0x80212ED8
// The environment matrix is normalized and goes through the post matrix
void gjSetTexGen(s32 dst, s32 func, s32 src, u32 mtx)
{
	if (mtx == 0x2A) {
		GXSetTexCoordGen2(dst, func, src, mtx, 1, 0x79);
	} else {
		GXSetTexCoordGen2(dst, func, src, mtx, 0, 0x7D);
	}
}

// 0x80212F18
void gjMotionActor(GjActor* actor, void* motion, f32 frame)
{
	njStartMotionObj(motion, frame);
	gjBeginCalcActor(actor);
	gjDrawMotionTree(actor->object);
	gjEndCalcActor(actor);
}

// 0x80212F60
void gjDrawMotionTree(NJS_OBJECT* obj)
{
	do {
		njPushMatrixEx();
		njMotionTransformEx(obj);
		if (gjObjectCallback != NULL) {
			gjObjectCallback(obj);
		}
		if (obj->model != NULL && (obj->evalflags & 8) == 0) {
			gjCalcActor(obj->model);
		}
		if (obj->child != NULL) {
			gjDrawMotionTree(obj->child);
		}
		njPopMatrixEx();
		obj = obj->sibling;
	} while (obj != NULL);
}

// 0x80212FEC
void gjSetMotionCallback(void (*f)(NJS_OBJECT*))
{
	gjObjectCallback = f;
}

// 0x80212FF4
void gjDrawMotion(NJS_OBJECT* obj, void* motion, f32 frame)
{
	njStartMotionObj(motion, frame);
	gjDrawCalls->drawModel = (void (*)(void*))_gjDrawModel;
	gjBeginDrawModel();
	gjDrawListBegin(_gjDrawModel);
	gjDrawMotionObj(obj);
	gjEndDrawModel();
	gjDrawListFlush();
}

// 0x80213054
void gjDrawMotionObj(NJS_OBJECT* obj)
{
	do {
		njPushMatrixEx();
		njMotionTransformEx(obj);
		if (gjObjectCallback != NULL) {
			gjObjectCallback(obj);
		}
		if (obj->model != NULL && (obj->evalflags & 8) == 0) {
			gjDrawListAdd(obj, _gj_matrix_);
		}
		if (obj->child != NULL) {
			gjDrawMotionObj(obj->child);
		}
		njPopMatrixEx();
		obj = obj->sibling;
	} while (obj != NULL);
}
