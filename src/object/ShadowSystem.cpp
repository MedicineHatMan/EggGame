#include "stage/Stage.h"
#include "enemy/EnemyCommon.h"
#include "object/ObjCommon.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "Ninja/ninja.h"
#include "engine/FrameTex.h"
#include "object/ObjShadow.h"
#include "enemy/EnemyFx.h"
#include "stl/math.h"
#include "object/ObjAction.h"
#include "core/SdkCalls.h"
struct NJS_OBJECT;

// 0x800F189C
// Destination alpha is on for the volumes
void shadowSystemInit()
{
	GXSetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
	GXSetDstAlpha(GX_ENABLE, 0x10);
	sShadowVolume = sShadowVolumeModel;
	sShadowFlags = 0;
	shadowListInit();
}

// 0x800F18E8
void shadowSystemExit()
{
	shadowListStub();
}

// 0x800F1908
void* shadowGetModel()
{
	return sShadowVolume;
}

// 0x800F1910
void shadowRestoreAlpha()
{
	if (sShadowFlags & 1) {
		GXSetAlphaUpdate(GX_DISABLE);
	} else {
		GXSetAlphaUpdate(GX_ENABLE);
	}
}

// 0x800F194C
void shadowAlphaOff()
{
	sShadowFlags |= 1;
	GXSetAlphaUpdate(GX_DISABLE);
}

// 0x800F197C
void shadowAlphaOn()
{
	sShadowFlags &= ~1;
	GXSetAlphaUpdate(GX_ENABLE);
}

// 0x800F19AC
void shadowAlphaBegin()
{
	GXSetAlphaUpdate(GX_ENABLE);
	GXSetDstAlpha(GX_ENABLE, 0);
}

// 0x800F19DC
void shadowAlphaEnd()
{
	shadowRestoreAlpha();
	GXSetDstAlpha(GX_ENABLE, 0x10);
}

// 0x800F1A08
void shadowDraw()
{
	shadowDrawAll();
}

// 0x800F1A28
// Darkens the screen where the volumes left destination alpha
void shadowDarkenScreen()
{
	NJS_QUAD_TEXTURE quad;
	quad.x1 = 0.0f;
	quad.y1 = 0.0f;
	quad.x2 = 640.0f;
	quad.y2 = 480.0f;
	quad.u1 = quad.x1 / quad.x2;
	quad.v1 = quad.y1 / quad.y2;
	quad.u2 = quad.x2 / quad.x2;
	quad.v2 = quad.y2 / quad.y2;
	frameTexSetCur();
	njQuadTextureStart(1);
	njSetQuadTexture(0, -1);
	GXColor konst = { 0, 0, 0, 0x20 };
	GXSetTevKColor(GX_KCOLOR0, konst);
	GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
	GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
	GXColor shade = { 0, 0, 0, 0x80 };
	GXSetTevColor(GX_TEVREG0, shade);
	GXSetNumTevStages(1);
	GXSetTevDirect(GX_TEVSTAGE0);
	GXSetNumIndStages(0);
	GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
	GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_TEXA, GX_CA_KONST, GX_CA_A0, GX_CA_ZERO);
	GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_COMP_A8_GT, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_COPY);
	GXSetZCompLoc(GX_FALSE);
	GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
	njDrawQuadTexture(&quad, 0.1f);
	njQuadTextureEnd();
	GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
}

// 0x800F1BA4
void shadowTevStub()
{
}

// 0x800F1BA8
void shadowSetTevPass()
{
	GXSetNumTevStages(1);
	GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
}

// 0x800F1BEC
void shadowSetNoTexGen()
{
	GXSetNumTexGens(0);
}

// 0x800F1C10
void shadowSetChans()
{
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
	GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
}

// 0x800F1C74
void shadowBlendAdd()
{
	GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_COPY);
}

// 0x800F1CA4
void shadowBlendSub()
{
	GXSetBlendMode(GX_BM_SUBTRACT, GX_BL_ONE, GX_BL_ONE, GX_LO_COPY);
}

// 0x800F1CD4
// The volumes write only destination alpha and leave the depth alone
void shadowVolumeBegin()
{
	GXSetColorUpdate(GX_FALSE);
	GXSetAlphaUpdate(GX_TRUE);
	GXSetDstAlpha(GX_FALSE, 0x10);
	GXColor color;
	color.r = 0;
	color.g = 0;
	color.b = 0;
	color.a = 0x10;
	GXSetChanMatColor(GX_COLOR0A0, color);
	GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0xFF);
	njZWriteMode(0);
	GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
	getCullMode((int*)&sShadowCull);
}

// 0x800F1D70
// The back faces adding to destination alpha
void shadowVolumeAdd()
{
	void* funcs[8];
	setCullMode(0x10000000);
	GXSetCullMode(GX_CULL_BACK);
	funcs[1] = (void*)shadowTevStub;
	funcs[3] = (void*)shadowSetTevPass;
	funcs[4] = (void*)shadowSetNoTexGen;
	funcs[5] = (void*)shadowSetChans;
	funcs[6] = (void*)shadowBlendAdd;
	gjChangeDrawObjIF(funcs, 0xEA);
}

// 0x800F1DE4
// The front faces taking it away again
void shadowVolumeSubtract()
{
	void* funcs[8];
	setCullMode(0x18000000);
	GXSetCullMode(GX_CULL_FRONT);
	funcs[1] = (void*)shadowTevStub;
	funcs[3] = (void*)shadowSetTevPass;
	funcs[4] = (void*)shadowSetNoTexGen;
	funcs[5] = (void*)shadowSetChans;
	funcs[6] = (void*)shadowBlendSub;
	gjChangeDrawObjIF(funcs, 0xEA);
}

// 0x800F1E58
// Puts the defaults back
void shadowVolumeEnd()
{
	void* funcs[8];
	setCullMode(sShadowCull);
	GXSetCullMode(GX_CULL_BACK);
	funcs[1] = 0;
	funcs[3] = 0;
	funcs[4] = 0;
	funcs[5] = 0;
	funcs[6] = 0;
	gjChangeDrawObjIF(funcs, 0xEA);
	njZWriteMode(1);
	GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	GXSetColorUpdate(GX_TRUE);
	shadowRestoreAlpha();
	GXSetDstAlpha(GX_TRUE, 0x10);
}

// 0x800F1ED8
void shadowListInit()
{
	sShadowList = NULL;
	sShadowVolumesOn = 1;
}

// 0x800F1EEC
void shadowListStub()
{
}

// 0x800F1EF0
void shadowVolumesDisable()
{
	sShadowVolumesOn = 0;
}
