#include "io/ResBank.h"
#include "Ninja/ninja.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "engine/Color.h"
#include "core/MslMath.h"
#include "engine/Viewport.h"
#include "egg/EggCreature.h"
#include "object/ObjAction.h"
#include "ui/NowLoading.h"
#include "ui/AeArc.h"

#define GX_FIFO_F32 (*(volatile f32*)0xCC008000)
#define GX_FIFO_U32 (*(volatile u32*)0xCC008000)

struct NowLoadingPart {
	f32 mU0, mV0, mU1, mV1;
};

static const NowLoadingPart sParts[12] = {
	{ 0.0f, 0.0f, 0.15625f, 0.171875f },        // N
	{ 0.15625f, 0.0f, 0.328125f, 0.171875f },   // O
	{ 0.328125f, 0.0f, 0.5390625f, 0.171875f }, // W
	{ 0.0f, 0.1875f, 0.15625f, 0.359375f },     // L
	{ 0.15625f, 0.1875f, 0.328125f, 0.359375f },
	{ 0.328125f, 0.1875f, 0.4921875f, 0.359375f },
	{ 0.4921875f, 0.1875f, 0.578125f, 0.359375f },
	{ 0.578125f, 0.1875f, 0.7265625f, 0.359375f },
	{ 0.0f, 0.375f, 1.0f, 0.484375f },
	{ 0.0f, 0.484375f, 0.84375f, 0.59375f },
	{ 0.0f, 0.75f, 0.4140625f, 1.0f },
	{ 0.4296875f, 0.734375f, 0.84375f, 1.0f }, // the egg
};

