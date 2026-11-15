#include "ui/TEnemyLifeBar.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "ui/AeArc.h"
#include "camera/TCameraCtrl.h"
#include "camera/TCamLockOnCursor.h"
struct CamView;

static inline void lifeFastCast()
{
	// clang-format off
	asm {
		li r3, 4
		oris r3, r3, 4
		mtspr 914, r3
		li r3, 5
		oris r3, r3, 5
		mtspr 915, r3
		li r3, 6
		oris r3, r3, 6
		mtspr 916, r3
		li r3, 7
		oris r3, r3, 7
		mtspr 917, r3
	}
	// clang-format on
}

static inline f32 lifeS16ToF32(register const s16* arg)
{
	register f32 ret;
	// clang-format off
	asm {
		psq_l ret, 0(arg), 1, 5
	}
	// clang-format on
	return ret;
}

/** The start of GXRenderModeObj */
struct LifeRenderMode {
	u32 viTVmode;  // _00
	u16 fbWidth;   // _04
	u16 efbHeight; // _06
	u16 xfbHeight; // _08
};

/** What camGetDrawView gives the camera of this frame */
struct LifeCamera {
	u8 _00[0x48];
	f32 mFocal;  // _48 the screen's half width over this is the view's slope
	f32 mAspect; // _4C
};

static inline void fifoF32(f32 v)
{
	*(volatile f32*)0xCC008000 = v;
}

static inline void fifoU32(u32 v)
{
	*(volatile u32*)0xCC008000 = v;
}

// One corner of the bar a 2D position white and its place in the bar's texture
static inline void lifeVertex(f32 x, f32 y, f32 s, f32 t)
{
	fifoF32(x);
	fifoF32(y);
	fifoU32(0xFFFFFFFF);
	fifoF32(s);
	fifoF32(t);
}

// 0x8004BFE4
void LifePart::init()
{
	mKind  = 3;
	mFlags = 0;
	mLevel = 0.0f;
	mDepth = 10.0f;
}

// 0x8004C008
void LifePart::hide()
{
	mFlags |= 1;
}

// 0x8004C018
void LifePart::show()
{
	mFlags &= ~1;
}

// 0x8004C028
void LifePart::update()
{
	if ((mFlags & 1) && mKind != 3) {
		mKind = 2;
	}
	switch (mKind) {
	case 1:
		mLevel += 0.1f;
		if (mLevel >= 1.0f) {
			mLevel = 1.0f;
			mKind  = 0;
		}
		break;
	case 2:
		mLevel -= 0.1f;
		if (mLevel <= 0.0f) {
			mLevel = 0.0f;
			mKind  = 3;
		}
		break;
	}
}

// 0x8004C0C8
// Grows while the bar is on screen and shrinks away when it is not
void LifePart::track(Vec3f* pos, f32 size, f32 max)
{
	if (project(pos, size, max)) {
		if (mKind != 0) {
			mKind = 1;
		}
	} else if (mKind != 3) {
		mKind = 2;
	}
}

// 0x8004C128
// Puts the bar on screen over pos kept inside the edges
int LifePart::project(Vec3f* pos, f32 size, f32 max)
{
	Vec3f view;
	f32 x;
	f32 y;
	njCalcPointEx(pos, &view);
	f32 depth = view.z;
	depth += size;
	if (depth >= 0.0f) {
		return 0;
	}
	depth *= -1.0f;
	if (depth > 250.0f) {
		return 0;
	}
	mDepth             = depth;
	f32 inv            = 1.0f / depth;
	LifeCamera* camera = (LifeCamera*)camGetDrawView();
	if (camera == 0) {
		return 0;
	}
	f32 focal  = camera->mFocal;
	f32 aspect = camera->mAspect;
	f32 side   = (f32)__fabs(view.x) - 30.0f;
	side *= inv;
	if (side > focal) {
		return 0;
	}
	y      = view.y;
	f32 up = (f32)__fabs(y) - 30.0f;
	up *= inv;
	if (aspect * up > focal) {
		return 0;
	}
	max *= 0.1f;
	f32 edge = 0.5f * _gj_rmode_->fbWidth;
	inv *= edge / focal;
	edge -= 20.0f + max;
	x = inv * (view.x + size) + max;
	y *= inv;
	if (x > edge) {
		x = edge;
	} else {
		edge *= -1.0f;
		if (x < edge) {
			x = edge;
		}
	}
	f32 top = 0.5f * _gj_rmode_->xfbHeight;
	top -= 34.0f;
	if (y > top) {
		y = top;
	} else {
		top *= -1.0f;
		if (y < top) {
			y = top;
		}
	}
	mX = x;
	mY = y;
	return 1;
}

