#include "Ninja/ninja.h"
struct DrawTask;
#include "enemy/boss/EneBoss.h"
#include "engine/DrawTask.h"
#include "object/ObjArchive.h"
#include "engine/TLensFlare.h"
#include "engine/Viewport.h"
#include "object/ObjAction.h"
#include "egg/EggCreature.h"
#include "core/MathUtil.h"
#include "ui/AeArc.h"
#include "camera/TCameraCtrl.h"
#include "core/VecMath.h"
#include "core/TMainTask.h"
struct NJS_QUAD_TEXTURE;
struct CamView;

typedef char TLensFlare_size_check[(sizeof(TLensFlare) == 0x40) ? 1 : -1];

static inline f32 flareSqrt(f32 x)
{
	if (x > 0.0f) {
		f64 guess = __frsqrte(x);
		guess     = 0.5 * guess * (3.0 - guess * guess * x);
		guess     = 0.5 * guess * (3.0 - guess * guess * x);
		guess     = 0.5 * guess * (3.0 - guess * guess * x);
		return (f32)(x * guess);
	}
	return x;
}

// 0x800D3468
extern "C" TLensFlare* lensFlareCreate(void*, const Vec3f* pos)
{
	return new TLensFlare(gTL_02, pos);
}

const char* gTLensFlareName = "TLensFlare";

// 0x800D34B8
TLensFlare::TLensFlare(TObject* parent, const Vec3f* pos)
    : TObject(parent)
    , mPos(*pos)
{
	mAlpha   = 0xFF;
	mFade    = 0.0f;
	mVisible = 0;
	_00      = (u32)gTLensFlareName;
	mSize    = 0x40;
	mTexture = getStgObjTextures()[44];
	mTask    = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x24, 0);
}

// 0x800D3560
TLensFlare::~TLensFlare()
{
	removeDrawTask(mTask);
}

// 0x800D35C8
void TLensFlare::draw()
{
	if (mVisible != 0) {
		mFade += 1.0f / 30.0f;
		mFade = mFade > 1.0f ? 1.0f : mFade;
		return;
	}
	mFade -= 1.0f / 15.0f;
	mFade = mFade < 0.0f ? 0.0f : mFade;
}

// 0x800D3634
void TLensFlare::vfn_10()
{
	if (mAlpha == 0) {
		mTask->mFlags |= 1;
		return;
	}
	CamView* camera = (CamView*)camGetDrawView();
	if (camera == 0) {
		mTask->mFlags |= 1;
		return;
	}
	Vec3f to;
	vecSub(&to, &mPos, (Vec3f*)camera);
	vecNormalize(&to, &to);
	if (njInnerProduct(&camera->mFront, &mPos) <= 0.0f) {
		mTask->mFlags |= 1;
		return;
	}
	mTask->mSortKey = drawTaskDepthKey(10000.0f);
	mTask->mFlags &= 0xFFFE;
}

// 0x800D3708
void TLensFlare::render()
{
	mVisible = 0;
	if (mTexture == 0) {
		return;
	}
	f32 screen[2];
	njProjectScreen((f32*)0, (const Vec3f*)&mPos, (NJS_POINT2*)screen);
	f32 viewX, viewY, viewW, viewH;
	getViewport(&viewX, &viewY, &viewW, &viewH);
	if (screen[0] < viewX || viewX + viewW < screen[0] || screen[1] < viewY || viewY + viewH < screen[1]) {
		return;
	}

	u32 depth = 0;
	if (sLensFlareInit == 0) {
		sLensFlareFrame = 0;
		sLensFlareInit  = 1;
	}
	sLensFlareFrame++;
	GXSetDrawSync(sLensFlareFrame);
	while (sLensFlareFrame != GXReadDrawSync()) { }
	GXPeekZ((int)screen[0], (int)screen[1], &depth);
	if (depth < 0xFFEC77) {
		if (0.0f == mFade) {
			return;
		}
	} else {
		mVisible = 1;
	}

	f32 centerX = 0.5f * viewW;
	f32 centerY = 0.5f * viewH;
	screen[0] -= centerX;
	screen[1] -= centerY;
	f32 dist = flareSqrt(screen[0] * screen[0] + screen[1] * screen[1]);
	f32 half = flareSqrt(centerX * centerX + centerY * centerY);
	f32 t    = dist / half;

	FlareQuad at = sFlareZeroQuad;
	FlareQuad quad;
	quad.v1 = 0.0f;
	quad.u1 = 0.0f;
	quad.v2 = 1.0f;
	quad.u2 = 1.0f;
	((void (*)(int, void*))pushCullMode)(0, 0);
	njColorBlendingMode(0, 8);
	njColorBlendingMode(1, 0xA);
	njSetTexture(mTexture);
	njQuadTextureStart(1);
	njSetQuadTexture(0, ((u32)((f32)mAlpha * mFade) << 24) | 0xFF0000 | 0xFFFF);

	for (int i = 0; i < 8; i++) {
		FlareSprite* sprite = &sFlareSprites[i];
		f32 size            = lerpF(sprite->mNear, sprite->mFar, t);
		quad.u1             = sprite->mU;
		quad.u2             = 0.125f + sprite->mU;
		at.x1               = screen[0] * sprite->mAlong - size;
		at.y1               = screen[1] * sprite->mAlong - size;
		at.x2               = screen[0] * sprite->mAlong + size;
		at.y2               = screen[1] * sprite->mAlong + size;
		quad.x1             = at.x1 + centerX;
		quad.y1             = at.y1 + centerY;
		quad.x2             = at.x2 + centerX;
		quad.y2             = at.y2 + centerY;
		njDrawQuadTexture((const NJS_QUAD_TEXTURE*)&quad, 0.5f);
	}

	quad.u1 = 0.125f;
	at.x1   = screen[0] - 128.0f;
	at.y1   = screen[1] - 128.0f;
	at.x2   = screen[0] + 128.0f;
	at.y2   = screen[1] + 128.0f;
	quad.x1 = at.x1 + centerX;
	quad.y1 = at.y1 + centerY;
	quad.x2 = at.x2 + centerX;
	quad.y2 = at.y2 + centerY;
	quad.u2 = 0.125f + 0.125f;
	njDrawQuadTexture((const NJS_QUAD_TEXTURE*)&quad, 0.5f);
	njQuadTextureEnd();
	njColorBlendingMode(0, 8);
	njColorBlendingMode(1, 6);
	popCullMode();
}
