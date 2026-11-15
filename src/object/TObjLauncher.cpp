#include "object/TObjLauncher.h"
struct DrawTask;
#include "object/ObjArchive.h"
#include "object/ObjModel.h"
#include "engine/DrawState.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "stage/SetTransform.h"
#include "sound/SoundCtrl.h"
#include "engine/DrawTask.h"
#include "object/ObjAction.h"
#include "core/MathUtil.h"
#include "ui/AeArc.h"
#include "stl/math.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "core/VecMath.h"
#include "stage/TEventCtrl.h"
#include "core/SdkCalls.h"
struct _GXColor;
struct GjModel;

extern "C" {

// Set nowhere with it at 1 launched pieces wait 30 frames before they fly unless an event is running
int gLauncherHold;
int _802C4EB4;
DrawStateSlot* gDrawStateSlot;
// Set by enableSSMode and read nowhere
int gSSMode;
}

ObjDrawState gDefaultDrawState;

// The eight corners of a cube with its corners on the unit sphere
static Vec3f gWireBoxVerts[8] = {
	{ 0.57735026f, 0.57735026f, 0.57735026f },    { 0.57735026f, 0.57735026f, -0.57735026f },
	{ -0.57735026f, 0.57735026f, 0.57735026f },   { -0.57735026f, 0.57735026f, -0.57735026f },
	{ 0.57735026f, -0.57735026f, 0.57735026f },   { 0.57735026f, -0.57735026f, -0.57735026f },
	{ -0.57735026f, -0.57735026f, 0.57735026f },  { -0.57735026f, -0.57735026f, -0.57735026f },
};

#define LAUNCH_MIN(a, b) ((a) > (b) ? (b) : (a))

typedef char TObjLauncher_size_check[(sizeof(TObjLauncher) == 0xE4) ? 1 : -1];

inline void TObjLauncher::spin()
{
	int y = (int)(65536.0f * (0.000030517578f * (f32)(int)rand()));
	int x = (int)(65536.0f * (0.000030517578f * (f32)(int)rand()));
	mX.mRot.x = x;
	mX.mRot.y = y;
	mX.mRot.z = 0;
}

const char* gTObjLauncherName = "TObjLauncher";

// 0x800551E0
void TObjLauncher::init(const Vec3f* pos, const Vec3f* vel)
{
	SetTransform* x = &mX;

	x->mGravity.x = 0.0f;
	x->mGravity.y = -0.08166667f;
	x->mGravity.z = 0.0f;
	x->mRadius    = 0.5f;
	x->vfn_0C();
	mX.mVel.x = vel->x;
	mX.mVel.y = vel->y;
	mX.mVel.z = vel->z;
	mX.mPos.set(pos->x, pos->y, pos->z);
	mDrag     = 0.01f;
	mBits     = 0;
	mAge      = 0;
	mSpinMax  = LAUNCH_MIN(1024.0f * vecLength(vel), 1024.0f);
	setRandomSpin();
	if (gLauncherHold == 1 && !eventIsBusy()) {
		mBits |= BIT_DELAYED;
		mDelay = 30;
	}
}

// 0x800552E8
TObjLauncher::TObjLauncher(TObject* parent, f32 floorY, f32 scale, const Vec3f* pos, const Vec3f* vel, int bank,
                           int model, int texture)
    : TObjAction(parent)
{
	ObjCommonData* pack;

	_00      = (u32)gTObjLauncherName;
	mSize    = 0xE4;
	mScale.x = scale;
	mScale.y = scale;
	mScale.z = scale;
	mFloor   = floorY;
	pack     = bank != 0 ? gGeObjCommon : gStgObjCommon;
	mModel   = pack->mFix.mModels[model];
	mMotion  = 0;
	mTexture = pack->mFix.mTexLists[texture];
	init(pos, vel);
	spin();
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	calcMatrix();
}

// 0x80055480
TObjLauncher::TObjLauncher(TObject* parent, f32 floorY, f32 scale, const Vec3f* pos, const Vec3f* vel, void* model,
                           void* texture)
    : TObjAction(parent)
{
	_00      = (u32)gTObjLauncherName;
	mSize    = 0xE4;
	mScale.x = scale;
	mScale.y = scale;
	mScale.z = scale;
	mFloor   = floorY;
	mModel   = (NJS_OBJECT*)model;
	mMotion  = 0;
	mTexture = texture;
	init(pos, vel);
	spin();
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	calcMatrix();
}

