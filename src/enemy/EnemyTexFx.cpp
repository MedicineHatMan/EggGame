#include "object/ObjAction.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "camera/TCameraCtrl.h"
#include "engine/GenTex.h"
#include "enemy/EnemyFx.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "engine/FrameTex.h"
#include "core/MathUtil.h"
#include "enemy/EnemyTexFx.h"

extern "C" {
const int texFxTexMaps[8]      = { 0, 1, 2, 3, 4, 5, 6, 7 };
const int texFxTexMtxs[10]     = { 30, 33, 36, 39, 42, 45, 48, 51, 54, 57 };
const int texFxPostTexMtxs[20] = { 64, 67, 70, 73, 76, 79, 82, 85, 88, 91, 94, 97, 100, 103, 106, 109, 112, 115, 118, 121 };
const int texFxTexCoords[8]    = { 0, 1, 2, 3, 4, 5, 6, 7 };
const int texFxTevStages[16]   = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
const int texFxKColors[4]      = { 0, 1, 2, 3 };
const int texFxKColorSels[4]   = { 12, 13, 14, 15 };
const int texFxTevRegs[4]      = { 1, 2, 3, 0 };
const int texFxTevRegInputs[4] = { 2, 4, 6, 0 };
const int texFxIndStages[4]    = { 0, 1, 2, 3 };
const int texFxIndMtxs[4]      = { 1, 2, 3, 0 };
}

// The warp's choices
static int sWarpTexMap;    // 0x802C5168 the screen
static int sWarpIndMap;    // 0x802C516C the offsets
static int sWarpTexMtx;    // 0x802C5170
static int sWarpIndTexMtx; // 0x802C5174
static int sWarpPostMtx;   // 0x802C5178
static int sWarpStage;     // 0x802C517C
static int sWarpKSel;      // 0x802C5180
static int sWarpRegIn;     // 0x802C5184
static int sWarpIndStage;  // 0x802C5188
static int sWarpIndMtx;    // 0x802C518C
static int sWarpCoord;     // 0x802C5190
static int sWarpIndCoord;  // 0x802C5194

// The suit's
static int sSuitTexMap; // 0x802C5198
static int sSuitCoord;  // 0x802C519C
static int sSuitTexMtx; // 0x802C51A0
static int sSuitStage;  // 0x802C51A4
static int sSuitKSel;   // 0x802C51A8

// The tint's
static int sTintTexMap; // 0x802C51AC
static int sTintCoord;  // 0x802C51B0
static int sTintTexMtx; // 0x802C51B4
static int sTintStage;  // 0x802C51B8
static int sTintKSel;   // 0x802C51BC

// 0x800ACDC4
// Maps view space onto the screen texture x and y from -1..1 to 0..1 flipped in y
extern "C" void texFxScreenMtx(f32* out)
{
	f32 m[12];
	PSMTXIdentity(m);
	f32 half = 0.5f;
	f32 down = -0.5f;
	m[0]     = ((f32*)&gProjection)[0] * half;
	m[2]     = -half;
	m[5]     = ((f32*)&gProjection)[5] * down;
	m[6]     = -half;
	m[10]    = -1.0f;
	PSMTXConcat(m, _gj_matrix_, out);
}

// 0x800ACE3C
// Two coordinates the screen from the position the offsets from the normal
static void warpTexGen(u8 base)
{
	sWarpCoord    = texFxTexCoords[base];
	sWarpIndCoord = texFxTexCoords[base + 1];
	GXSetTexCoordGen2(sWarpCoord, GX_TG_MTX3X4, GX_TG_POS, sWarpTexMtx, GX_FALSE, GX_PTIDENTITY);
	GXSetTexCoordGen2(sWarpIndCoord, GX_TG_MTX3X4, GX_TG_NRM, sWarpIndTexMtx, GX_TRUE, sWarpPostMtx);
}

