#include "stage/Stage.h"
#include "engine/Fx.h"
#include "engine/FxSystem.h"
#include "engine/FxSetup.h"
#include "Ninja/ninja.h"
#include "engine/FxShape.h"

void fxDrawBegin(FxGroup* group)
{
	u8* state = gFxRenderState;
	GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_1);
	GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_1);
	GXLoadPosMtxImm((const float*)state, GX_PNMTX0);
	f32 m[3][4];
	PSMTXIdentity(m[0]);
	GXLoadPosMtxImm((const float*)m, GX_PNMTX1);
	gFxLastMatrixSlot = 3;
	fxSetCurrentMtx(0);
	gFxLastBlendMode = -1;
	gFxLastArray = 0;
	fxSetCoordArray(gFxCoordArray, 8);
	gFxLastMode = 0;
	fxSetCull(2);
	gFxLastZMode.mWord = 0;
	fxSetZMode(2, 0);
	GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	gFxVertexColor[0] = 0xFF;
	gFxVertexColor[1] = 0xFF;
	fxSetTevMode(0, 0);
	fxSetTevMode(1, 0);
	gFxLastColor[3] = 0xFF;
	gFxLastColor[2] = 0xFF;
	gFxLastColor[1] = 0xFF;
	gFxLastColor[0] = 0xFF;
	u32 color = *(u32*)gFxLastColor;
	GXSetTevColor(GX_TEVREG0, *(GXColor*)&color);
	gFxRenderReady = 0;
	int* in = (int*)(state + 0x260);
	GXSetTevColorIn(GX_TEVSTAGE0, in[0], in[1], in[2], in[3]);
	in = (int*)(state + 0x280);
	GXSetTevAlphaIn(GX_TEVSTAGE0, in[0], in[1], in[2], in[3]);
	f32 ind[2][3];
	ind[0][0] = 0.25f;
	ind[0][1] = 0.0f;
	ind[0][2] = 0.0f;
	ind[1][0] = 0.0f;
	ind[1][1] = 0.25f;
	ind[1][2] = 0.0f;
	GXSetIndTexMtx(GX_ITM_0, ind[0], -1);
	if (gFxFlag40Callback != NULL) {
		gFxLastFlag40 = 0x40;
		((void (*)(int))gFxFlag40Callback)(0);
	}
	fxLoadExtraTex();
}

// 0x800DB790
// Run after a group of effects draws
void fxDrawEnd()
{
	GXSetTevDirect(GX_TEVSTAGE0);
	GXSetTevDirect(GX_TEVSTAGE1);
	GXSetTevDirect(GX_TEVSTAGE2);
	GXSetNumIndStages(0);
	fxSetCurrentMtx(0);
}
