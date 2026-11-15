#include "stage/Stage.h"
#include "engine/DrawState.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "engine/LightSet.h"
#include "engine/GenTex.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "Dolphin/OS/OSCache.h"
#include "object/TObjLauncher.h"
#include "Ninja/ninja.h"
#include "engine/FrameTex.h"
#include "egg/EggCreature.h"
#include "camera/TCameraCtrl.h"
#include "core/TSceneManager.h"
struct GjActor;
struct GjModel;
struct NJS_OBJECT;
struct CamView;
struct SceneRecord;

struct DrawCamera {
	u8 _00[0x44];
	int mFov;    // _44
	u8 _48[4];
	f32 mAspect; // _4C
};

extern "C" {

static void drawChanCtrl(s32 chan, u8 enable, s32 amb, s32 mat, u32 lights);
static void drawBlend(s32 type, s32 src, s32 dst, s32 op);
static void noiseChanCtrl();
static void noiseTexGen(s32 dst);
static void noiseTevOrder(s32 stage);
static void noisePostTexGen(u8 dst);
static void glowPostTexGen(int first);
static void glowPostTev(u8 s);
static void silhouetteChanCtrl();
static void silhouetteTevOrder(s32 stage);
static void silhouetteBlend();
static void noTexture();
static void blendAlpha();
static void blendAdd();
static void drawPostTev(int stage);
}

u32 sDrawLights;           // the scene's diffuse lights and the extra ones
s32 sDrawSpecLights;       // the scene's alpha-specular lights
u8 sDrawTevFlags;          // 1 specular 2 TEV colour 1 4 TEV colour 2 8 extra stages
s32 sDrawLit;              // the model's lighting enable from drawChanCtrl
DrawStateExtra* sDrawExtra;
int sDrawAmbientOn;
GXColor sDrawAmbient;
GXColor sDrawGlowColor;    // only the alpha is set from the state's ambient
s32 sDrawSavedBlend;

static void (*sDrawBlendFuncs[])() = { blendAlpha, blendAdd };

static const GXColor sZeroColor  = { 0, 0, 0, 0 };
static const GXColor sWhiteColor = { 0xFF, 0xFF, 0xFF, 0xFF };

// 0x80043B40
void drawStateInit()
{
}

// 0x80043B44
void drawStateExit()
{
}

// 0x80043B48
DrawStateSlot::DrawStateSlot()
{
	NJS_TEXINFO info;
	mTexList.textures = &mTexName;
	mTexList.nbTexture    = 1;
	njSetTextureInfo(&info, NULL, 3, 0x20, 0x20);
	njSetTextureName(&mTexName, (void*)&info, (void*)1000, (u32)0x08800000);
	njLoadTexture(&mTexList);
	makeDrawNoise((u8*)((GjTexMem*)mTexList.textures->texaddr)->hdr->info.image);
}

// 0x80043BCC
DrawStateSlot::~DrawStateSlot()
{
	njReleaseTexture(&mTexList);
}

static inline void drawCameraMatrix(f32* m)
{
	DrawCamera* camera = (DrawCamera*)camGetDrawView();
	C_MTXLightPerspective(m, (f32)(0.75 * (0.0054931640625 * camera->mFov)), camera->mAspect, 0.5f, -0.5f, 0.5f, 0.5f);
	PSMTXConcat(m, _gj_matrix_, m);
}

