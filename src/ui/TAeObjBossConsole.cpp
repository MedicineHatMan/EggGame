#include "ui/TAeObjBossConsole.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "engine/RingDraw.h"
#include "core/dolphin.h"
#include "egg/ObjEgg.h"
#include "object/ObjAction.h"
#include "object/ObjShadow.h"
#include "stl/math.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "ui/TAeLayer.h"
#include "ui/BossConsole.h"

// TAeObjBossConsole class at 0x800F2634 Vtable 0x8025C430
// The boss's life bar two bars the back one trailing and a face layer bound into the first

typedef char TAeObjBossConsole_size_check[(sizeof(TAeObjBossConsole) == 0x48) ? 1 : -1];

static inline void park(TAeLayer* layer, int hold)
{
	if (layer == 0) {
		return;
	}
	layer->mBanks = sBossConsoleBanks;
	TAeClip* clip = static_cast<TAeClip*>(layer);
	clip->cue(1);
	clip->setRate(1.5f);
	clip->setMode(0x20);
	layer->_AC = 2;
	layer->_A8 = 0.0f;
	clip->show();
	if (hold != 0) {
		clip->hold();
	}
}

// 0x800F2634
TAeObjBossConsole::TAeObjBossConsole(TObject* parent, void* data, void* assets)
    : TAeObject(parent, data, assets)
{
	_00   = (u32)gTAeObjBossConsoleName;
	mSize = 0x48;
	mBar  = createLayer(0xB7, 0);
	park(mBar, 0);
	mBarBack = createLayer(0xBA, 0);
	park(mBarBack, 1);
	mFace = createLayer(sBossConsoleOpen, 0);
	if (mFace != 0) {
		mFace->mBanks = sBossConsoleBanks;
	}
	if (mBar != 0 && mFace != 0) {
		mBar->bind(0xB5, static_cast<TAeClip*>(mFace));
	}
	mTask    = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3C, 0);
	mClosing = 0;
}

// 0x800F2810
TAeObjBossConsole::~TAeObjBossConsole()
{
	removeDrawTask(mTask);
	if (sBossConsole == this) {
		sBossConsole = 0;
	}
}

// 0x800F288C
void TAeObjBossConsole::draw()
{
	if (mBar == 0 && mBarBack == 0) {
		return;
	}
	TAeClip& clip = *mBar;
	f32 end       = clip.mEnd - clip._68 * gAeFrameTime;
	if (end == clip.mTime) {
		clip.hide();
		mBarBack->hide();
	}
	if (mClosing != 0) {
		int done2 = mBarBack->play();
		int done  = mBar->play();
		if (done2 != 0 && done != 0) {
			mClosing = 0;
			if (mFace != 0) {
				mFace->reset(sBossConsoleOpen, 1);
				mFace->mBanks = sBossConsoleBanks;
			}
			if (mBarBack != 0) {
				mBarBack->hold();
			}
		}
	} else {
		mBar->play();
		mBarBack->play();
	}
	if (mFace != 0) {
		mFace->update();
	}
}

// 0x800F2994
void TAeObjBossConsole::render()
{
	if (mBar != 0) {
		mBar->present();
	}
	if (mBarBack != 0) {
		mBarBack->present();
	}
}

// 0x800F29DC
// Hands a message to the boss console if there is one
extern "C" int bossConsoleMsg(BossConsoleMsg* msg)
{
	if (sBossConsole != 0) {
		return sBossConsole->vfn_20(&msg->mCode);
	}
	return 0;
}

// 0x800F2A24
// Moves both bars toward the boss's life
int TAeObjBossConsole::setLife(BossConsoleMsg* msg)
{
	if (mBar == 0 && mBarBack == 0) {
		return 0;
	}
	f32 time      = (1.0f - msg->mLife) * mBar->mEnd;
	mBar->_A8     = time;
	mBarBack->_A8 = time;
	return 1;
}

// 0x800F2A6C
// Plays the console out draw() closes it once both bars are there
int TAeObjBossConsole::close()
{
	if (mBar == 0) {
		return 0;
	}
	mClosing = TRUE;
	if (mFace != 0) {
		mFace->reset(sBossConsoleClose, 1);
		mFace->mBanks = sBossConsoleBanks;
	}
	if (mBarBack != 0) {
		mBarBack->release();
	}
	return 1;
}

