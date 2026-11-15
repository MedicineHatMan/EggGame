#include "enemy/boss/EneBoss.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "ui/TFadeModel.h"
#include "ui/Fill2D.h"
#include "Ninja/ninja.h"
#include "ui/TFade.h"
#include "core/TMainTask.h"

typedef char TWhiteFadeInOut_size_check[(sizeof(TWhiteFadeInOut) == 0x30) ? 1 : -1];

typedef char TBlackFadeInOut_size_check[(sizeof(TBlackFadeInOut) == 0x30) ? 1 : -1];

/**
 * @size 0x58
 */
class TCircleFadeInOut : public TObject {
public:
	TCircleFadeInOut(TObject* parent, int close, f32 x, f32 y, f32 scaleX, f32 scaleY);
	virtual ~TCircleFadeInOut();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	static void drawTask(TCircleFadeInOut* fade);

	u16 mFrames;     // _20
	DrawTask* mTask; // _24
	f32 mRadius;     // _28
	f32 mTarget;     // _2C
	f32 mX;          // _30
	f32 mY;          // _34
	f32 mScaleX;     // _38
	f32 mScaleY;     // _3C
	u8 _40[0x18];    // _40
};

typedef char TCircleFadeInOut_size_check[(sizeof(TCircleFadeInOut) == 0x58) ? 1 : -1];

typedef char TAlphaFade_size_check[(sizeof(TAlphaFade) == 0x30) ? 1 : -1];

/**
 * @size 0x10
 */
struct FadeVertex {
	f32 x;      // _00
	f32 y;      // _04
	f32 z;      // _08
	u32 mColor; // _0C
};

// 0x80111BB8
extern "C" void fadeToWhite()
{
	new TWhiteFadeInOut(gTL_01, 1);
}

// 0x80111BF4
extern "C" void fadeToBlack()
{
	new TBlackFadeInOut(gTL_01, 1);
}

// 0x80111C30
extern "C" void fadeCircleOpen()
{
	new TCircleFadeInOut(gTL_01, 0, 0.0f, 0.0f, 1.0f, 1.0f);
}

// 0x80111C7C
extern "C" void fadeCircleClose()
{
	new TCircleFadeInOut(gTL_01, 1, 0.0f, 0.0f, 1.0f, 1.0f);
}

// Stripped from the DOL Its 320 and 240 the screen centre come before the int-to-float constant in the pool
extern "C" void fadeCircleCentred(int close)
{
	new TCircleFadeInOut(gTL_01, close, 320.0f, 240.0f, 1.0f, 1.0f);
}

static inline void* fadeModelAlloc()
{
	return objHeapAlloc(gObjHeap, 0x3C);
}

// 0x80111CC8
extern "C" void fadeModel00()
{
	new TFadeModel(gTL_01, 0, 0);
}

// 0x80111D08
extern "C" void fadeModel01()
{
	new TFadeModel(gTL_01, 0, 1);
}

// 0x80111D48
extern "C" void fadeModel10()
{
	new TFadeModel(gTL_01, 1, 0);
}

// 0x80111D88
extern "C" void fadeModel11()
{
	new TFadeModel(gTL_01, 1, 1);
}

// 0x80111DC8
// Dims the screen drawing in the given task slot
extern "C" void alphaFadeDim(int slot)
{
	if (sAlphaFadeObj != 0) {
		sAlphaFadeObj->mClearing = 0;
		moveDrawTask(sAlphaFadeObj->mTask, slot);
	} else {
		TAlphaFade* fade = new TAlphaFade(gTL_08, 0);
		sAlphaFadeObj    = fade;
		if (fade != 0) {
			moveDrawTask(fade->mTask, slot);
		}
	}
}

// 0x80111E54
extern "C" void alphaFadeClear()
{
	if (sAlphaFadeObj != 0) {
		sAlphaFadeObj->mClearing = 1;
	} else {
		sAlphaFadeObj = new TAlphaFade(gTL_08, 1);
	}
}

// 0x80111EB0
extern "C" void alphaFadeDelete()
{
	if (sAlphaFadeObj != 0) {
		sAlphaFadeObj->mFlags |= 1;
	}
}

const char* gTWhiteFadeInOutName = "TWhiteFadeInOut";

