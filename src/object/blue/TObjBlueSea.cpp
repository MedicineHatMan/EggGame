#include "object/blue/TObjBlueSea.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "io/ResBank.h"
#include "engine/GenTex.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "stl/math.h"
#include "ui/AeArc.h"
#include "engine/Color.h"
#include "Ninja/ninja.h"
#include "engine/FrameTex.h"
#include "enemy/EnemyTexFx.h"
#include "core/TMainTask.h"

// 0x8004E630
// is fogRecord the next unit

#define GX_FIFO_F32 (*(volatile f32*)0xCC008000)
#define GX_FIFO_U32 (*(volatile u32*)0xCC008000)

typedef char TObjBlueSea_size_check[(sizeof(TObjBlueSea) == 0x48) ? 1 : -1];

// 0x8004DFD8
// spacing is the distance between rows
static void drawSeaGrid(f32 y, f32 spacing, u32 color)
{
	f32 cell    = spacing * 5.0f;
	f32 countF  = (f32)ceil(14000.0f / cell);
	int count   = (int)countF;
	f32 half    = 0.5f * cell * countF;
	f32 z1      = half;

	GXSetCullMode(GX_CULL_BACK);
	for (int row = 0; row < count; row++) {
		f32 z0 = z1;
		f32 x1 = half;
		z1 -= cell;
		for (int col = 0; col < count; col++) {
			f32 x0 = x1;
			x1 -= cell;
			GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT4, 4);
			GX_FIFO_F32 = x0;
			GX_FIFO_F32 = y;
			GX_FIFO_F32 = z1;
			GX_FIFO_U32 = color;
			GX_FIFO_F32 = 0.0f;
			GX_FIFO_F32 = 5.0f;
			GX_FIFO_F32 = x0;
			GX_FIFO_F32 = y;
			GX_FIFO_F32 = z0;
			GX_FIFO_U32 = color;
			GX_FIFO_F32 = 0.0f;
			GX_FIFO_F32 = 0.0f;
			GX_FIFO_F32 = x1;
			GX_FIFO_F32 = y;
			GX_FIFO_F32 = z1;
			GX_FIFO_U32 = color;
			GX_FIFO_F32 = 5.0f;
			GX_FIFO_F32 = 5.0f;
			GX_FIFO_F32 = x1;
			GX_FIFO_F32 = y;
			GX_FIFO_F32 = z0;
			GX_FIFO_U32 = color;
			GX_FIFO_F32 = 5.0f;
			GX_FIFO_F32 = 0.0f;
		}
	}
}

// 0x8004DCAC
extern "C" void blueSeaDrawTask(TObjBlueSea* self)
{
	self->drawSea();
}

// 0x8004DBA0
extern "C" void initBlueSea()
{
	resBankLoad(&sBlueSeaBank, "ar_obj_blue_sea.arc");
	resBankAddTextures(&sBlueSeaBank, 0, "obj_blue_sea.gvm");
	sBlueSeaTextures = resBankGetTextures(&sBlueSeaBank);
	sBlueSeaColor = new u32;
	*sBlueSeaColor = 0xFFFFFF80;
}

// 0x8004DC0C
extern "C" void termBlueSea()
{
	if (sBlueSeaColor != 0) {
		delete sBlueSeaColor;
		sBlueSeaColor = 0;
	}
	resBankRemoveTextures(&sBlueSeaBank, 0);
	resBankRelease(&sBlueSeaBank);
	sBlueSeaTextures = 0;
}

// 0x8004DC64
extern "C" TObjBlueSea* createTObjBlueSea()
{
	return new TObjBlueSea(gTL_05, 1, 0.0f);
}

// 0x8004DCCC
extern "C" void setBlueSeaColor(f32 blend)
{
	if (sBlueSeaColor != 0) {
		u32 dark  = 0xFFFFFF80;
		u32 light = 0x6F5F7F80;
		((void (*)(u32*, u32*, u32*))lerpColor)(sBlueSeaColor, &dark, &light);
	}
}

const char* gTObjBlueSeaName = "TObjBlueSea";

// 0x8004DD14
TObjBlueSea::TObjBlueSea(TObject* parent, int secondPass, f32 height)
    : TObject(parent)
{
	_00         = (u32)gTObjBlueSeaName;
	mSize       = 0x48;
	mHeight     = height;
	mSecondPass = secondPass;
	mDrawTask   = addDrawTask(this, (void (*)(void*))blueSeaDrawTask, 0x22, 0);
	if (mSecondPass != 0) {
		mWarpTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x2A, 0);
	} else {
		mWarpTask = 0;
	}
	mScrollU = 0.0f;
	mScrollV = 0.0f;
	mWaveU   = 0.0f;
	mWaveV   = 0.0f;
	mAngleU  = (int)(65536.0f * (3.0517578125e-05f * (f32)rand()));
	mAngleV  = (int)(65536.0f * (3.0517578125e-05f * (f32)rand()));
}