// 0x800F2AF0
int TAeObjBossConsole::vfn_20(s16* code)
{
	switch (*(u16*)code) {
	case 0xB9:
		return setLife((BossConsoleMsg*)code);
	case 0xBA:
		return close();
	}
	return 0;
}

#define GX_FIFO_U8  (*(volatile u8*)0xCC008000)
#define GX_FIFO_F32 (*(volatile f32*)0xCC008000)

/** A Vec3f the function statics are built with */
struct RingVec : public Vec3f {
	RingVec(f32 nx, f32 ny, f32 nz)
	{
		x = nx;
		y = ny;
		z = nz;
	}
};

// The SDK's sqrtf three Newton steps from the estimate
static inline f32 ringSqrt(f32 x)
{
	volatile f32 y;
	if (x > 0.0f) {
		f64 guess = __frsqrte((f64)x);
		guess     = 0.5 * guess * (3.0 - guess * guess * x);
		guess     = 0.5 * guess * (3.0 - guess * guess * x);
		guess     = 0.5 * guess * (3.0 - guess * guess * x);
		y         = (f32)(x * guess);
		return y;
	}
	return x;
}

static HitVec** sRingPos;    // 0x802C53F0 [segment][side]
static HitVec** sRingNormal; // 0x802C53F4
static HitVec** sRingShadow; // 0x802C53F8 [segment][4]
static HitVec* sRingCenters; // 0x802C53FC each segment's direction from the middle

static void ringBuild();
static void ringFree();

// 0x800F2B3C
extern "C" void ringDrawInit()
{
	ringBuild();
}

// 0x800F2B5C
extern "C" void ringDrawTerm()
{
	ringFree();
}

