#include "stage/SetManager.h"
#include "stage/SetActor.h"
#include "engine/DrawTask.h"
#include "object/ObjScript.h"
#include "engine/CharaColli.h"
#include "Ninja/ninja.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "sdk/stl/math.h"
#include "object/ObjAction.h"
#include "engine/FrameTex.h"
#include "engine/GenTex.h"
#include "object/TObjLauncher.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "camera/TCameraCtrl.h"
#include "object/last/TObjLastRipple.h"
#include "core/VecMath.h"
#include "enemy/EnemyTexFx.h"
#include "object/last/TObjLastRippleField.h"

class TObjLastRipple;

extern "C" {
const f32 gLastRippleFieldSetRange = 700.0f;

// 0x802C4710
ObjClassName gTObjLastRippleFieldName = { "TObjLastRippleField", 0 };
}

typedef char TObjLastRippleField_size_check[(sizeof(TObjLastRippleField) == 0xB0) ? 1 : -1];

extern "C" TObject* createTObjLastRippleField(TObject* parent, SetDesc* desc)
{
	return new TObjLastRippleField(parent, desc);
}

// 0x801A81C4
TObjLastRippleField::TObjLastRippleField(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	mScroll   = 0.0f;
	mCooldown = 0;
	_00       = (u32)gTObjLastRippleFieldName.mName;
	mSize     = 0xB0;
	mDraw     = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x2A, 0);
	mShape    = colliCreate(1, 1);
	colliSetOwner((CharaColli*)mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 4, 0xFF, 0xF7, 0xF7);
	initFromDesc(desc);
}

// 0x801A828C
TObjLastRippleField::~TObjLastRippleField()
{
	colliRetire((CharaColli*)mShape);
	mShape = 0;
	removeDrawTask(mDraw);
}

// 0x801A8304
// Int 0 the warp's exponent
static inline f32 rippleSqrt(f32 x)
{
	static const f64 _half  = .5;
	static const f64 _three = 3.0;
	volatile f32 y;
	if (x > 0.0f) {
		f64 guess = __frsqrte((f64)x);
		guess     = _half * guess * (_three - guess * guess * x);
		guess     = _half * guess * (_three - guess * guess * x);
		y         = (f32)(x * (_half * guess * (_three - guess * guess * x)));
		return y;
	}
	return x;
}

void TObjLastRippleField::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mPos.y += 0.1f;
	mIndExp      = desc->getInt(0);
	mIndScale    = 1.0f + desc->getFloat(2);
	mScrollSpeed = 0.01f * desc->getFloat(1);
	mtxSetTransV(mMtx, &mPos);
	mtxRotateY(mMtx, mMtx, mRot.y);
	s32 ry = mRot.y;
	f32 sz;
	f32 sx = 0.5f * desc->getFloat(0);
	sz     = 0.5f * desc->getFloat(0);
	colliShapeMakeBox(colliGetShapes((CharaColli*)mShape), &mPos, ry, sz, 2.5f, sx, 1);
	colliGetShapes((CharaColli*)mShape)->mFlags |= 0x10;
	mReach = rippleSqrt(colliGetShapes((CharaColli*)mShape)->mSizeX * colliGetShapes((CharaColli*)mShape)->mSizeX
	                    + colliGetShapes((CharaColli*)mShape)->mSizeZ * colliGetShapes((CharaColli*)mShape)->mSizeZ);
}

// 0x801A84F4
// What the set file's values mean
void TObjLastRippleField::debugPrint(void*)
{
	njPrintC(0xA0013, ":IND EXP");
	njPrintC(0xA0014, "");
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, "");
	njPrintC(0xA0017, ":XZ-SIZE");
	njPrintC(0xA0018, "*0.01:SCROLL SPD");
	njPrintC(0xA0019, "+1.0:IND SCALE");
	njPrintC(0xA001A, "");
}