// 0x80043C18
void beginDrawState(DrawStateSlot* slot, const DrawState* state)
{
	f32 shadow[3][4];
	f32 view[3][4];
	f32 glow[3][4];
	void* funcs[8];
	f32 ind[2][3];
	f32 color[4];
	GXColor tevColor;
	GXColor k;
	u32 ambient;
	u32 flags;
	LightGroup* lights;
	int tevStages;
	u8 texGens;
	u8 tev;

	lights = getSceneRecord()->mLights;
	saveDrawBlend();
	flags        = state->mFlags;
	tevStages    = 0;
	texGens      = 0;
	slot->mFlags = flags;
	if ((flags & 0x1C00) == 0) {
		tev = 0;
		if (flags & 0x3438) {
			_nj_control_3d_flag_ |= 0x80000;
		}
		if (flags & DRAWSTATE_MATERIAL) {
			u32 c = state->mMaterial;
			_nj_control_3d_flag_ |= 0x10;
			color[0] = (u8)c / 255.0f;
			color[1] = (c >> 24) / 255.0f;
			color[2] = (u8)(c >> 16) / 255.0f;
			color[3] = (u8)(c >> 8) / 255.0f;
			njSetConstantMaterial(color);
		}
		if (flags & DRAWSTATE_BLEND) {
			_nj_control_3d_flag_ |= 0x8000;
			sDrawBlendFuncs[state->mBlend]();
		}
		u32 diffuse     = lights->mDiffuse;
		sDrawLights     = diffuse | lightSetApply();
		sDrawSpecLights = lights->mAlphaSpec;
		if (flags & DRAWSTATE_AMBIENT) {
			ambient = state->mAmbient;
			beginDrawAmbient(slot, &ambient);
		} else {
			funcs[5] = (void*)drawChanCtrl;
			gjChangeDrawObjIF(funcs, 0x80);
			sDrawAmbientOn = 0;
		}
		if (lights->mAlphaSpec != 0) {
			_nj_control_3d_flag_ |= 0x80000;
			GXSetChanAmbColor(GX_COLOR1A1, sZeroColor);
			GXSetChanMatColor(GX_COLOR1A1, sWhiteColor);
			tev |= 1;
			tevStages = 1;
			gjSetNumPostChans(1);
		}
		if (flags & DRAWSTATE_EXTRA) {
			tev |= 8;
			sDrawExtra = (DrawStateExtra*)state->mExtra;
			texGens = texGens + sDrawExtra->mTexGens;
			tevStages += sDrawExtra->mTevStages;
			if (texGens != 0) {
				gjSetNumPostTexGens(texGens);
				gjSetPostTexGenCallback((void*)((DrawStateExtra*)state->mExtra)->mTexGen);
			}
		}
		if (flags & DRAWSTATE_TEV1) {
			u32 c = state->mTev1;
			tev |= 2;
			tevStages++;
			tevColor.r = c >> 24;
			tevColor.g = c >> 16;
			tevColor.b = c >> 8;
			tevColor.a = c;
			GXSetTevColor(GX_TEVREG0, tevColor);
		}
		if (flags & DRAWSTATE_TEV2) {
			u32 c = state->mTev2;
			tev |= 4;
			tevStages++;
			tevColor.r = c >> 24;
			tevColor.g = c >> 16;
			tevColor.b = c >> 8;
			tevColor.a = c;
			GXSetTevColor(GX_TEVREG1, tevColor);
		}
		if (tevStages != 0) {
			gjSetPostTevStageCallback((void*)drawPostTev);
			gjSetNumPostTevStages(tevStages);
		}
		sDrawTevFlags = tev;
	} else if (flags & 0x400) {
		drawCameraMatrix(shadow[0]);
		GXLoadTexMtxImm(shadow[0], GX_TEXMTX2, GX_MTX3x4);
		PSMTXCopy(_gj_matrix_, view[0]);
		view[0][3] = 0.0f;
		view[1][3] = 0.0f;
		view[2][3] = 0.0f;
		GXLoadTexMtxImm(view[0], GX_TEXMTX1, GX_MTX3x4);
		frameTexSetCur();
		gjLoadTextureNum(0, 0, 0, 4);
		genTexLoad(0, 5);
		funcs[5] = (void*)noiseChanCtrl;
		funcs[3] = (void*)noiseTevOrder;
		funcs[4] = (void*)noiseTexGen;
		funcs[7] = (void*)noTexture;
		gjChangeDrawObjIF(funcs, 0xF0);
		_nj_control_3d_flag_ |= 0x80000;
		gjSetNumPostTexGens(1);
		gjSetPostTexGenCallback((void*)noisePostTexGen);
		GXSetNumIndStages(1);
		GXSetIndTexOrder(GX_IND_TEX_STAGE_0, GX_TEXCOORD1, GX_TEXMAP5);
		GXSetIndTexCoordScale(GX_IND_TEX_STAGE_0, GX_ITS_1, GX_ITS_1);
		GXSetTevIndWarp(GX_TEVSTAGE0, GX_IND_TEX_STAGE_0, GX_TRUE, GX_FALSE, GX_ITM_0);
		ind[0][0] = 0.5f;
		ind[0][1] = 0.0f;
		ind[0][2] = 0.0f;
		ind[1][0] = 0.0f;
		ind[1][1] = 0.5f;
		ind[1][2] = 0.0f;
		GXSetIndTexMtx(GX_ITM_0, ind[0], -1);
	} else if (flags & 0x800) {
		GXSetNumIndStages(0);
		GXSetTevDirect(GX_TEVSTAGE0);
		GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
		GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
		GXSetBlendMode(GX_BM_NONE, GX_BL_ZERO, GX_BL_ZERO, GX_LO_CLEAR);
		k.a = 0;
		k.r = state->mMaterial >> 24;
		k.b = 0;
		k.g = 0;
		GXSetTevKColor(GX_KCOLOR3, k);
		funcs[5] = (void*)silhouetteChanCtrl;
		funcs[3] = (void*)silhouetteTevOrder;
		funcs[6] = (void*)silhouetteBlend;
		funcs[7] = (void*)noTexture;
		gjChangeDrawObjIF(funcs, 0xB8);
		_nj_control_3d_flag_ |= 0x80000;
	} else if (flags & 0x1000) {
		drawCameraMatrix(glow[0]);
		GXLoadTexMtxImm(glow[0], GX_TEXMTX1, GX_MTX3x4);
		frameTexSetCur();
		gjLoadTextureNum(0, 0, 0, 4);
		genTexLoad(2, 5);
		_nj_control_3d_flag_ |= 0x80000;
		sDrawGlowColor.a = state->mAmbient;
		gjSetNumPostTexGens(2);
		gjSetPostTexGenCallback((void*)glowPostTexGen);
		gjSetNumPostTevStages(1);
		gjSetPostTevStageCallback((void*)glowPostTev);
		GXSetNumIndStages(1);
		GXSetIndTexOrder(GX_IND_TEX_STAGE_0, GX_TEXCOORD2, GX_TEXMAP5);
		GXSetIndTexCoordScale(GX_IND_TEX_STAGE_0, GX_ITS_1, GX_ITS_1);
		GXSetTevIndWarp(GX_TEVSTAGE1, GX_IND_TEX_STAGE_0, GX_TRUE, GX_FALSE, GX_ITM_0);
		if (state->_20 != NULL) {
			GXSetIndTexMtx(GX_ITM_0, (const f32*)state->_20, -2);
		}
		GXSetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_COPY);
		GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0xFF);
	}
}