// 0x800F2B7C
// A tube of n sides along z from z0 to z1 radius wide lit with the colour
extern "C" void drawTube(const f32* mtx, u8 n, const u32* color, f32 z0, f32 z1, f32 radius)
{
	f32 m[12];
	GXColor rgba;
	f32 c;
	f32 s;
	int angle;
	int step;
	u8 i;

	if (n < 3) {
		return;
	}
	mtxScale(radius, radius, 1.0f, m, mtx);
	GXSetNumTexGens(0);
	GXSetNumChans(1);
	GXSetNumTevStages(1);
	GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GXSetChanCtrl(GX_COLOR0, GX_ENABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
	GXSetChanCtrl(GX_ALPHA0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	rgba.r = *color >> 24;
	rgba.g = *color >> 16;
	rgba.b = *color >> 8;
	rgba.a = *color;
	GXSetChanMatColor(GX_COLOR0A0, rgba);
	GXLoadPosMtxImm(m, GX_PNMTX0);
	GXLoadNrmMtxImm(m, GX_PNMTX0);
	GXSetCurrentMtx(GX_PNMTX0);
	if ((*color & 0xFF) == 0xFF) {
		gjGXSetBlendMode(0);
	} else {
		gjGXSetBlendMode(1);
	}
	gjSetPolygonCullingMode();
	gjSetZBufferMode(1);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
	step = 0x10000;
	step /= n;
	angle = 0;
	GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, (u16)(n * 2 + 2));
	f32 zero = 0.0f;
	for (i = 0; i < n; i++) {
		njSinCos(angle, &s, &c);
		angle += step;
		f32 x = c;
		f32 y = s;
		GXPosition3f32(x, y, z0);
		GXNormal3f32(x, y, zero);
		GXPosition3f32(x, y, z1);
		GXNormal3f32(x, y, zero);
	}
	f32 x = 1.0f;
	f32 y = 0.0f;
	GXPosition3f32(x, y, z0);
	GXNormal3f32(x, y, y);
	GXPosition3f32(x, y, z1);
	GXNormal3f32(x, y, y);
}

// 0x800F2E20
// count rows of n points
static HitVec** allocRing(u32 count, u32 n)
{
	HitVec** rows = new HitVec*[count];
	for (u32 i = 0; i < count; i++) {
		rows[i] = new HitVec[n];
	}
	return rows;
}

// 0x800F2EA8
static void freeRing(HitVec** rows, u32 count)
{
	if (count != 0) {
		do {
			count--;
			delete[] rows[count];
		} while (count != 0);
		delete[] rows;
	}
}

// 0x800F2F08
// Builds the hoop the tube's points and normals its middle directions and the shadow's points
static void ringBuild()
{
	static RingVec sOut(0.0f, 0.0f, 3.0f);                     // the tube's middle from the hoop's
	static RingVec sTube(0.0f, 0.0f, 2.0f);                    // the tube's radius
	static RingVec sShadow(0.0f, 0.0f, 2.0f * ringSqrt(2.0f)); // to the corners of a square round the tube
	static RingVec sForward(0.0f, 0.0f, 1.0f);
	f32 rot[12];
	f32 m[12];
	int seg;
	int side;
	int yaw;
	int pitch;

	sRingPos     = allocRing(sRingSegments, sRingSides);
	sRingNormal  = allocRing(sRingSegments, sRingSides);
	sRingShadow  = allocRing(16, 4);
	sRingCenters = new HitVec[16];
	for (seg = 0, yaw = 0; seg < 16; seg++, yaw += 0x1000) {
		mtxSetRotY(rot, yaw);
		for (side = 0, pitch = 0; side < 8; side++, pitch += 0x2000) {
			mtxTranslateV(m, rot, &sOut);
			mtxRotateX(m, m, pitch);
			mtxMultVec(m, &sTube, &sRingPos[seg][side]);
			mtxRotateX(m, rot, pitch);
			mtxMultVec(m, &sForward, &sRingNormal[seg][side]);
		}
		vecRotateY(&sForward, &sRingCenters[seg], yaw);
	}
	for (seg = 0, yaw = 0; seg < 16; seg++, yaw += 0x1000) {
		mtxSetRotY(rot, yaw);
		for (side = 0, pitch = -0x2000; side < 4; side++, pitch += 0x4000) {
			mtxTranslateV(m, rot, &sOut);
			mtxRotateX(m, m, pitch);
			mtxMultVec(m, &sShadow, &sRingShadow[seg][side]);
		}
	}
}

// 0x800F31D8
static void ringFree()
{
	delete[] sRingCenters;
	freeRing(sRingShadow, sRingSegments);
	freeRing(sRingNormal, sRingSegments);
	freeRing(sRingPos, sRingSegments);
}

// 0x800F322C
RingDraw::RingDraw()
{
	mRadius    = 15.0f;
	mLod       = 0;
	mShadowLod = 0;
	mColor     = 0xFFFFFFFF;
}

// 0x800F324C
RingDraw::~RingDraw()
{
}

// 0x800F3288
f32 RingDraw::getWidth()
{
	return 2.0f;
}

// 0x800F3290
void RingDraw::draw()
{
	f32 m[12];
	GXColor rgba;

	pushGXCullMode(2);
	gjSetZBufferMode(1);
	gjGXSetBlendMode(0);
	PSMTXIdentity(m);
	GXLoadTexMtxImm(m, GX_TEXMTX0, GX_MTX2x4);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2X4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0, GX_ENABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
	GXSetChanCtrl(GX_ALPHA0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	rgba.r = mColor >> 24;
	rgba.g = mColor >> 16;
	rgba.b = mColor >> 8;
	rgba.a = mColor;
	GXSetChanMatColor(GX_COLOR0, rgba);
	GXSetNumTexGens(0);
	GXSetNumTevStages(1);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
	drawHoop();
	popGXCullMode();
}

static inline void loadSegment(f32* m, const f32* view, const HitVec* center, f32 radius)
{
	Vec3f at;

	gjScaleVector(radius, &at, center);
	Vec3f offset = at;
	mtxTranslateV(m, view, &offset);
}

// 0x800F3414
// Each segment takes the next matrix slot
void RingDraw::drawHoop()
{
	f32 view[12];
	f32 m[12];
	int segStep;
	int sideStep;
	int seg;
	int slot;
	int side;
	u32 prevId;
	u32 id;

	switch (mLod) {
	case 0:
		segStep  = 1;
		sideStep = 1;
		break;
	case 1:
		segStep  = 2;
		sideStep = 2;
		break;
	default:
		segStep  = 3;
		sideStep = 2;
		break;
	}
	njGetMatrix(view);
	id = sRingMtxIds[0];
	loadSegment(m, view, sRingCenters, mRadius);
	GXLoadPosMtxImm(m, id);
	GXLoadNrmMtxImm(m, id);
	slot = segStep;
	for (seg = 0; seg < 16; seg += (u8)segStep) {
		int next     = (seg + (u8)segStep) % 16;
		HitVec* pos0 = sRingPos[seg];
		HitVec* nrm0 = sRingNormal[seg];
		HitVec* pos1 = sRingPos[next];
		HitVec* nrm1 = sRingNormal[next];
		loadSegment(m, view, &sRingCenters[next], mRadius);
		prevId = id;
		id     = sRingMtxIds[slot & 7];
		GXLoadPosMtxImm(m, id);
		GXLoadNrmMtxImm(m, id);
		GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, (u16)((u16)(8 / (u8)sideStep) + 1) * 2);
		for (side = 0; side < 8; side += (u8)sideStep) {
			GX_FIFO_U8  = prevId;
			GX_FIFO_F32 = pos0[side].x;
			GX_FIFO_F32 = pos0[side].y;
			GX_FIFO_F32 = pos0[side].z;
			GX_FIFO_F32 = nrm0[side].x;
			GX_FIFO_F32 = nrm0[side].y;
			GX_FIFO_F32 = nrm0[side].z;
			GX_FIFO_U8  = id;
			GX_FIFO_F32 = pos1[side].x;
			GX_FIFO_F32 = pos1[side].y;
			GX_FIFO_F32 = pos1[side].z;
			GX_FIFO_F32 = nrm1[side].x;
			GX_FIFO_F32 = nrm1[side].y;
			GX_FIFO_F32 = nrm1[side].z;
		}
		GX_FIFO_U8  = prevId;
		GX_FIFO_F32 = pos0[0].x;
		GX_FIFO_F32 = pos0[0].y;
		GX_FIFO_F32 = pos0[0].z;
		GX_FIFO_F32 = nrm0[0].x;
		GX_FIFO_F32 = nrm0[0].y;
		GX_FIFO_F32 = nrm0[0].z;
		GX_FIFO_U8  = id;
		GX_FIFO_F32 = pos1[0].x;
		GX_FIFO_F32 = pos1[0].y;
		GX_FIFO_F32 = pos1[0].z;
		GX_FIFO_F32 = nrm1[0].x;
		GX_FIFO_F32 = nrm1[0].y;
		GX_FIFO_F32 = nrm1[0].z;
		slot += segStep;
	}
}

// 0x800F36C4
// The shadow volume drawn once to mark the stencil and once to clear it
void RingDraw::drawShadow()
{
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	GXSetNumTexGens(0);
	GXSetNumTevStages(1);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	shadowVolumeBegin();
	shadowVolumeAdd();
	GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_COPY);
	drawShadowHoop();
	shadowVolumeSubtract();
	GXSetBlendMode(GX_BM_SUBTRACT, GX_BL_ONE, GX_BL_ONE, GX_LO_COPY);
	drawShadowHoop();
	shadowVolumeEnd();
}