// 0x801A85A0
// Starts a ripple under whatever takes a hit from the water then scrolls the warp
void TObjLastRippleField::draw()
{
	if (mCooldown == 0) {
		ColliContact* contact;

		for (contact = ((CharaColli*)mShape)->mContacts; contact != 0; contact = contact->mNext) {
			if (colliContactNoBit10(contact) && colliContactHasBit8(contact)) {
				TObjAction* other = findObjById((u16)colliContactOwner(contact));

				if (other != 0 && other->mHolderId != 0xFFFF) {
					HitInfo hit;

					if (other->onHit(this, 0, (int*)&hit)) {
						Vec3f at = *other->getPos();

						at.y = 0.1f + mPos.y;
						createTObjLastRipple(this, &at);
						mCooldown = 15;
					}
				}
			}
		}
	}
	mCooldown--;
	mCooldown = mCooldown < 0 ? 0 : mCooldown;
	mScroll += mScrollSpeed;
	if (mScroll > 1.0f) {
		mScroll -= 1.0f;
	} else if (mScroll < -1.0f) {
		mScroll += 1.0f;
	}
}

// 0x801A870C
// Out of view within 750 or sorted by depth
void TObjLastRippleField::vfn_10()
{
	Vec3f view;

	if (cameraIsVisible(750.0f, mReach, &mPos, &view)) {
		mDraw->mSortKey = drawTaskDepthKey(view.z);
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x801A8784
// Unlit
static void rippleSetupChans()
{
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	GXSetChanCtrl(GX_ALPHA0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
}

// 0x801A87E8
// Texture matrix 0 maps positions onto the screen copy
static void rippleLoadScreenMtx()
{
	f32 mtx[12];

	texFxScreenMtx(mtx);
	GXLoadTexMtxImm(mtx, GX_TEXMTX0, GX_MTX3x4);
}

// 0x801A881C
// Coordinate 0 from the screen 1 the warp texture scrolled and repeated
static void rippleSetupTexGens(f32 scroll, f32 scale)
{
	f32 mtx[12];

	mtxSetTrans(mtx, 0.0f, scroll, 0.0f);
	mtxScale(scale, scale, scale, mtx, mtx);
	GXLoadTexMtxImm(mtx, GX_TEXMTX1, GX_MTX2x4);
	GXSetNumTexGens(2);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3X4, GX_TG_POS, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
	GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2X4, GX_TG_TEX0, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
}

/** White for the first stage's konst */
static const GXColor sRippleWhite = { 0xFF, 0xFF, 0xFF, 0xFF };

// 0x801A88CC
// Stage 0 the warped screen
static void rippleSetupTev()
{
	GXSetNumTevStages(2);
	GXSetTevKColor(GX_KCOLOR0, sRippleWhite);
	GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
	GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
	GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
	GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_1_8);
	GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR_NULL);
	GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_KONST, GX_CC_ZERO, GX_CC_TEXA, GX_CC_CPREV);
	GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetAlphaCompare(GX_ALWAYS, 0xFF, GX_AOP_AND, GX_ALWAYS, 0xFF);
}

// 0x801A8A2C
// The warp texture bends stage 0's screen coordinates at half strength times 2 to the exp
static void rippleSetupInd(int exp)
{
	f32 mtx[2][3];

	GXSetNumIndStages(1);
	GXSetIndTexOrder(GX_IND_TEX_STAGE_0, GX_TEXCOORD1, GX_TEXMAP1);
	GXSetIndTexCoordScale(GX_IND_TEX_STAGE_0, GX_ITS_1, GX_ITS_1);
	GXSetTevIndWarp(GX_TEVSTAGE0, GX_IND_TEX_STAGE_0, GX_TRUE, GX_FALSE, GX_ITM_0);
	mtx[0][0] = 0.5f;
	mtx[0][1] = 0.0f;
	mtx[0][2] = 0.0f;
	mtx[1][0] = 0.0f;
	mtx[1][1] = 0.5f;
	mtx[1][2] = 0.0f;
	GXSetIndTexMtx(GX_ITM_0, (const float*)mtx, exp);
}

