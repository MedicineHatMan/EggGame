#include "object/TObjTitleHelios.h"
#include "engine/DrawTask.h"
#include "object/TObjLauncher.h"
#include "stl/math.h"
#include "engine/Color.h"
#include "engine/TFogCtrl.h"
#include "ui/TitleObjArc.h"
#include "engine/TLensFlare.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "core/MathUtil.h"
#include "camera/TCameraCtrl.h"
#include "core/VecMath.h"
#include "engine/Landscape.h"
#include "mode/TModeTitle.h"
struct CamView;
struct NJS_TEXLIST;

// Lens flare hung on the sun
struct HeliosFlare {
	u8 _00[4];
	u16 mFlags; // _04 bit 0 deletes it
	u8 _06[0x22];
	Vec3f mPos; // _28
	u8 mAlpha;  // _34
};

typedef char TObjTitleHelios_size_check[(sizeof(TObjTitleHelios) == 0x13C) ? 1 : -1];

// 0x801AD8F8
extern "C" TObjTitleHelios* createTObjTitleHelios(TObject* parent)
{
	return new TObjTitleHelios(parent);
}

// 0x801AD944
void TObjTitleHelios::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
}

const char* gTObjTitleHeliosName = "TObjTitleHelios";

// 0x801AD964
TObjTitleHelios::TObjTitleHelios(TObject* parent)
    : TSetActor(parent)
{
	_110  = 0;
	_130  = 0;
	_00   = (u32)gTObjTitleHeliosName;
	mSize = 0x13C;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mFlare = lensFlareCreate(this, &mPos);
	vfn_74();
}

// 0x801ADA14
TObjTitleHelios::~TObjTitleHelios()
{
	if (mFlare != 0) {
		((HeliosFlare*)mFlare)->mFlags |= 1;
		mFlare = 0;
	}
	removeDrawTask(mDraw);
}

// 0x801ADAB4
void TObjTitleHelios::vfn_74()
{
	mTimer = 0;
	setup();
}

// 0x801ADADC
void TObjTitleHelios::setup()
{
	RailPath* path = railFind(1);
	if (path != 0) {
		mPath.setPath(path);
		switch (gTitleScene) {
		case 0:
			mPath.advance(2000.0f);
			break;
		case 1:
			mPath.advance(4000.0f);
			break;
		case 2:
			mPath.advance(6000.0f);
			break;
		}
	}
	ride();
	mMotion.init(sTitleHeliosMotions, getTitleObjMotion(1));
	mMotion.play(0);
	NJS_OBJECT** models = getTitleObjModels();
	mModel.setModel(models[6]);
	void** textures = (void**)getTitleObjTexLists();
	mModel.mTexList = (NJS_TEXLIST*)textures[0];
	mModel.mState   = getDefaultDrawState();
	mModel.mMotion  = &mMotion;
}

// 0x801ADBD8
void TObjTitleHelios::draw()
{
	f32 mtx[12];
	if (gTitleScene == 3) {
		((HeliosFlare*)mFlare)->mAlpha = 0;
		return;
	}

	f32 sun = 1.0f - getLandscapeBlend();
	if (sun < 0.5f) {
		sun = 0.0f;
	}
	int alpha = (int)(255.0f * sun);
	setColorAlpha(&mColor, alpha);
	if (mFlare != 0) {
		((HeliosFlare*)mFlare)->mPos   = mPos;
		((HeliosFlare*)mFlare)->mAlpha = (u8)alpha;
	}
	mMotion.update();
	mModel.calcMotion();
	ride();
	mtxSetTransV(mtx, &mPos);
}

// 0x801ADCA8
void TObjTitleHelios::ride()
{
	mPath.getPos(&mPos);
	mtxSetTransV(mMatrix, &mPos);
	mPath.advance(1.1111112f);
}

// 0x801ADCF4
void TObjTitleHelios::setPace(int pace)
{
	mPath.advance(1.1111112f * (f32)pace);
}

// 0x801ADD3C
void TObjTitleHelios::vfn_10()
{
	switch (gTitleScene) {
	case 4:
		if (++mTimer >= 600) {
			break;
		}
	case 3:
		mDraw->mFlags |= 1;
		return;
	}

	if (isInRange((void*)this, (const Vec3f*)&mPos, 100000.0f) != 0) {
		Vec3f dir;
		const Vec3f* view = (const Vec3f*)camGetDrawView();
		vecSub(&dir, view, &mPos);
		vecNormalize(&dir, &dir);
		mtxSetTransV(mMatrix, &mPos);
		mtxRotateY(mMatrix, mMatrix, vecAngleXZ(&dir));
		int pitch = (int)(10430.38043493439 * (f64)(f32)atan(-(f64)dir.y));
		mtxRotateX(mMatrix, mMatrix, pitch);
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x801ADE60
void TObjTitleHelios::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	njScale(0, 20.0f, 20.0f, 20.0f);
	fogOff();
	mModel.draw();
	fogApply();
	njPopMatrixEx();
}