// 0x800F37C0
void RingDraw::drawShadowHoop()
{
	f32 view[12];
	f32 m[12];
	int segStep;
	int seg;
	int slot;
	int side;
	u32 prevId;
	u32 id;

	switch (mShadowLod) {
	case 0:
		segStep = 1;
		break;
	case 1:
		segStep = 2;
		break;
	case 2:
		segStep = 4;
		break;
	}
	njGetMatrix(view);
	id = sRingMtxIds[0];
	loadSegment(m, view, sRingCenters, mRadius);
	GXLoadPosMtxImm(m, id);
	slot = segStep;
	for (seg = 0; seg < 16; seg += (u8)segStep) {
		int next     = (seg + (u8)segStep) % 16;
		HitVec* pos0 = sRingShadow[seg];
		HitVec* pos1 = sRingShadow[next];
		loadSegment(m, view, &sRingCenters[next], mRadius);
		prevId = id;
		id     = sRingMtxIds[slot & 7];
		GXLoadPosMtxImm(m, id);
		GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 10);
		for (side = 0; side < 4; side++) {
			GX_FIFO_U8  = prevId;
			GX_FIFO_F32 = pos0[side].x;
			GX_FIFO_F32 = pos0[side].y;
			GX_FIFO_F32 = pos0[side].z;
			GX_FIFO_U8  = id;
			GX_FIFO_F32 = pos1[side].x;
			GX_FIFO_F32 = pos1[side].y;
			GX_FIFO_F32 = pos1[side].z;
		}
		GX_FIFO_U8  = prevId;
		GX_FIFO_F32 = pos0[0].x;
		GX_FIFO_F32 = pos0[0].y;
		GX_FIFO_F32 = pos0[0].z;
		GX_FIFO_U8  = id;
		GX_FIFO_F32 = pos1[0].x;
		GX_FIFO_F32 = pos1[0].y;
		GX_FIFO_F32 = pos1[0].z;
		slot += segStep;
	}
}
