#include "enemy/boss/EneBoss.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "object/ObjAction.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "engine/FrameTex.h"
#include "engine/TFrameBlur.h"
#include "core/TMainTask.h"

extern "C" {
TexVtx sFrameBlurQuad[4] = {
	{ 0.0f, 0.0f, 0.9f, 0.0f, 0.0f, 0xC0FFFFFF },
	{ 0.0f, 480.0f, 0.9f, 0.0f, 1.0f, 0xC0FFFFFF },
	{ 640.0f, 0.0f, 0.9f, 1.0f, 0.0f, 0xC0FFFFFF },
	{ 640.0f, 480.0f, 0.9f, 1.0f, 1.0f, 0xC0FFFFFF },
};
}

TFrameBlur* sFrameBlurObj; // 0x802C4D48
int sFrameBlurOn;          // 0x802C4D4C

typedef char TFrameBlur_size_check[(sizeof(TFrameBlur) == 0x30) ? 1 : -1];

// 0x8002E748
extern "C" void frameBlurInit()
{
	if (frameTexIsOn() != 0) {
		frameTexCreate(1, 0x280, 0x1E0, 4);
		sFrameBlurOn = 1;
	}
}

// 0x8002E78C
extern "C" void frameBlurReset()
{
	sFrameBlurObj = 0;
	sFrameBlurOn = 0;
}

// 0x8002E79C
extern "C" void frameBlurCreate() { sFrameBlurObj = new TFrameBlur(gTL_07); }

// 0x8002E7DC
extern "C" void frameBlurStart(int frames, f32 alpha)
{
	if (sFrameBlurObj != 0) {
		sFrameBlurObj->start(frames, alpha);
	}
}

const char* gTFrameBlurName = "TFrameBlur";

// 0x8002E810
TFrameBlur::TFrameBlur(TObject* parent)
    : TObject(parent)
{
	_00       = (u32)gTFrameBlurName;
	mSize     = 0x30;
	mAlpha    = 0.0f;
	mFrames   = 0;
	mCaptured = 0;
	mTask     = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3B, 0);
}

// 0x8002E88C
TFrameBlur::~TFrameBlur() { removeDrawTask(mTask); }

// 0x8002E8F4
void TFrameBlur::start(int frames, f32 alpha)
{
	mFrames   = frames;
	mAlpha    = alpha;
	mCaptured = 0;
}

// 0x8002E908
void TFrameBlur::draw()
{
	if (mFrames != 0) {
		mFrames--;
		return;
	}
	mAlpha = 0.0f;
}

// 0x8002E92C
void TFrameBlur::render()
{
	if (mFrames == 0) {
		return;
	}
	if (mCaptured != 0) {
		u32 flags = _nj_current_ctx_->blend;
		_nj_current_ctx_->blend = flags | 0x10000 | 0x8000;
		frameTexSet(1);
		njSetTextureNum(0);
		u32 color = ((u32)(255.0f * mAlpha) << 24) | 0xFF0000 | 0xFFFF;
		sFrameBlurQuad[0].col = color;
		sFrameBlurQuad[1].col = color;
		sFrameBlurQuad[2].col = color;
		sFrameBlurQuad[3].col = color;
		pushCullMode(0);
		njDrawTexture2DExStart(1);
		njDrawTexture2DExSetData((const NJS_TEXTURE_VTX*)sFrameBlurQuad, 4);
		njDrawTexture2DExEnd();
		popCullMode();
		_nj_current_ctx_->blend = flags;
	}
	frameTexRequest(1);
	frameTexUpdate(1);
	mCaptured = 1;
}