// 0x80111ECC
TWhiteFadeInOut::TWhiteFadeInOut(TObject* parent, int toWhite)
    : TObject(parent)
{
	_00   = (u32)gTWhiteFadeInOutName;
	mSize = 0x30;
	mTask = addDrawTask(this, (void (*)(void*))drawTask, 0x3E, 0);
	if (toWhite == 0) {
		mAlpha  = 1.0f;
		mTarget = 0.0f;
	} else {
		mAlpha  = 0.0f;
		mTarget = 1.0f;
	}
	mFrames = 0x3C;
}

// 0x80111F6C
TWhiteFadeInOut::~TWhiteFadeInOut()
{
	removeDrawTask(mTask);
}

// 0x80111FD4
void TWhiteFadeInOut::draw()
{
	if (mFrames != 0) {
		mAlpha += (mTarget - mAlpha) / (f32)mFrames;
		mFrames--;
	}
}

static inline void fadeFillScreen(u32 color)
{
	f32 rect[4];
	blendModeAlpha();
	fill2DBegin(1);
	fill2DSetColor(color);
	rect[0] = 0.0f;
	rect[2] = 640.0f;
	rect[1] = 0.0f;
	rect[3] = 480.0f;
	((void (*)(f32, const f32*))fill2DRect)(1.0f, rect);
	fill2DEnd();
}

// Stripped from the DOL Its fill comes before the white drawTask which puts 640 and 480 ahead of 255 in the pool
extern "C" void fadeFill(u32 color)
{
	fadeFillScreen(color);
}

// 0x80112028
void TWhiteFadeInOut::drawTask(TWhiteFadeInOut* fade)
{
	u32 c = 0xFFFFFF00 | ((int)(255.0f * fade->mAlpha) & 0xFF);
	fadeFillScreen(((c << 24) & 0xFF000000) | (c >> 8));
}

const char* gTBlackFadeInOutName = "TBlackFadeInOut";

// 0x801120B0
TBlackFadeInOut::TBlackFadeInOut(TObject* parent, int toBlack)
    : TObject(parent)
{
	_00   = (u32)gTBlackFadeInOutName;
	mSize = 0x30;
	mTask = addDrawTask(this, (void (*)(void*))drawTask, 0x3E, 0);
	if (toBlack == 0) {
		mAlpha  = 1.0f;
		mTarget = 0.0f;
	} else {
		mAlpha  = 0.0f;
		mTarget = 1.0f;
	}
	mFrames = 0x3C;
}

// 0x80112150
TBlackFadeInOut::~TBlackFadeInOut()
{
	removeDrawTask(mTask);
}

// 0x801121B8
void TBlackFadeInOut::draw()
{
	if (mFrames != 0) {
		mAlpha += (mTarget - mAlpha) / (f32)mFrames;
		mFrames--;
	}
}

// 0x8011220C
void TBlackFadeInOut::drawTask(TBlackFadeInOut* fade)
{
	u32 alpha = (int)(255.0f * fade->mAlpha) & 0xFF;
	fadeFillScreen(((alpha << 24) & 0xFF000000) | (alpha >> 8));
}

const char* gTCircleFadeInOutName = "TCircleFadeInOut";

// 0x80112290
TCircleFadeInOut::TCircleFadeInOut(TObject* parent, int close, f32 x, f32 y, f32 scaleX, f32 scaleY)
    : TObject(parent)
{
	mX      = x;
	mY      = y;
	mScaleX = scaleX;
	mScaleY = scaleY;
	_00     = (u32)gTCircleFadeInOutName;
	mSize   = 0x58;
	mTask   = addDrawTask(this, (void (*)(void*))drawTask, 0x3E, 0);
	mFrames = 0x3C;
	if (close == 0) {
		mRadius = 0.0f;
		mTarget = 640.0f;
	} else {
		mRadius = 640.0f;
		mTarget = 0.0f;
	}
}

// 0x80112390
TCircleFadeInOut::~TCircleFadeInOut()
{
	removeDrawTask(mTask);
}

// 0x801123F8
void TCircleFadeInOut::draw()
{
	if (mFrames != 0) {
		mRadius += (mTarget - mRadius) / (f32)mFrames;
		mFrames--;
	} else {
		mFlags |= 1;
	}
}