// 0x800ACEAC
// The screen offset by the indirect texture plus the glow colour
static void warpTev(u8 base)
{
	sWarpStage = texFxTevStages[base];
	GXSetNumIndStages(1);
	GXSetIndTexOrder(sWarpIndStage, sWarpIndCoord, sWarpIndMap);
	GXSetIndTexCoordScale(sWarpIndStage, GX_ITS_1, GX_ITS_1);
	GXSetTevIndWarp(sWarpStage, sWarpIndStage, GX_TRUE, GX_FALSE, sWarpIndMtx);
	GXSetTevOrder(sWarpStage, sWarpCoord, sWarpTexMap, GX_COLOR_NULL);
	GXSetTevKColorSel(sWarpStage, sWarpKSel);
	GXSetTevColorIn(sWarpStage, GX_CC_CPREV, GX_CC_TEXC, GX_CC_KONST, sWarpRegIn);
	GXSetTevColorOp(sWarpStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(sWarpStage, GX_CA_APREV, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
	GXSetTevAlphaOp(sWarpStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}

// 0x800ACFA4
// Loads the warp's three matrices the screen the normals in view space and the offsets' scale
extern "C" void texFxWarpLoadMtx(int mtx, int post)
{
	f32 m[12];
	f32 screen[12];
	sWarpTexMtx    = texFxTexMtxs[mtx];
	sWarpIndTexMtx = texFxTexMtxs[mtx + 1];
	sWarpPostMtx   = texFxPostTexMtxs[post];
	texFxScreenMtx(screen);
	GXLoadTexMtxImm(screen, sWarpTexMtx, GX_MTX3x4);
	PSMTXIdentity(m);
	PSMTXCopy(_gj_matrix_, m);
	m[3]  = 0.0f;
	m[7]  = 0.0f;
	m[11] = 0.0f;
	GXLoadTexMtxImm(m, sWarpIndTexMtx, GX_MTX3x4);
	PSMTXIdentity(m);
	m[0]  = 0.5f;
	m[1]  = 0.0f;
	m[2]  = 0.0f;
	m[3]  = 0.5f;
	m[4]  = 0.0f;
	m[5]  = -0.5f;
	m[6]  = 0.0f;
	m[7]  = -0.5f;
	m[8]  = 0.0f;
	m[9]  = 0.0f;
	m[10] = 0.0f;
	m[11] = 1.0f;
	GXLoadTexMtxImm(m, sWarpPostMtx, GX_MTX3x4);
}

// 0x800AD098
// Grabs the screen into a texture and loads it with the offsets
extern "C" void texFxWarpLoadTex(int map)
{
	sWarpTexMap = texFxTexMaps[map];
	sWarpIndMap = texFxTexMaps[map + 1];
	frameTexSetCur();
	gjLoadTextureNum(0, 0, 0, sWarpTexMap);
	genTexLoad(0, sWarpIndMap);
}

// 0x800AD0F8
// level is the glow from 0 to 1 wave how far the offsets push red the tint's red
extern "C" void texFxWarpBegin(f32 level, int kcolor, int reg, int indStage, int indMtx, int red, f32 wave)
{
	_nj_control_3d_flag_ |= 0x180000;
	gjSetNumPostChans(0);
	gjSetNumPostTexGens(2);
	gjSetNumPostTevStages(1);
	gjSetPostTexGenCallback(warpTexGen);
	gjSetPostTevStageCallback(warpTev);

	f32 grey = (u8)(255.0f * level);
	u8 color[4];
	color[0] = grey;
	color[1] = grey;
	color[2] = grey;
	color[3] = grey;
	GXSetTevKColor(texFxKColors[kcolor], *(GXColor*)color);
	sWarpKSel = texFxKColorSels[kcolor];

	color[3] = 0xFF;
	color[0] = red;
	color[1] = 0;
	f32 blue = (u8)(80.0f * level);
	color[2] = blue;
	GXSetTevColor(texFxTevRegs[reg], *(GXColor*)color);
	sWarpRegIn    = texFxTevRegInputs[reg];
	sWarpIndStage = texFxIndStages[indStage];
	sWarpIndMtx   = texFxIndMtxs[indMtx];

	f32 m[6];
	m[0] = 0.5f;
	m[1] = 0.0f;
	m[2] = 0.5f * wave;
	m[3] = 0.0f;
	m[4] = 0.5f;
	m[5] = 0.0f;
	GXSetIndTexMtx(sWarpIndMtx, m, -1);
}

// 0x800AD2F4
extern "C" void texFxWarpEnd()
{
	_nj_control_3d_flag_ &= ~0x180000;
	GXSetTevDirect(sWarpStage);
	GXSetNumIndStages(0);
	gjSetNumPostChans(0);
	gjSetNumPostTexGens(0);
	gjSetNumPostTevStages(0);
}

// 0x800AD344
static void suitTexGen(u8 base)
{
	sSuitCoord = texFxTexCoords[base];
	GXSetTexCoordGen2(sSuitCoord, GX_TG_MTX3X4, GX_TG_POS, sSuitTexMtx, GX_FALSE, GX_PTIDENTITY);
}

// 0x800AD38C
// The previous colour times the suit's colour faded by the texture's alpha
static void suitTev(u8 base)
{
	sSuitStage = texFxTevStages[base];
	GXSetTevOrder(sSuitStage, sSuitCoord, sSuitTexMap, GX_COLOR_NULL);
	GXSetTevKColorSel(sSuitStage, sSuitKSel);
	GXSetTevColorIn(sSuitStage, GX_CC_ZERO, GX_CC_CPREV, GX_CC_KONST, GX_CC_ZERO);
	GXSetTevColorOp(sSuitStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(sSuitStage, GX_CA_ZERO, GX_CA_APREV, GX_CA_TEXA, GX_CA_ZERO);
	GXSetTevAlphaOp(sSuitStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	alphaTestBegin();
}

// 0x800AD444
extern "C" void texFxSuitLoadTex(int map)
{
	sSuitTexMap = texFxTexMaps[map];
	genTexLoadWrap(5, sSuitTexMap, 0);
}

// 0x800AD480
// The texture runs up the body from bottom to top turning with the clock
extern "C" void texFxSuitScroll(f32 top, f32 bottom, int mtx)
{
	f32 m[12];
	sSuitTexMtx = texFxTexMtxs[mtx];
	PSMTXIdentity(m);
	m[0]      = 0.0f;
	m[1]      = -1.0f;
	f32 scale = 1.0f / (top - bottom);
	m[2]      = 0.0f;
	m[3]      = 1.0f;
	m[4]      = 1.0f;
	m[5]      = 0.0f;
	m[6]      = 0.0f;
	m[7]      = 0.0f;
	m[8]      = 0.0f;
	m[9]      = 0.0f;
	m[10]     = 0.0f;
	m[11]     = 1.0f;
	mtxScale(scale, scale, 1.0f, m, m);
	mtxRotateY(m, m, -camGetDrawYaw());
	mtxTranslate(m, m, 0.0f, -bottom, 0.0f);
	GXLoadTexMtxImm(m, sSuitTexMtx, GX_MTX3x4);
}

// 0x800AD570
extern "C" void texFxSuitBeginColor(int kcolor, const u32* color)
{
	_nj_control_3d_flag_ |= 0x180000;
	gjSetNumPostChans(0);
	gjSetNumPostTexGens(1);
	gjSetNumPostTevStages(1);
	gjSetPostTexGenCallback(suitTexGen);
	gjSetPostTevStageCallback(suitTev);
	u32 c = *color;
	u8 parts[4];
	parts[0] = c >> 24;
	parts[1] = c >> 16;
	parts[2] = c >> 8;
	parts[3] = c;
	GXSetTevKColor(texFxKColors[kcolor], *(GXColor*)parts);
	sSuitKSel = texFxKColorSels[kcolor];
}

// 0x800AD628
// The suit in grey the same level in all four channels
extern "C" void texFxSuitBegin(int kcolor, u32 level)
{
	u32 c = (u8)level << 24 | (u8)level << 16 | (u8)level << 8 | (u8)level;
	texFxSuitBeginColor(kcolor, &c);
}

// 0x800AD660
extern "C" void texFxSuitEnd()
{
	_nj_control_3d_flag_ &= ~0x180000;
	GXSetTevDirect(sSuitStage);
	GXSetNumIndStages(0);
	gjSetNumPostChans(0);
	gjSetNumPostTexGens(0);
	gjSetNumPostTevStages(0);
}

// 0x800AD6B0
static void tintTexGen(u8 base)
{
	sTintCoord = texFxTexCoords[base];
	GXSetTexCoordGen2(sTintCoord, GX_TG_MTX3X4, GX_TG_POS, sTintTexMtx, GX_FALSE, GX_PTIDENTITY);
}

// 0x800AD6F8
// The previous colour plus the tint where the texture lets it through
static void tintTev(u8 base)
{
	sTintStage = texFxTevStages[base];
	GXSetTevOrder(sTintStage, sTintCoord, sTintTexMap, GX_COLOR_NULL);
	GXSetTevKColorSel(sTintStage, sTintKSel);
	GXSetTevColorIn(sTintStage, GX_CC_ZERO, GX_CC_CPREV, GX_CC_ONE, GX_CC_KONST);
	GXSetTevColorOp(sTintStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(sTintStage, GX_CA_ZERO, GX_CA_APREV, GX_CA_TEXA, GX_CA_ZERO);
	GXSetTevAlphaOp(sTintStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	alphaTestBegin();
}

// 0x800AD7B0
extern "C" void texFxTintLoadTex(int map)
{
	sTintTexMap = texFxTexMaps[map];
	genTexLoadWrap(5, sTintTexMap, 0);
}

// 0x800AD7EC
// Projects the texture flat from pos turned by angle
extern "C" void texFxTintSetMtx(int mtx, const Vec3f* pos, int angle)
{
	f32 m[12];
	sTintTexMtx = texFxTexMtxs[mtx];
	PSMTXIdentity(m);
	m[0]  = 0.0f;
	m[1]  = 0.0f;
	m[2]  = 1.0f;
	m[3]  = 0.0f;
	m[4]  = 1.0f;
	m[5]  = 0.0f;
	m[6]  = 0.0f;
	m[7]  = 0.0f;
	m[8]  = 0.0f;
	m[9]  = 0.0f;
	m[10] = 0.0f;
	m[11] = 1.0f;
	mtxRotateY(m, m, -angle);
	mtxTranslate(m, m, -pos->x, -pos->y, -pos->z);
	PSMTXConcat(m, _gj_matrix_, m);
	GXLoadTexMtxImm(m, sTintTexMtx, GX_MTX3x4);
}

// 0x800AD8C0
extern "C" void texFxTintBegin(int kcolor, const u32* color)
{
	_nj_control_3d_flag_ |= 0x80000;
	gjSetNumPostChans(0);
	gjSetNumPostTexGens(1);
	gjSetNumPostTevStages(1);
	gjSetPostTexGenCallback(tintTexGen);
	gjSetPostTevStageCallback(tintTev);
	u32 c = *color;
	u8 parts[4];
	parts[0] = c >> 24;
	parts[1] = c >> 16;
	parts[2] = c >> 8;
	parts[3] = c;
	GXSetTevKColor(texFxKColors[kcolor], *(GXColor*)parts);
	sTintKSel = texFxKColorSels[kcolor];
}

// 0x800AD978
extern "C" void texFxTintEnd()
{
	_nj_control_3d_flag_ &= ~0x80000;
	GXSetTevDirect(sTintStage);
	GXSetNumIndStages(0);
	gjSetNumPostChans(0);
	gjSetNumPostTexGens(0);
	gjSetNumPostTevStages(0);
	gjSetPostTexGenCallback(0);
	gjSetPostTevStageCallback(0);
}
