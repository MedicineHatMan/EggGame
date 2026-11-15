#include "engine/NoiseTex.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "stl/math.h"
#include "Dolphin/OS/OSCache.h"
#include "core/Mem.h"
#include "stage/SetActor.h"
#include "engine/DrawTask.h"
#include "object/ObjAction.h"
#include "stage/TStageOrange.h"
#include "camera/TCamera.h"
#include "Ninja/ninja.h"
#include "object/ObjClassName.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCameraCtrl.h"
#include "core/MathUtil.h"
#include "stage/SetSlot.h"
#include "core/VecMath.h"
#include "object/TObjFractalCloud.h"

#define B 0x100

extern const f32 sNoiseTexMaxLod[1];

// 0x80147D88
NoiseTex::NoiseTex(int width, int height, s32 wrapS, s32 wrapT)
{
	mWidth  = width;
	mHeight = height;
	mSize   = width * height;
	mData   = (u8*)memCalloc(mSize, 1);
	GXInitTexObj(&mTexObj, mData, mWidth, mHeight, GX_TF_I8, (GXTexWrapMode)(wrapS & 0xFFFF), (GXTexWrapMode)(wrapT & 0xFFFF), GX_FALSE);
	GXInitTexObjLOD(&mTexObj, GX_LINEAR, GX_LINEAR, 0.0f, sNoiseTexMaxLod[0], 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
}

__declspec(section ".sdata2") const f32 sNoiseTexMaxLod[1] = { 10.0f };

// 0x80147E30
NoiseTex::~NoiseTex()
{
	memFree(mData);
}

// 0x80147E7C
void NoiseTex::flush()
{
	DCFlushRange(mData, mSize);
}

// 0x80147EA8
// Fills the texture in I8 tile order 8 by 4 texel tiles of 32 bytes
void NoiseTex::fill(f32 freq, f32 x, f32 y, f32 amp)
{
	u32 col;
	u32 row;
	f32 step = (1.0f / B) * freq;
	f32 nx;
	f32 v    = y;
	for (row = 0; row < mHeight; row++) {
		nx = x;
		for (col = 0; col < mWidth; col++) {
			f32 n = 0.5f * (1.0f + noise2(nx, v));
			nx += step;
			mData[((row & 3) << 3) + (col & 7) + ((col >> 3) + (row >> 2) * (mWidth >> 3)) * 32]
			    = (int)(n * amp + 0.5f);
		}
		v += step;
	}
}

// 0x80147FF0
void NoiseTex::load(s32 map)
{
	GXLoadTexObj(&mTexObj, (u16)map);
}

struct SetLink;

extern "C" {
// 0x802C4488
ObjClassName gTObjFractalCloudName = { "TObjFractalCloud", 0 };
}

// One vertex its place its normal and its texture coordinate
static inline void GXVertex(f32 x, f32 y, f32 z, f32 nx, f32 ny, f32 nz, f32 s, f32 t)
{
	GX_FIFO_F32 = x;
	GX_FIFO_F32 = y;
	GX_FIFO_F32 = z;
	GX_FIFO_F32 = nx;
	GX_FIFO_F32 = ny;
	GX_FIFO_F32 = nz;
	GX_FIFO_F32 = s;
	GX_FIFO_F32 = t;
}

typedef char TObjFractalCloud_size_check[(sizeof(TObjFractalCloud) == 0xD0) ? 1 : -1];

// 0x80148014
static const GXColor sPuffWhite = {0xFF, 0xFF, 0xFF, 0xFF};

FractalPuff::FractalPuff()
{
	mTex      = 0;
	_04       = 0.0f;
	_08       = 0.0f;
	mSpeedX   = 0.05f;
	mSpeedY   = 0.05f;
	mFreq     = 8.0f;
	mOctaves  = 4;
	mThreshold = 0;
	*(u32*)&mColor = *(const u32*)&sPuffWhite;
	mScrollU  = 0.0f;
	mScrollV  = 0.0f;
	mDU       = 0.0f;
	mDV       = 0.0f;
	mRepeat   = 8.0f;
}

// 0x80148068
FractalPuff::~FractalPuff()
{
	if (mTex != 0) {
		delete mTex;
	}
}

// 0x801480C0
// A 128 by 128 texture that repeats both ways
bool FractalPuff::build()
{
	if (mTex != 0) {
		delete mTex;
	}
	mTex = new NoiseTex(128, 128, 1, 1);
	return mTex != 0;
}

// 0x80148134
void FractalPuff::setOctaves(int count)
{
	mOctaves = (u8)count < 1 ? 1 : ((u8)count > 4 ? 4 : count);
}

// 0x80148160
void FractalPuff::fill()
{
	mTex->fill(mFreq, 0.0f, 0.0f, 255.0f);
	mTex->flush();
}

// 0x801481A8
void FractalPuff::scroll()
{
	mScrollU += mDU;
	if (mScrollU > 1.0f) {
		mScrollU -= 1.0f;
	} else if (mScrollU < -1.0f) {
		mScrollU += 1.0f;
	}
	mScrollV += mDV;
	if (mScrollV > 1.0f) {
		mScrollV -= 1.0f;
	} else if (mScrollV < -1.0f) {
		mScrollV += 1.0f;
	}
}

// 0x8014822C
void FractalPuff::setColor(u8 r, u8 g, u8 b, u8 a)
{
	mColor.r = r;
	mColor.g = g;
	mColor.b = b;
	mColor.a = a;
}

// 0x80148240
// A two by two sheet in the xz plane
void FractalPuff::draw()
{
	gjGXSetBlendMode(1);
	GXSetZCompLoc(GX_TRUE);
	gjSetZBufferMode(1);
	gjLoadPosMatrix(0);
	gjLoadNrmMatrix(0);
	GXSetCurrentMtx(GX_PNMTX0);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0, GX_TRUE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
	GXSetChanCtrl(GX_ALPHA0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	GXSetChanMatColor(GX_COLOR0A0, mColor);
	mTex->load(GX_TEXMAP0);
	GXSetNumTexGens(mOctaves);

	f32 base[12];
	f32 tex0[12];
	f32 tex1[12];
	f32 tex2[12];
	f32 tex3[12];
	Vec3f scale;
	f32 scrollU = mScrollU;
	f32 scrollV = mScrollV;
	scale.x = mRepeat;
	scale.y = mRepeat;
	scale.z = 0.0f;
	mtxSetTrans(base, scrollU, scrollV, 0.0f);
	mtxScale(scale.x, scale.y, 1.0f, tex0, base);
	vecAdd(&scale, &scale, &scale);
	mtxScale(scale.x, scale.y, 1.0f, tex1, base);
	vecAdd(&scale, &scale, &scale);
	mtxScale(scale.x, scale.y, 1.0f, tex2, base);
	vecAdd(&scale, &scale, &scale);
	mtxScale(scale.x, scale.y, 1.0f, tex3, base);
	GXLoadTexMtxImm(tex0, GX_TEXMTX0, GX_MTX2x4);
	GXLoadTexMtxImm(tex1, GX_TEXMTX0 + 3, GX_MTX2x4);
	GXLoadTexMtxImm(tex2, GX_TEXMTX0 + 6, GX_MTX2x4);
	GXLoadTexMtxImm(tex3, GX_TEXMTX0 + 9, GX_MTX2x4);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2X4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
	GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2X4, GX_TG_TEX0, GX_TEXMTX0 + 3, GX_FALSE, GX_PTIDENTITY);
	GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2X4, GX_TG_TEX0, GX_TEXMTX0 + 6, GX_FALSE, GX_PTIDENTITY);
	GXSetTexCoordGen2(GX_TEXCOORD3, GX_TG_MTX2X4, GX_TG_TEX0, GX_TEXMTX0 + 9, GX_FALSE, GX_PTIDENTITY);

	GXSetNumTevStages(mOctaves + 1);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP0, GX_COLOR_NULL);
	GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD2, GX_TEXMAP0, GX_COLOR_NULL);
	GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD3, GX_TEXMAP0, GX_COLOR_NULL);
	GXSetTevOrder(mOctaves, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GXColor konst = mColor;
	konst.a       = 0xFF - konst.a;
	GXSetTevKColor(GX_KCOLOR0, konst);
	GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_1);
	GXSetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_1_2);
	GXSetTevKColorSel(GX_TEVSTAGE2, GX_TEV_KCSEL_1_4);
	GXSetTevKColorSel(GX_TEVSTAGE3, GX_TEV_KCSEL_1_8);
	GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_1);
	GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_1_2);
	GXSetTevKAlphaSel(GX_TEVSTAGE2, GX_TEV_KASEL_1_4);
	GXSetTevKAlphaSel(GX_TEVSTAGE3, GX_TEV_KASEL_1_8);
	GXSetTevKAlphaSel(mOctaves, GX_TEV_KASEL_K0_A);
	GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXA);
	GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXA, GX_CC_KONST, GX_CC_CPREV);
	GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXA, GX_CC_KONST, GX_CC_CPREV);
	GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_ZERO, GX_CC_TEXA, GX_CC_KONST, GX_CC_CPREV);
	GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevColorIn(mOctaves, GX_CC_ZERO, GX_CC_RASC, GX_CC_CPREV, GX_CC_ZERO);
	GXSetTevColorOp(mOctaves, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
	GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_APREV);
	GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_APREV);
	GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_APREV);
	GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(mOctaves, GX_CA_KONST, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevAlphaOp(mOctaves, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetAlphaCompare(GX_GEQUAL, mThreshold, GX_AOP_AND, GX_ALWAYS, 0xFF);

	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);
	GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	int cull;
	GXGetCullMode((int*)&cull);
	GXSetCullMode(GX_CULL_NONE);
	GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
	f32 lo  = -1.0f;
	f32 mid = 0.0f;
	f32 hi  = 1.0f;
	GXVertex(lo, mid, hi, mid, hi, mid, mid, mid);
	GXVertex(lo, mid, lo, mid, hi, mid, mid, hi);
	GXVertex(hi, mid, hi, mid, hi, mid, hi, mid);
	GXVertex(hi, mid, lo, mid, hi, mid, hi, hi);
	GXSetCullMode(cull);
	GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0xFF);
	GXSetTevDirect(GX_TEVSTAGE0);
}

