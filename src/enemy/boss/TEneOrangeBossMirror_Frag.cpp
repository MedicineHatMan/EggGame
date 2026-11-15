#include "enemy/boss/EneBoss.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "object/ObjAction.h"
#include "Ninja/ninja.h"
#include "core/MathUtil.h"
#include "enemy/boss/TEneOrangeBossMirror_Frag.h"

typedef char MirrorShard_size_check[(sizeof(MirrorShard) == 0x48) ? 1 : -1];

typedef char TEneOrangeBossMirror_Frag_size_check[(sizeof(TEneOrangeBossMirror_Frag) == 0x1228) ? 1 : -1];

static inline f32 randUnit()
{
	return 3.0517578125e-05f * (f32)rand();
}

// lo and span are pooled before the 2^-15 in randUnit
static inline f32 randRange(f32 lo, f32 span)
{
	return span * (3.0517578125e-05f * (f32)rand()) + lo;
}

#define RAND_SIGNED() (2.0f * (3.0517578125e-05f * (f32)rand() - 0.5f))

// 0x801B704C
void MirrorShard::setVel(const Vec3f* from)
{
	mVel.x = mPos.x;
	mVel.y = mPos.y;
	mVel.z = mPos.z;
	njSubVector(&mVel, from);
	mVel.y = 0.0f;
	if (njUnitVector(&mVel) < gUnitVectorMin) {
		mVel = gVecZero;
	}
	gjScaleVector(randRange(0.5f, 1.5f), &mVel, &mVel);
	f32 base = 1.0f;
	mVel.y   = base * randUnit() + base;
}

// Not in the DOL the linker stripped it
f32 mirrorShardRandSigned()
{
	return RAND_SIGNED();
}

// 0x801B7140
void MirrorShard::setSpin(int yaw)
{
	mRotX    = 0;
	mRotY    = yaw;
	int spin = (int)(3000.0f * (2.0f * (3.0517578125e-05f * (f32)rand() - 0.5f)));
	if (spin < 0) {
		spin -= 500;
	} else {
		spin += 500;
	}
	mSpin = spin;
}

// 0x801B71D4
// Three corners about 120 degrees apart each moved a little
void MirrorShard::setCorners(f32 sizeX, f32 sizeY)
{
	f32 s;
	f32 c;
	f32 far = -2.0f * randUnit() + 2.5f;
	njSinCos((int)(24576.0f * RAND_SIGNED()), &s, &c);
	mCorners[0].x = far * (c * sizeX);
	mCorners[0].y = far * (s * sizeY);
	mCorners[0].z = 0.0f;
	far           = -2.0f * randUnit() + 2.5f;
	njSinCos((int)(24576.0f * RAND_SIGNED()) + 0x5555, &s, &c);
	mCorners[1].x = far * (c * sizeX);
	mCorners[1].y = far * (s * sizeY);
	mCorners[1].z = 0.0f;
	far           = -2.0f * randUnit() + 2.5f;
	njSinCos((int)(24576.0f * RAND_SIGNED()) + 0xAAAA, &s, &c);
	mCorners[2].x = far * (c * sizeX);
	mCorners[2].y = far * (s * sizeY);
	mCorners[2].z = 0.0f;
}

// 0x801B7450
void MirrorShard::step()
{
	int spin = mSpin;
	njAddVector(&mPos, &mVel);
	mVel.y += -0.1f;
	if (mPos.y <= 50.0f) {
		mPos.y = 50.0f;
		spin *= -1;
		mVel.x *= 0.8f;
		mVel.y *= -0.5f;
		mVel.z *= 0.8f;
		mSpin = spin;
	}
	mRotY += spin / 3;
	mRotX += spin;
}

const char* gTEneOrangeBossMirror_FragName = "TEneOrangeBossMirror_Frag";

// 0x801B7500
TEneOrangeBossMirror_Frag::TEneOrangeBossMirror_Frag(TObject* parent, const Vec3f* pos, int yaw)
    : TObject(parent)
{
	_00       = (u32)gTEneOrangeBossMirror_FragName;
	mSize     = 0x1228;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	mAlpha    = 0xE8;
	Vec3f from;
	Vec3f across;
	vecFromAngleXZ(1.0f, &across, yaw);
	across.y = 0.0f;
	gjMulAddVector(-40.0f, &from, pos, &across);
	from.y   = 0.0f;
	f32 z    = across.z;
	across.z = -across.x;
	across.x = z;
	int n    = 0;
	for (int row = 0; row < 8; row++) {
		f32 up = 100.0f - (12.5f * (f32)row + 6.25f);
		for (int col = 0; col < 8; col++) {
			gjMulAddVector(6.25f + (12.5f * (f32)col + -50.0f), &mShards[n].mPos, pos, &across);
			mShards[n].mPos.y += up;
			mShards[n].setVel(&from);
			mShards[n].setSpin(yaw);
			mShards[n].setCorners(6.25f, 6.25f);
			n++;
		}
	}
}

// 0x801B770C
MirrorShard::MirrorShard()
{
}

// 0x801B7754
TEneOrangeBossMirror_Frag::~TEneOrangeBossMirror_Frag()
{
	removeDrawTask(mDrawTask);
}

// 0x801B77BC
void TEneOrangeBossMirror_Frag::draw()
{
	mAlpha -= 2;
	if (mAlpha <= 0) {
		mAlpha = 0;
		mFlags |= 1;
	}
	for (int i = 0; i < 64; i++) {
		mShards[i].step();
	}
}

// 0x801B7834
void TEneOrangeBossMirror_Frag::vfn_10()
{
}

static inline void writeVertex(const Vec3f* at, u32 color)
{
	*(volatile f32*)0xCC008000 = at->x;
	*(volatile f32*)0xCC008000 = at->y;
	*(volatile f32*)0xCC008000 = at->z;
	*(volatile u32*)0xCC008000 = color;
}

// White at the given alpha
static inline u32 whiteAlpha(u32 alpha)
{
	return alpha | 0xFFFFFF00;
}

// 0x801B7838
void TEneOrangeBossMirror_Frag::render()
{
	gjLoadPosMatrix(0);
	gjLoadNrmMatrix(0);
	GXSetCurrentMtx(GX_PNMTX0);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
	GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
	GXSetNumTexGens(0);
	GXSetNumTevStages(1);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	pushGXCullMode(0);
	GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_COPY);
	if (mAlpha < 0xFF) {
		GXSetAlphaCompare(GX_GREATER, 0x14, GX_AOP_AND, GX_LEQUAL, 0xFF);
		GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_DISABLE);
		GXSetZCompLoc(GX_FALSE);
	} else {
		GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0xFF);
		GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_ENABLE);
		GXSetZCompLoc(GX_TRUE);
	}
	u32 alpha = mAlpha;
	if (alpha > 0xFF) {
		alpha = 0xFF;
	}
	GXBegin(GX_TRIANGLES, GX_VTXFMT0, 0xC0);
	u32 color = whiteAlpha(alpha);
	for (int i = 0; i < 64; i++) {
		gjPushUnitMatrix();
		njTranslateEx(&mShards[i].mPos);
		gjRotateY(mShards[i].mRotY);
		gjRotateX(mShards[i].mRotX);
		Vec3f at;
		njCalcPointEx(&mShards[i].mCorners[0], &at);
		writeVertex(&at, color);
		njCalcPointEx(&mShards[i].mCorners[1], &at);
		writeVertex(&at, color);
		njCalcPointEx(&mShards[i].mCorners[2], &at);
		writeVertex(&at, color);
		njPopMatrixEx();
	}
	popCullMode();
}