// 0x800555E8
TObjLauncher::TObjLauncher(TObject* parent, f32 floorY, const Vec3f* pos, const SetRot* rot, const Vec3f* vel,
                           Vec3f scale, void* motion, void* texture)
    : TObjAction(parent)
{
	_00      = (u32)gTObjLauncherName;
	mSize    = 0xE4;
	mScale.x = scale.x;
	mScale.y = scale.y;
	mScale.z = scale.z;
	mFloor   = floorY;
	mModel   = 0;
	mMotion  = (GjModel*)motion;
	mTexture = texture;
	init(pos, vel);
	mX.mRot = *rot;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	calcMatrix();
}

// 0x800556E0
TObjLauncher::~TObjLauncher()
{
	removeDrawTask(mDraw);
	if ((mBits & BIT_OBJ_ID) != 0) {
		releaseObjId(this);
	}
}

// 0x80055784
void TObjLauncher::draw()
{
	if ((mBits & BIT_DELAYED) != 0) {
		if ((u16)mDelay < 15) {
			mDelay = (s16)(mDelay - 1);
			return;
		}
		if (mDelay == 0) {
			mBits &= ~BIT_DELAYED;
		}
		mDelay = (s16)(mDelay - 1);
	}
	if ((mBits & BIT_STOPPED) == 0) {
		step();
		calcMatrix();
	}
}

// 0x80055808
void TObjLauncher::vfn_10()
{
	if (mAge < 30) {
		mDraw->mFlags &= ~1;
		return;
	}
	if (mAge & 1) {
		mDraw->mFlags &= ~1;
		return;
	}
	mDraw->mFlags |= 1;
}

// 0x80055858
void TObjLauncher::render()
{
	DrawStateSlot* slot;

	njPushMatrixEx();
	njMultiMatrix(0, mMtx);
	if (mModel != 0) {
		njSetTexture(mTexture);
		drawObjectDefault(mModel);
	} else if (mMotion != 0) {
		njSetTexture(mTexture);
		slot = getDrawStateSlot();
		beginDrawState(slot, getDefaultDrawState());
		gjDrawModel(mMotion);
		endDrawState(slot);
	} else {
		GXColor color = { 0xFF, 0xFF, 0xFF, 0x80 };
		debugSphere(&gVecZero, (_GXColor*)&color, 1.0f);
	}
	njPopMatrixEx();
}

// 0x80055910
void TObjLauncher::calcMatrix()
{
	mtxSetTransV(mMtx, &mX.mPos);
	mtxRotateZYX((f32*)mMtx, (const f32*)mMtx, (const int*)&mX.mRot);
	mtxScaleV(mMtx, mMtx, &mScale);
}

// 0x80055964
void TObjLauncher::step()
{
	int i;

	switch (mSpinMode) {
	case 0:
		for (i = 0; i < 3; i++) {
			(&mX.mRot.x)[i] = (&mX.mRot.x)[i] + mSpin[i];
			(&mX.mRot.x)[i] = (&mX.mRot.x)[i] & 0xFFFF;
		}
		break;
	case 1: {
		Vec3f flat = mX.mVel;
		f32 vy     = flat.y;
		f32 len;

		flat.y = 0.0f;
		len    = vecLength(&flat);
		if (len > 0.0f) {
			if (vy > 0.0f || vy < 0.0f) {
				mX.mRot.x = 0x10000 - (int)(10430.38043493439 * (f32)atan2(vy, len));
			}
			mX.mRot.y = (int)(10430.38043493439 * (f32)atan2(mX.mVel.x, mX.mVel.z));
		}
		break;
	}
	}

	if (mX.mPos.y <= mFloor) {
		mBits |= BIT_STOPPED;
		mX.mPos.y = mFloor;
		if ((mBits & BIT_KEEP) == 0) {
			mFlags |= 1;
		}
		return;
	}
	mX.vfn_0C();
	if ((mBits & BIT_DRAG) != 0) {
		transAddDrag(mDrag, &mX);
	}
	mX.vfn_14();
	mX.vfn_18();
	mAge++;
}

// 0x80055B30
void TObjLauncher::setGravity(f32 g)
{
	mX.mGravity.x = 0.0f;
	mX.mGravity.y = -g;
	mX.mGravity.z = 0.0f;
}