static const int sTextParts[10]   = { 0, 1, 2, 3, 1, 4, 5, 6, 0, 7 };
static const f32 sTextAdvance[10] = { 0.0f, 0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
static const u8 sRed[4]           = { 0xFF, 0x00, 0x00, 0xFF };
static const u8 sGreen[4]         = { 0xFF, 0xFF, 0xC8, 0x00 };
static const u8 sBlue[4]          = { 0x00, 0x00, 0xFF, 0x00 };

static ResBank sNowLoadingBank;
static f32 sColorT;
static f32 sHopT;
static f32 sEggT;
static u32 sFlags;

// 0x801BD568
void nowLoadingInit()
{
	resBankLoad(&sNowLoadingBank, "ar_nowloading.arc");
	resBankAddTextures(&sNowLoadingBank, 0, "nowloading.gvm");
	nowLoadingReset();
}

// 0x801BD5B4
void nowLoadingReset()
{
	sFlags  = 0;
	sColorT = 0.0f;
	sHopT   = 0.0f;
	sEggT   = 0.0f;
}

// 0x801BD5D0
void nowLoadingDisable()
{
	sFlags |= 1;
}

// 0x801BD5E0
int nowLoadingIsDisabled()
{
	return sFlags & 1;
}

// 0x801BD5EC
void nowLoadingUpdate()
{
	sColorT += 0.01f;
	if (sColorT > 1.0f) {
		sColorT -= 1.0f;
	}
	sHopT += 0.01f;
	if (sHopT > 1.0f) {
		sHopT -= 1.0f;
	}
	sEggT += 0.02f;
	if (sEggT > 1.0f) {
		sEggT -= 1.0f;
	}
}

static inline u32 makeColor(int i)
{
	return (sRed[i] & 0xFF) << 24 | (sGreen[i] & 0xFF) << 16 | (sBlue[i] & 0xFF) << 8 | 0xFF;
}

// 0x801BD65C
u32 nowLoadingGetColor(f32 t)
{
	t = njFraction(t);
	if (t < 0.0f) {
		t += 1.0f;
	}
	t *= 4.0f;

	int i    = 0;
	f32 step = 1.0f;
	while (true) {
		if (t < step) {
			t -= step - 1.0f;
			break;
		}
		step += 1.0f;
		i++;
	}

	int b = i + 1;
	i %= 4;
	b %= 4;
	u32 from = makeColor(i);
	u32 to   = makeColor(b);
	u32 out;
	lerpColor(t, &out, &from, &to);
	return out;
}

// 0x801BD754
// How high a letter hops it sits still for most of the cycle
f32 nowLoadingGetHop(f32 t)
{
	t = njFraction(t);
	if (t < 0.0f) {
		t += 1.0f;
	}
	f32 hop = 0.0f;
	if (t >= 0.7f) {
		t -= 0.7f;
		t /= 0.3f;
		hop = 0.5f * (1.0f - njCos((s32)(65536.0f * t)));
		hop *= 20.0f;
	}
	return hop;
}

// 0x801BD7DC
// One quad top left at x y 128 units to the texture
f32 nowLoadingDrawPart(f32 x, f32 y, u32 color, int part)
{
	const NowLoadingPart* p = &sParts[part];
	f32 x1;
	f32 z  = 0.0f;
	f32 u0 = p->mU0;
	f32 u1 = p->mU1;
	f32 v0 = p->mV0;
	f32 v1 = p->mV1;
	x1     = 128.0f * (u1 - u0) + x;
	f32 y1 = y - 128.0f * (v1 - v0);

	GXPosition3f32(x, y, z);
	GXColor1u32(color);
	GXTexCoord2f32(u0, v0);
	GXPosition3f32(x1, y, z);
	GXColor1u32(color);
	GXTexCoord2f32(u1, v0);
	GXPosition3f32(x1, y1, z);
	GXColor1u32(color);
	GXTexCoord2f32(u1, v1);
	GXPosition3f32(x, y1, z);
	GXColor1u32(color);
	GXTexCoord2f32(u0, v1);
	return x1;
}

// 0x801BD880
// A black screen
void nowLoadingDrawBack()
{
	f32 left   = -320.0f;
	f32 top    = 240.0f;
	f32 zero   = 0.0f;
	f32 right  = 320.0f;
	f32 one    = 1.0f;
	f32 bottom = -240.0f;

	GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	GXBegin(GX_QUADS, GX_VTXFMT0, 4);
	GXPosition3f32(left, top, zero);
	GXColor1u32(0xFF);
	GXTexCoord2f32(zero, zero);
	GXPosition3f32(right, top, zero);
	GXColor1u32(0xFF);
	GXTexCoord2f32(one, zero);
	GXPosition3f32(right, bottom, zero);
	GXColor1u32(0xFF);
	GXTexCoord2f32(one, one);
	GXPosition3f32(left, bottom, zero);
	GXColor1u32(0xFF);
	GXTexCoord2f32(zero, one);
	GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
}

// 0x801BD944
// "NOW LOADING"
void nowLoadingDrawText(f32 colorT, f32 hopT)
{
	f32 x = 30.0f;
	f32 c = colorT;
	f32 h = hopT;
	GXBegin(GX_QUADS, GX_VTXFMT0, 40);
	u32 color;
	int i;
	for (i = 0; i < 10; i++) {
		color = nowLoadingGetColor(c);
		x     = nowLoadingDrawPart(x, -150.0f + nowLoadingGetHop(h), color, sTextParts[i]);
		x += sTextAdvance[i];
		c += 0.05f;
		h += 0.05f;
	}
}

// 0x801BDA3C
void nowLoadingDrawDots()
{
	GXBegin(GX_QUADS, GX_VTXFMT0, 8);
	f32 x = nowLoadingDrawPart(24.0f, -180.0f, 0xFFFFFFFF, 8);
	nowLoadingDrawPart(x, -180.0f, 0xFFFFFFFF, 9);
}

// 0x801BDA8C
// The egg rocks about its bottom right corner then its shadow
void nowLoadingDrawEgg(f32 t)
{
	GXBegin(GX_QUADS, GX_VTXFMT0, 8);
	f32 px, py;
	const NowLoadingPart* p = &sParts[11];
	f32 u0                  = p->mU0;
	f32 u1                  = p->mU1;
	f32 v0                  = p->mV0;
	f32 v1                  = p->mV1;
	f32 w                   = -(128.0f * (u1 - u0));
	f32 h                   = 128.0f * (v1 - v0);
	px                      = 240.0f - w;
	py                      = -138.0f - h;

	t = njFraction(t);
	if (t < 0.0f) {
		t += 1.0f;
	}
	if (t < 0.2f) {
		t /= 0.2f;
	} else if (t < 0.4f) {
		t = 1.0f;
	} else {
		t = 1.0f - t;
		t /= 0.6f;
	}

	f32 s, c;
	njSinCos(-((s32)(4551.0f * t) + 0xAAA), &s, &c);

	f32 wx, wy;
	f32 wc = w * c;
	f32 ws = w * s;
	f32 hx = h * -s;
	wx     = px + wc;
	f32 hy = h * c;
	wy     = py + ws;
	f32 z  = 0.0f;

	GX_FIFO_F32 = hx + wx;
	GX_FIFO_F32 = hy + wy;
	GX_FIFO_F32 = z;
	GX_FIFO_U32 = 0xFFFFFFFF;
	GX_FIFO_F32 = u0;
	GX_FIFO_F32 = v0;

	GX_FIFO_F32 = px + hx;
	GX_FIFO_F32 = py + hy;
	GX_FIFO_F32 = z;
	GX_FIFO_U32 = 0xFFFFFFFF;
	GX_FIFO_F32 = u1;
	GX_FIFO_F32 = v0;

	GX_FIFO_F32 = px;
	GX_FIFO_F32 = py;
	GX_FIFO_F32 = z;
	GX_FIFO_U32 = 0xFFFFFFFF;
	GX_FIFO_F32 = u1;
	GX_FIFO_F32 = v1;

	GX_FIFO_F32 = wx;
	GX_FIFO_F32 = wy;
	GX_FIFO_F32 = z;
	GX_FIFO_U32 = 0xFFFFFFFF;
	GX_FIFO_F32 = u0;
	GX_FIFO_F32 = v1;

	nowLoadingDrawPart(240.0f, -166.0f, 0xFFFFFFFF, 10);
}

// 0x801BDCC0
void nowLoadingDraw()
{
	gjSetPerspective(0x2AAA, 1.3333334f, 1.0f, 10000.0f);
	setViewport(0);
	njSetTexture(*resBankGetTextures(&sNowLoadingBank));
	gjLoadTextureNum(0, 0, 0, 0);
	loadScreenMatrix(50.0f, 0);

	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);  // GX_VA_POS GX_DIRECT
	GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT); // GX_VA_CLR0
	GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT); // GX_VA_TEX0
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
	GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
	GXSetNumTexGens(1);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2X4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
	GXSetNumTevStages(1);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GXSetTevDirect(GX_TEVSTAGE0);
	GXSetNumIndStages(0);
	GXSetAlphaCompare(GX_GREATER, 0x14, GX_AOP_AND, GX_LEQUAL, 0xFF);
	GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_COPY);
	GXSetZMode(GX_ENABLE, GX_ALWAYS, GX_DISABLE);
	GXSetZCompLoc(GX_FALSE);
	GXSetCullMode(GX_CULL_BACK);
	GXSetFog(GX_FOG_NONE, 0.0f, 10000.0f, 0.0f, 10000.0f, sNowLoadingFogColor);

	nowLoadingDrawBack();
	nowLoadingDrawText(sColorT, sHopT);
	nowLoadingDrawDots();
	nowLoadingDrawEgg(sEggT);
}