// 0x8011245C
// A strip of 50 points
void TCircleFadeInOut::drawTask(TCircleFadeInOut* fade)
{
	FadeVertex verts[0x32];
	f32 width;
	f32 height;
	f32 centerX;
	f32 centerY;
	f32 angle;
	int turn;
	int i;
	f32 sx  = fade->mScaleX;
	f32 sy  = fade->mScaleY;
	width   = 640.0f * sx;
	centerX = 320.0f * sx;
	height  = 480.0f * sy;
	centerY = 240.0f * sy;
	angle   = 0.0f;
	for (i = 0; i < 0x32; i++) {
		verts[i].mColor = 0xFF000000;
		if (i % 2 == 1) {
			if (angle >= 0.0f && angle < 45.0f) {
				verts[i].x = width;
				verts[i].y = verts[i - 1].y;
			} else if (angle == 45.0f) {
				verts[i].x = width;
				verts[i].y = height;
			} else if (angle > 45.0f && angle < 135.0f) {
				verts[i].x = verts[i - 1].x;
				verts[i].y = height;
			} else if (angle == 135.0f) {
				verts[i].x = 0.0f;
				verts[i].y = height;
			} else if (angle > 135.0f && angle < 225.0f) {
				verts[i].x = 0.0f;
				verts[i].y = verts[i - 1].y;
			} else if (angle == 225.0f) {
				verts[i].x = 0.0f;
				verts[i].y = 0.0f;
			} else if (angle > 225.0f && angle < 315.0f) {
				verts[i].x = verts[i - 1].x;
				verts[i].y = 0.0f;
			} else if (angle == 315.0f) {
				verts[i].x = width;
				verts[i].y = 0.0f;
			} else if (angle > 315.0f && angle <= 360.0f) {
				verts[i].x = width;
				verts[i].y = verts[i - 1].y;
			}
			verts[i].z = 1.0f;
			angle += 15.0f;
		} else {
			turn       = (int)(182.04445f * angle);
			verts[i].x = fade->mRadius * njCos(turn) + centerX;
			verts[i].y = fade->mRadius * njSin(turn) + centerY;
			if (verts[i].x < 0.0f) {
				verts[i].x = 0.0f;
			} else if (verts[i].x > width) {
				verts[i].x = width;
			}
			if (verts[i].y < 0.0f) {
				verts[i].y = 0.0f;
			} else if (verts[i].y > height) {
				verts[i].y = height;
			}
			verts[i].z = 1.0f;
		}
		verts[i].x += fade->mX;
		verts[i].y += fade->mY;
	}
	njDrawPolygon2DExStart(0);
	njDrawPolygon2DExSetData((const NJS_POLYGON_VTX*)verts, (s32)0x32);
	njDrawPolygon2DExEnd();
}

const char* gTAlphaFadeName = "TAlphaFade";

// 0x80112748
TAlphaFade::TAlphaFade(TObject* parent, int clearing)
    : TObject(parent)
{
	mClearing = clearing;
	_00       = (u32)gTAlphaFadeName;
	mSize     = 0x30;
	mTask     = addDrawTask(this, (void (*)(void*))drawTask, 0x3C, 0);
	if (mClearing == 0) {
		mAlpha = 0.0f;
	} else {
		mAlpha = 0.7f;
	}
}

// 0x801127D8
TAlphaFade::~TAlphaFade()
{
	sAlphaFadeObj = 0;
	removeDrawTask(mTask);
}

// 0x80112848
void TAlphaFade::draw()
{
	if (mClearing == 0) {
		mAlpha += 0.1f;
	} else {
		mAlpha -= 0.1f;
	}
	f32 alpha;
	if (mAlpha < 0.0f) {
		alpha = 0.0f;
	} else if (mAlpha > 0.7f) {
		alpha = 0.7f;
	} else {
		alpha = mAlpha;
	}
	mAlpha = alpha;
}

// 0x801128A8
void TAlphaFade::drawTask(TAlphaFade* fade)
{
	u32 alpha = (int)(255.0f * fade->mAlpha) & 0xFF;
	fadeFillScreen(((alpha << 24) & 0xFF000000) | (alpha >> 8));
}