// 0x801488DC
extern "C" void initTObjFractalCloud()
{
	noiseInit();
}

// 0x801488FC
extern "C" void termTObjFractalCloud() { }

// 0x80148900
extern "C" TObject* createTObjFractalCloud(TObject* parent, SetDesc* desc)
{
	return new TObjFractalCloud(parent, desc);
}

// 0x80148954
TObjFractalCloud::TObjFractalCloud(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjFractalCloudName.mName;
	mSize = 0xD0;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	mPuff.build();
	initFromDesc(desc);
	init();
}

// 0x801489FC
TObjFractalCloud::~TObjFractalCloud()
{
	removeDrawTask(mDraw);
}

// 0x80148A70
void TObjFractalCloud::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mPuff.mThreshold = (int)(f32)(u8)desc->getInt(0);
	f32 dv           = 0.00001f * (f32)desc->getInt(2);
	mPuff.mDU        = 0.00001f * (f32)desc->getInt(1);
	mPuff.mDV        = dv;
	mPuff.mRepeat    = (f32)(s16)desc->getInt(3);
	mScale           = 10.0f + desc->getFloat(0);
	mPuff.mFreq      = desc->getFloat(1);
	mPuff.setOctaves((u8)mRot.x);
	f32 speedY     = 0.001f * desc->getFloat(3);
	mPuff.mSpeedX  = 0.001f * desc->getFloat(2);
	mPuff.mSpeedY  = speedY;
	mPuff.setColor(0xFF, 0xFF, 0xFF, 0xFF - mRot.y);
	mtxSetTransV(mMtx, &mPos);
	mtxScale(mScale, mScale, mScale, mMtx, mMtx);
	mPuff.fill();
}