// 0x80055B48
void TObjLauncher::noDrag()
{
	mDrag = 0.0f;
	mBits &= ~BIT_DRAG;
}

// 0x80055B60
void TObjLauncher::keepAtFloor()
{
	mBits |= BIT_KEEP;
}

// 0x80055B70
void TObjLauncher::setRandomSpin()
{
	int i;

	mSpinMode = 0;
	for (i = 0; i < 3; i++) {
		f32 min;
		f32 max  = mSpinMax;
		min = -max;
		mSpin[i] = (int)((max - min) * (3.0517578125e-05f * (f32)(int)rand()) + min);
	}
}

// 0x80055C40
void TObjLauncher::faceVelocity()
{
	mSpinMode = 1;
}

// 0x80055C4C
void TObjLauncher::takeObjId()
{
	setObjId(this, 0xFFFF);
	mBits |= BIT_OBJ_ID;
}

// 0x80055C8C
int TObjLauncher::getKind()
{
	return 3;
}

// 0x80055C94
int TObjLauncher::getRotY()
{
	return mX.mRot.y;
}

// 0x80055C9C
void createDrawStateSlot()
{
	gDrawStateSlot = new DrawStateSlot();
}

// 0x80055CD4
void deleteDrawStateSlot()
{
	delete gDrawStateSlot;
}

// 0x80055CFC
DrawStateSlot* getDrawStateSlot()
{
	return gDrawStateSlot;
}

// 0x80055D04
DrawState* getDefaultDrawState()
{
	return &gDefaultDrawState;
}

// 0x80055D10
void enableSSMode()
{
	gSSMode = 1;
}

// 0x80055D1C
void endFrameSwap()
{
	soundUpdate();
	gjSwapBuffers();
}

// 0x80055D40
f32* gjGetMatrix()
{
	return _gj_matrix_;
}

// The colour channels and TEV for a flat colour at mtx
#define debugDrawSetup(mtx, color) \
	do { \
	GXColor c; \
	u32 rgba; \
	GXSetNumTexGens(0); \
	GXSetNumChans(1); \
	GXSetNumTevStages(1); \
	GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR); \
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0); \
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE); \
	GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE); \
	rgba = *color; \
	c.r  = rgba >> 24; \
	c.g  = rgba >> 16; \
	c.b  = rgba >> 8; \
	c.a  = rgba; \
	GXSetChanMatColor(GX_COLOR0A0, c); \
	GXLoadPosMtxImm(mtx, GX_PNMTX0); \
	GXLoadNrmMtxImm(mtx, GX_PNMTX0); \
	GXSetCurrentMtx(GX_PNMTX0); \
	GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0); \
	} while (0)

// 0x80055D48
// Blends unless the colour is opaque
void debugDrawBegin(const f32* mtx, const u32* color)
{
	debugDrawSetup(mtx, color);
	if ((u8)*color == 0xFF) {
		gjGXSetBlendMode(0);
	} else {
		gjGXSetBlendMode(1);
	}
	gjSetPolygonCullingMode();
	gjSetZBufferMode(1);
}

// 0x80055E88
void debugDrawSphere(const f32* mtx, const u32* color)
{
	debugDrawBegin(mtx, color);
	GXDrawSphere1(1);
}

// 0x80055EB0
void debugDrawCylinder(const f32* mtx, const u32* color)
{
	debugDrawBegin(mtx, color);
	GXDrawCylinder(8);
}

// 0x80055ED8
void debugDrawBeginOpaque(const f32* mtx, const u32* color)
{
	debugDrawSetup(mtx, color);
	gjGXSetBlendMode(0);
	gjSetPolygonCullingMode();
	gjSetZBufferMode(1);
}

#define WIRE_PIPE (*(volatile f32*)0xCC008000)
#define WIRE_POS(x, y, z) \
	WIRE_PIPE = (x);      \
	WIRE_PIPE = (y);      \
	WIRE_PIPE = (z)