// 0x8004DE64
TObjBlueSea::~TObjBlueSea()
{
	if (mWarpTask != 0) {
		removeDrawTask((DrawTask*)mWarpTask);
	}
	removeDrawTask(mDrawTask);
	mWarpTask = 0;
}

// 0x8004DEE4
void TObjBlueSea::draw()
{
	mScrollU += 0.0008f;
	if (mScrollU < 0.0f) {
		mScrollU += 1.0f;
	} else if (mScrollU > 1.0f) {
		mScrollU -= 1.0f;
	}
	mScrollV += -0.0008f;
	if (mScrollV < 0.0f) {
		mScrollV += 1.0f;
	} else if (mScrollV > 1.0f) {
		mScrollV -= 1.0f;
	}
	mAngleU += 0x120;
	mWaveU = 0.6f * njSin(mAngleU);
	mAngleV += 0x150;
	mWaveV = 0.6f * njSin(mAngleV);
}

// 0x8004DFD4
void TObjBlueSea::vfn_10()
{
}

// 0x8004E188
void TObjBlueSea::drawSea()
{
	f32 mtx[12];
	u32 color;

	njSetTexture(sBlueSeaTextures[0]);
	gjLoadTextureNum(0, 1, 1, 0);
	gjLoadPosMatrix(0);
	gjLoadNrmMatrix(0);
	GXSetCurrentMtx(GX_PNMTX0);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
	GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
	PSMTXIdentity(mtx);
	mtx[2] = mScrollU;
	mtx[6] = mScrollV;
	GXLoadTexMtxImm(mtx, GX_TEXMTX0, GX_MTX2x4);
	GXSetNumTexGens(1);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2X4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
	GXSetNumTevStages(1);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_COPY);
	GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0xFF);
	GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_ENABLE);
	if (mSecondPass != 0) {
		GXSetZCompLoc(GX_FALSE);
	} else {
		GXSetZCompLoc(GX_TRUE);
	}
	if (sBlueSeaColor != 0) {
		color = *sBlueSeaColor;
	}
	if (mSecondPass != 0) {
		setColorAlpha(&color, 0x80);
	} else {
		setColorAlpha(&color, 0xFF);
	}
	drawSeaGrid(mHeight, 200.0f, color);
	GXSetTevDirect(GX_TEVSTAGE0);
	GXSetNumIndStages(0);
}

// 0x8004E3CC
void TObjBlueSea::render()
{
	f32 mtx[12];
	f32 ind[6];

	frameTexSetCur();
	gjLoadTextureNum(0, 0, 0, 0);
	genTexLoad(2, 1);
	gjLoadPosMatrix(0);
	gjLoadNrmMatrix(0);
	GXSetCurrentMtx(GX_PNMTX0);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
	GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
	texFxScreenMtx(mtx);
	GXLoadTexMtxImm(mtx, GX_TEXMTX0, GX_MTX3x4);
	GXSetNumTexGens(2);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3X4, GX_TG_POS, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
	GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2X4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
	GXSetNumTevStages(1);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
	ind[0] = mWaveU;
	ind[1] = 0.0f;
	ind[2] = 0.0f;
	ind[3] = 0.0f;
	ind[4] = mWaveV;
	ind[5] = 0.0f;
	GXSetIndTexMtx(GX_ITM_0, ind, -3);
	GXSetNumIndStages(1);
	GXSetIndTexOrder(GX_IND_TEX_STAGE_0, GX_TEXCOORD1, GX_TEXMAP1);
	GXSetIndTexCoordScale(GX_IND_TEX_STAGE_0, GX_ITS_1, GX_ITS_1);
	GXSetTevIndWarp(GX_TEVSTAGE0, GX_IND_TEX_STAGE_0, GX_TRUE, GX_FALSE, GX_ITM_0);
	GXSetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_COPY);
	GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0xFF);
	GXSetZCompLoc(GX_TRUE);
	drawSeaGrid(0.8f, 700.0f, 0xFFFFFFFF);
	GXSetTevDirect(GX_TEVSTAGE0);
	GXSetNumIndStages(0);
}