// 0x80148C94
void TObjFractalCloud::debugPrint(void*)
{
	njPrintC(0xA000A, ":OCTAVE(1-4)");
	njPrintC(0xA000B, ":SUB ALPHA");
	njPrintC(0xA0013, ":THRESHOLD");
	njPrintC(0xA0014, "*0.00001:DU");
	njPrintC(0xA0015, "*0.00001:DV");
	njPrintC(0xA0016, ":REPEAT");
	njPrintC(0xA0017, "+10.0:SIZE");
	njPrintC(0xA0018, ":SCALE");
	njPrintC(0xA0019, "*0.001:X-SPD");
	njPrintC(0xA001A, "*0.001:Y-SPD");
}

// 0x80148D60
void TObjFractalCloud::init()
{
	setLinkFree((SetLink*)&mLink);
}

// 0x80148D84
void TObjFractalCloud::draw()
{
	mPuff.scroll();
}

// 0x80148DA8
void TObjFractalCloud::vfn_10()
{
	Vec3f view;
	if (cameraIsVisible(100000.0f, mScale, &mPos, &view) != 0) {
		mDraw->mSortKey = drawTaskDepthKey(view.z);
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x80148E20
// Not drawn in mirrors
void TObjFractalCloud::render()
{
	if (mirrorIsReflecting() == 0) {
		njPushMatrixEx();
		njMultiMatrix(0, mMtx);
		mPuff.draw();
		njPopMatrixEx();
	}
}