// 0x80055FFC
// Line strips round each ring of latitude then down each line of longitude
void drawWireSphere(u8 rings, u8 segs)
{
	int ringStep = 0x8000 / rings;
	int segStep  = 0x10000 / segs;
	f32 ringSin, ringCos, segSin, segCos;
	int i, j;
	int a, b;

	a = ringStep;
	for (i = rings - 1; i > 0; i--) {
		njSinCos(a, &ringSin, &ringCos);
		GXBegin(GX_LINESTRIP, GX_VTXFMT4, segs + 1);
		WIRE_POS(ringSin, ringCos, 0.0f);
		b = segStep;
		for (j = segs - 1; j > 0; j--) {
			njSinCos(b, &segSin, &segCos);
			b += segStep;
			WIRE_POS(ringSin * segCos, ringCos, ringSin * segSin);
		}
		a += ringStep;
		WIRE_POS(ringSin, ringCos, 0.0f);
	}

	b = 0;
	for (i = segs; i > 0; i--) {
		njSinCos(b, &segSin, &segCos);
		GXBegin(GX_LINESTRIP, GX_VTXFMT4, rings + 1);
		WIRE_POS(0.0f, 1.0f, 0.0f);
		a = ringStep;
		for (j = rings - 1; j > 0; j--) {
			njSinCos(a, &ringSin, &ringCos);
			a += ringStep;
			WIRE_POS(ringSin * segCos, ringCos, ringSin * segSin);
		}
		WIRE_POS(0.0f, -1.0f, 0.0f);
		b += segStep;
	}
}

// 0x800561E4
void debugWireSphere(const f32* mtx, const u32* color)
{
	debugDrawBeginOpaque(mtx, color);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT4, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	drawWireSphere(4, 8);
}

// 0x80056238
// A unit cylinder from y = -1 to 1 the top and bottom rims then the sides
void drawWireCylinder(u8 segs)
{
	int step = 0x10000 / segs;
	f32 s, c;
	int i;
	int a;

	GXBegin(GX_LINESTRIP, GX_VTXFMT4, segs + 1);
	WIRE_POS(1.0f, 1.0f, 0.0f);
	a = step;
	for (i = segs - 1; i > 0; i--) {
		njSinCos(a, &s, &c);
		a += step;
		WIRE_POS(c, 1.0f, s);
	}
	WIRE_POS(1.0f, 1.0f, 0.0f);

	GXBegin(GX_LINESTRIP, GX_VTXFMT4, segs + 1);
	WIRE_POS(1.0f, -1.0f, 0.0f);
	a = step;
	for (i = segs - 1; i > 0; i--) {
		njSinCos(a, &s, &c);
		a += step;
		WIRE_POS(c, -1.0f, s);
	}
	WIRE_POS(1.0f, -1.0f, 0.0f);

	GXBegin(GX_LINES, GX_VTXFMT4, (segs + 1) * 2);
	WIRE_POS(1.0f, 1.0f, 0.0f);
	WIRE_POS(1.0f, -1.0f, 0.0f);
	a = step;
	for (i = segs - 1; i > 0; i--) {
		njSinCos(a, &s, &c);
		a += step;
		WIRE_POS(c, 1.0f, s);
		WIRE_POS(c, -1.0f, s);
	}
	WIRE_POS(1.0f, 1.0f, 0.0f);
	WIRE_POS(1.0f, -1.0f, 0.0f);
}

// 0x80056440
void debugWireCylinder(const f32* mtx, const u32* color)
{
	debugDrawBeginOpaque(mtx, color);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT4, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	drawWireCylinder(8);
}

#define GX_INDEX8(i) (*(volatile u8*)0xCC008000 = (i))

// 0x80056490
// The top and bottom squares then the four edges between them
void debugWireBox(const f32* mtx, const u32* color)
{
	debugDrawBeginOpaque(mtx, color);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
	GXSetVtxAttrFmt(GX_VTXFMT4, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetArray(GX_VA_POS, gWireBoxVerts, 12);

	GXBegin(GX_LINESTRIP, GX_VTXFMT4, 5);
	GX_INDEX8(0);
	GX_INDEX8(1);
	GX_INDEX8(3);
	GX_INDEX8(2);
	GX_INDEX8(0);

	GXBegin(GX_LINESTRIP, GX_VTXFMT4, 5);
	GX_INDEX8(4);
	GX_INDEX8(5);
	GX_INDEX8(7);
	GX_INDEX8(6);
	GX_INDEX8(4);

	GXBegin(GX_LINES, GX_VTXFMT4, 8);
	GX_INDEX8(0);
	GX_INDEX8(4);
	GX_INDEX8(1);
	GX_INDEX8(5);
	GX_INDEX8(2);
	GX_INDEX8(6);
	GX_INDEX8(3);
	GX_INDEX8(7);
}
