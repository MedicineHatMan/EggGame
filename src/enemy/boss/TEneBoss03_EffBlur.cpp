#include "enemy/boss/EneBoss.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "enemy/boss/TEneBoss03_EffBlur.h"
#include "engine/Color.h"
#include "engine/FrameTex.h"
#include "Ninja/ninja.h"
struct NJS_QUAD_TEXTURE;

typedef char TEneBoss03_EffBlur_size_check[(sizeof(TEneBoss03_EffBlur) == 0x3C) ? 1 : -1];

// 0x800C4488
TEneBoss03_EffBlur* createTEneBoss03_EffBlur(TObject* parent, const Vec3f* pos)
{
	return new TEneBoss03_EffBlur(parent, pos);
}

const char* gTEneBoss03_EffBlurName = "TEneBoss03_EffBlur";

// 0x800C44DC
TEneBoss03_EffBlur::TEneBoss03_EffBlur(TObject* parent, const Vec3f* pos)
    : TObject(parent)
{
	_00       = (u32)gTEneBoss03_EffBlurName;
	mSize     = 0x3C;
	mPos      = *pos;
	mColor    = 0xFFFFFF00;
	mEnding   = 0;
	mScale    = 1.0f;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3B, 0);
}

// 0x800C4578
TEneBoss03_EffBlur::~TEneBoss03_EffBlur()
{
	removeDrawTask(mDrawTask);
}

// 0x800C45E0
void TEneBoss03_EffBlur::setPos(const Vec3f* pos)
{
	mPos = *pos;
}

// 0x800C45FC
void TEneBoss03_EffBlur::end()
{
	mEnding |= 1;
}

// 0x800C460C
void TEneBoss03_EffBlur::draw()
{
	if ((mEnding & 1) != 0) {
		addColorAlpha(&mColor, -2);
		mScale -= 0.001f;
		if (mScale < 1.0f) {
			mScale = 1.0f;
		}
	} else {
		u32 alpha = (mColor & 0xFF) + 8;
		if (alpha > 0xC0) {
			alpha = 0xC0;
		}
		setColorAlpha(&mColor, alpha & 0xFF);
		mScale += 0.001f;
		if (mScale > 1.08f) {
			mScale = 1.08f;
		}
	}
}

// 0x800C46BC
void TEneBoss03_EffBlur::vfn_10()
{
}

// 0x800C46C0
void TEneBoss03_EffBlur::render()
{
	if ((mColor & 0xFF) != 0) {
		Vec3f at;
		njCalcPointEx(&mPos, &at);
		if (at.z < 0.0f) {
			struct {
				f32 mX0, mY0, mX1, mY1;
				f32 mU0, mV0, mU1, mV1;
			} quad;
			f32 persp = -1.0f / at.z;
			persp *= *(f32*)((u8*)&gProjection + 0x50);
			f32 zoom = 1.0f / mScale;
			f32 u    = 0.5f + (at.x * persp) / 640.0f;
			f32 v    = 0.5f + -(at.y * persp) / 320.0f;
			quad.mX0 = 0.0f;
			quad.mY0 = 0.0f;
			quad.mX1 = 640.0f;
			quad.mY1 = 480.0f;
			quad.mU0 = -u * zoom + u;
			quad.mU1 = (1.0f - u) * zoom + u;
			quad.mV0 = -v * zoom + v;
			quad.mV1 = (1.0f - v) * zoom + v;
			frameTexSetCur();
			blendModeAlpha();
			njQuadTextureStart(1);
			njSetQuadTexture(0, (mColor << 24) | (mColor >> 8));
			GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
			njDrawQuadTexture((const NJS_QUAD_TEXTURE*)&quad, 0.8f);
		}
	}
}