// 0x8004C318
// The left cap the empty bar the right cap and the fill all scaled by mLevel
void LifePart::render(u8* frame, f32 max, f32 value)
{
	if (mLevel <= 0.0f) {
		return;
	}
	pushGXCullMode(0);
	loadScreenMatrix(mDepth, 0);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GXSetNumTexGens(1);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2X4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
	GXSetNumTevStages(1);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_ENABLE);
	GXSetZCompLoc(GX_TRUE);
	GXSetAlphaCompare(GX_GREATER, 0x3C, GX_AOP_AND, GX_LEQUAL, 0xFF);

	f32 left = mX - 0.5f * max * mLevel;
	f32 y    = mY;
	GXBegin(GX_QUADS, GX_VTXFMT0, 0x10);
	f32 x0  = left - 40.0f * mLevel;
	f32 top = 21.0f * mLevel + y;
	f32 bot = y - 19.0f * mLevel;
	lifeVertex(x0, top, 0.0f, 0.46875f);
	lifeVertex(left, top, 0.3125f, 0.46875f);
	lifeVertex(left, bot, 0.3125f, 0.78125f);
	lifeVertex(x0, bot, 0.0f, 0.78125f);

	f32 right = max * mLevel + left;
	lifeVertex(left, top, 0.3125f, 0.46875f);
	lifeVertex(right, top, 0.375f, 0.46875f);
	lifeVertex(right, bot, 0.375f, 0.78125f);
	lifeVertex(left, bot, 0.3125f, 0.78125f);

	f32 cap = 12.0f * mLevel + right;
	lifeVertex(right, top, 0.375f, 0.46875f);
	lifeVertex(cap, top, 0.46875f, 0.46875f);
	lifeVertex(cap, bot, 0.46875f, 0.78125f);
	lifeVertex(right, bot, 0.375f, 0.78125f);

	f32 half = 4.0f * mLevel;
	f32 fill = value * mLevel + left;
	f32 ft   = y + half;
	f32 fb   = y - half;
	lifeVertex(left, ft, 0.4765625f, 0.4765625f);
	lifeVertex(fill, ft, 0.53125f, 0.4765625f);
	lifeVertex(fill, fb, 0.53125f, 0.53125f);
	lifeVertex(left, fb, 0.4765625f, 0.53125f);
	popGXCullMode();
}

static const char* sClassName = "TEnemyLifeBar";

inline void TEnemyLifeBar::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}

// 0x8004C6D8
TEnemyLifeBar::TEnemyLifeBar(TObject* parent)
    : TObject(parent)
{
	_00       = (u32)sClassName;
	mSize     = 0xAC;
	mTask     = addDrawTask(this, (void (*)(void*))drawObjectTask, 61, 0);
	u16* bits = &mTask->mFlags;
	*bits &= ~1;

	mTex = 0;
	if (gIcon3dArc != 0) {
		void* lists = findArcExport(gIcon3dArc, "texlists");
		if (lists != 0) {
			mTex = *(void**)lists;
		}
	}

	mPos.x  = 0.0f;
	mPos.y  = 0.0f;
	mPos.z  = 0.0f;
	mRise   = 50.0f;
	mHeight = 20.0f;
	mMax    = 100.0f;
	mValue  = mMax;
	mBlink  = 0;
	for (int i = 0; i < 4; i++) {
		mPart[i].init();
	}
}

// 0x8004C7D8
TEnemyLifeBar::~TEnemyLifeBar()
{
	removeDrawTask(mTask);
}

// 0x8004C840
void TEnemyLifeBar::setMax(s16 health)
{
	lifeFastCast();
	f32 value = lifeS16ToF32(&health);
	value *= 2.0f;
	mMax  = value;
	mMark = 0.1f * value;
}

// 0x8004C8A0
void TEnemyLifeBar::setMark(s16 health)
{
	lifeFastCast();
	f32 value = lifeS16ToF32(&health);
	value *= 2.0f;
	mMark = value;
}

// 0x8004C8F4
void TEnemyLifeBar::setValue(s16 health)
{
	lifeFastCast();
	f32 value = lifeS16ToF32(&health);
	value *= 2.0f;
	mValue = value;
}

// 0x8004C948
void TEnemyLifeBar::setSize(f32 rise, f32 height)
{
	mRise   = rise;
	mHeight = height;
}

// 0x8004C954
void TEnemyLifeBar::setPos(const Vec3f* pos)
{
	mPos.x = pos->x;
	mPos.y = pos->y;
	mPos.z = pos->z;
	mPos.y += mRise;
}

// 0x8004C980
// Clears the hidden bit on every part
void TEnemyLifeBar::show()
{
	for (int i = 0; i < 4; i++) {
		mPart[i].show();
	}
}

// 0x8004C9CC
void TEnemyLifeBar::hide()
{
	for (int i = 0; i < 4; i++) {
		mPart[i].hide();
	}
}

// 0x8004CA18
// While the value sits at or under the mark mBlink cycles through 1..7
void TEnemyLifeBar::draw()
{
	for (int i = 0; i < 4; i++) {
		mPart[i].update();
	}
	if (mValue <= mMark) {
		mBlink = (s16)(mBlink + 1);
		if (mBlink > 7) {
			mBlink = 1;
		}
	} else {
		mBlink = 0;
	}
}

// 0x8004CAA4
void TEnemyLifeBar::vfn_10()
{
	int index = camGetDrawIndex();
	mPart[index].track(&mPos, mHeight, mMax);
}

// 0x8004CAEC
// Blink 0 uses the flat frame
void TEnemyLifeBar::render()
{
	int index = camGetDrawIndex();
	if (mTex != 0) {
		njSetTexture(mTex);
		gjLoadTextureNum(0, 0, 0, 0);
	}
	const u8* frame = sLifeBarFrames;
	if (mBlink > 0) {
		frame = &sLifeBarBlinkFrames[mBlink - 1];
	}
	mPart[index].render((u8*)frame, mMax, mValue);
}