// 0x801A8AC4
// Positions and texture coordinates as floats
static void rippleSetupVtx()
{
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
}

// 0x801A8B2C
void TObjLastRippleField::beginDraw()
{
	gjLoadPosMatrix(0);
	gjLoadNrmMatrix(0);
	gjGXSetBlendMode(0);
	GXSetZCompLoc(GX_TRUE);
	GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_DISABLE);
	rippleSetupChans();
	rippleLoadScreenMtx();
	rippleSetupTexGens(mScroll, mIndScale);
	rippleSetupTev();
	rippleSetupInd(mIndExp);
	rippleSetupVtx();
}

// 0x801A8BA8
void TObjLastRippleField::endDraw()
{
	GXSetTevDirect(GX_TEVSTAGE0);
	GXSetTevDirect(GX_TEVSTAGE1);
	GXSetNumIndStages(0);
}

// 0x801A8CA8
// A fan of count steps round the middle out to the square's corners the circle is clamped to the square
static inline f32 fanRoot(f32 x)
{
	f64 guess = __frsqrte((f64)x);
	guess     = 0.5 * guess * (3.0 - guess * guess * x);
	guess     = 0.5 * guess * (3.0 - guess * guess * x);
	guess     = 0.5 * guess * (3.0 - guess * guess * x);
	vf32 y    = (f32)(x * guess);
	return y;
}

static void rippleDrawFan(u32 count, u8 alpha, f32 radius)
{
	static f32 sRoot2     = dolsqrtf(2.0f);
	static f32 sHalfRoot2 = dolsqrtf(0.5f);
	int angle             = 0;
	u32 step              = 0x10000 / count;
	f32 lo;
	f32 hi;
	f32 reach;
	f32 zero;
	u32 i;
	reach = radius * sRoot2;

	GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, count + 2);
	hi          = sHalfRoot2;
	zero        = 0.0f;
	lo          = -hi;
	GX_FIFO_F32 = zero;
	GX_FIFO_F32 = zero;
	GX_FIFO_F32 = zero;
	GX_FIFO_F32 = zero;
	GX_FIFO_F32 = zero;
	for (i = 0; i < count; i++) {
		f32 c;
		f32 s;
		f32 dist;

		njSinCos(angle, &s, &c);
		c           = c < lo ? lo : (c > hi ? hi : c);
		s           = s < lo ? lo : (s > hi ? hi : s);
		dist        = c * c + s * s;
		f32 x       = c * reach;
		f32 z       = s * reach;
		GX_FIFO_F32 = x;
		GX_FIFO_F32 = zero;
		GX_FIFO_F32 = z;
		if (dist > zero) {
			dist = fanRoot(dist);
		}
		GX_FIFO_F32 = zero;
		GX_FIFO_F32 = dist;
		angle += step;
	}
	hi          = sHalfRoot2;
	GX_FIFO_F32 = reach * hi;
	GX_FIFO_F32 = zero;
	GX_FIFO_F32 = zero;
	GX_FIFO_F32 = zero;
	GX_FIFO_F32 = hi;
}

// 0x801A8BDC
void TObjLastRippleField::render()
{
	f32 mtx[12];

	mtxConcat(mtx, (const f32*)gjGetMatrix(), mMtx);
	njPushMatrix(mtx);
	frameTexSetCur();
	gjLoadTextureNum(0, 0, 0, 0);
	genTexLoadWrap(2, 1, 1);
	genTexLoadWrap(2, 2, 1);
	beginDraw();
	pushGXCullMode(2);
	f32 size = colliGetShapes((CharaColli*)mShape)->mSizeX;
	rippleDrawFan(0x20, 0xFF, size);
	popGXCullMode();
	endDraw();
	njPopMatrixEx();
	if (objDebugEnabled(this)) {
		colliDrawShapes((CharaColli*)mShape);
	}
}

const char* gTObjLastRippleName = "TObjLastRipple";