// 0x800442D8
void endDrawState(DrawStateSlot* slot)
{
	u32 flags = slot->mFlags;
	void* funcs[8];
	if ((flags & 0x1C00) == 0) {
		funcs[5] = NULL;
		funcs[6] = NULL;
		gjChangeDrawObjIF(funcs, 0x88);
		gjSetNumPostChans(0);
		gjSetNumPostTevStages(0);
		gjSetNumPostTexGens(0);
	} else if (flags & 0x400) {
		GXSetNumIndStages(0);
		GXSetTevDirect(GX_TEVSTAGE0);
		funcs[5] = NULL;
		funcs[3] = NULL;
		funcs[4] = NULL;
		funcs[7] = NULL;
		gjChangeDrawObjIF(funcs, 0xF0);
		gjSetNumPostTexGens(0);
	} else if (flags & 0x800) {
		funcs[5] = NULL;
		funcs[3] = NULL;
		funcs[7] = NULL;
		funcs[6] = NULL;
		gjChangeDrawObjIF(funcs, 0xB8);
	} else if (flags & 0x1000) {
		GXSetNumIndStages(0);
		gjSetNumPostTevStages(0);
		GXSetTevDirect(GX_TEVSTAGE1);
		gjSetNumPostTexGens(0);
	}
	_nj_control_3d_flag_ &= ~0x88010;
	restoreDrawBlend();
}

// 0x800443E4
void beginDrawAmbient(DrawStateSlot* slot, const u32* color)
{
	void* funcs[8];
	funcs[5] = (void*)drawChanCtrl;
	funcs[6] = (void*)drawBlend;
	gjChangeDrawObjIF(funcs, 0x88);
	u32 c          = *color;
	sDrawAmbientOn = 1;
	sDrawAmbient.r = c >> 24;
	sDrawAmbient.g = c >> 16;
	sDrawAmbient.b = c >> 8;
	sDrawAmbient.a = c;
	setDrawBlendAmbient();
}

// 0x80044460
void endDrawAmbient(DrawStateSlot* slot)
{
	void* funcs[8];
	funcs[5] = NULL;
	funcs[6] = NULL;
	gjChangeDrawObjIF(funcs, 0x88);
	sDrawAmbientOn = 0;
}

// 0x8004449C
// Stands in for the model's GXSetChanCtrl
static void drawChanCtrl(s32 chan, u8 enable, s32 amb, s32 mat, u32 lights)
{
	GXSetChanCtrl(GX_COLOR0, enable, amb, mat, sDrawLights, GX_DF_CLAMP, GX_AF_SPOT);
	if (sDrawAmbientOn != 0) {
		GXSetChanAmbColor(GX_ALPHA0, sDrawAmbient);
		GXSetChanCtrl(GX_ALPHA0, GX_ENABLE, GX_SRC_REG, mat, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	} else {
		GXSetChanCtrl(GX_ALPHA0, GX_DISABLE, amb, mat, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	}
	sDrawLit = enable;
	if (sDrawSpecLights == 0) {
		GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	} else {
		GXSetChanCtrl(GX_COLOR1, enable, GX_SRC_REG, GX_SRC_REG, sDrawSpecLights, GX_DF_CLAMP, GX_AF_SPOT);
		GXSetChanCtrl(GX_ALPHA1, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	}
}

// 0x800445B8
// Type 0 is the model's own alpha blend with alpha testing on
static void drawBlend(s32 type, s32 src, s32 dst, s32 op)
{
	if (type == 0) {
		GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
		GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0);
	} else {
		GXSetBlendMode(type, src, dst, op);
	}
}

// 0x80044610
static void noiseChanCtrl()
{
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
}

// 0x8004466C
static void noiseTexGen(s32 dst)
{
	GXSetTexCoordGen2(dst, GX_TG_MTX3X4, GX_TG_POS, GX_TEXMTX2, GX_FALSE, GX_PTIDENTITY);
}

// 0x800446A0
static void noiseTevOrder(s32 stage)
{
	GXSetTevOrder(stage, GX_TEXCOORD0, GX_TEXMAP4, GX_COLOR0A0);
	GXSetTevColorIn(stage, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
	GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
	GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}

// 0x80044740
static void noisePostTexGen(u8 dst)
{
	GXSetTexCoordGen2(dst, GX_TG_MTX3X4, GX_TG_NRM, GX_TEXMTX1, GX_TRUE, GX_PTTEXMTX19);
}

// 0x80044778
static void glowPostTexGen(int first)
{
	u8 dst = first;
	GXSetTexCoordGen2(dst, GX_TG_MTX3X4, GX_TG_POS, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
	GXSetTexCoordGen2(dst + 1, GX_TG_MTX3X4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
}

// 0x800447D8
static void glowPostTev(u8 s)
{
	GXSetTevColor(GX_TEVREG0, sDrawGlowColor);
	u8 coord = gjGetPostTexGenStartID();
	GXSetTevColorIn(s, GX_CC_CPREV, GX_CC_TEXC, GX_CC_A0, GX_CC_ZERO);
	GXSetTevColorOp(s, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(s, GX_CA_APREV, GX_CA_TEXA, GX_CA_A0, GX_CA_ZERO);
	GXSetTevAlphaOp(s, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevOrder(s, coord, GX_TEXMAP4, GX_COLOR_NULL);
}

// 0x8004489C
static void silhouetteChanCtrl()
{
}

// 0x800448A0
static void silhouetteTevOrder(s32 stage)
{
	GXSetTevOrder(stage, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GXSetTevKColorSel(stage, GX_TEV_KCSEL_K3_R);
	GXSetTevColorIn(stage, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
	GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}

// 0x8004494C
static void silhouetteBlend()
{
}

// 0x80044950
static void noTexture()
{
}

// 0x80044954
// 32x32 IA8 in GX's 4x4 tiles intensity is a little noise around grey alpha a sine across x
void makeDrawNoise(u8* image)
{
	int x;
	int y;
	for (y = 0; y < 32; y++) {
		for (x = 0; x < 32; x++) {
			u8 i     = (int)(4.0f * ((1.0f / 32768.0f) * rand() - 0.5f)) + 0x80;
			f32 s    = njSin((x << 16) / 32);
			int texel = ((y >> 2) << 7) | (((y & 3) << 2) | ((x & 3) | ((x >> 2) << 4)));
			image[texel * 2]     = i;
			image[texel * 2 + 1] = (int)(3.0f * s) + 0x80;
		}
	}
	DCFlushRange(image, 0x800);
}

// 0x80044A98
void drawObjectDefault(void* object)
{
	DrawStateSlot* slot = getDrawStateSlot();
	beginDrawState(slot, getDefaultDrawState());
	gjDrawObject((NJS_OBJECT*)object);
	endDrawState(slot);
}

// 0x80044AE8
void drawModelDefault(void* model)
{
	DrawStateSlot* slot = getDrawStateSlot();
	beginDrawState(slot, getDefaultDrawState());
	gjDrawModel((GjModel*)model);
	endDrawState(slot);
}

// 0x80044B38
void drawActorDefault(void* actor)
{
	DrawStateSlot* slot = getDrawStateSlot();
	beginDrawState(slot, getDefaultDrawState());
	gjDrawActor((GjActor*)actor);
	endDrawState(slot);
}

// 0x80044B88
void drawMotionDefault(void* object, void* motion, f32 frame)
{
	DrawStateSlot* slot = getDrawStateSlot();
	beginDrawState(slot, getDefaultDrawState());
	gjDrawMotion((NJS_OBJECT*)object, motion, frame);
	endDrawState(slot);
}

// 0x80044BF0
static void blendAlpha()
{
	gjSetBlendOverride(1, 4, 5, 0xF);
}

// 0x80044C20
static void blendAdd()
{
	gjSetBlendOverride(1, 4, 1, 0xF);
}

// 0x80044C50
void saveDrawBlend()
{
	sDrawSavedBlend = gDrawAlphaRef;
}

// 0x80044C5C
void setDrawBlendAmbient()
{
	gDrawAlphaRef = 0x10;
}

// 0x80044C68
void restoreDrawBlend()
{
	gDrawAlphaRef = sDrawSavedBlend;
}

// The specular stage adds the second colour channel
static inline void specularTevStage(u8 s)
{
	GXSetTevOrder(s, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR1A1);
	if (sDrawLit != 0) {
		GXSetTevColorIn(s, GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_CPREV);
	} else {
		GXSetTevColorIn(s, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
	}
	GXSetTevAlphaIn(s, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevColorOp(s, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaOp(s, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}

// Multiplies by TEV colour 0
static inline void colour0TevStage(u8 s)
{
	GXSetTevOrder(s, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
	GXSetTevColorIn(s, GX_CC_ZERO, GX_CC_C0, GX_CC_CPREV, GX_CC_ZERO);
	GXSetTevAlphaIn(s, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevColorOp(s, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaOp(s, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}

// Adds TEV colour 1
static inline void colour1TevStage(u8 s)
{
	GXSetTevOrder(s, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
	GXSetTevColorIn(s, GX_CC_ZERO, GX_CC_C1, GX_CC_ONE, GX_CC_CPREV);
	GXSetTevAlphaIn(s, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevColorOp(s, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaOp(s, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}

// 0x80044C74
// The stages added after the model's specular the extra ones then the two TEV colours
static void drawPostTev(int stage)
{
	u8 tev = sDrawTevFlags;
	if (tev & 1) {
		specularTevStage(stage++);
	}
	if (tev & 8) {
		sDrawExtra->mTev(stage);
		stage += sDrawExtra->mTevStages;
	}
	if (tev & 2) {
		colour0TevStage(stage++);
	}
	if (tev & 4) {
		colour1TevStage(stage);
	}
}
