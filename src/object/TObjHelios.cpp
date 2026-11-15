#include "stage/Stage.h"
#include "engine/ModelDraw.h"
#include "engine/DrawTask.h"
#include "object/ObjArchive.h"
#include "engine/TLensFlare.h"
#include "object/ObjAction.h"
#include "egg/EggCreature.h"
#include "animal/AniMount.h"
#include "engine/Color.h"
#include "engine/TFogCtrl.h"
#include "enemy/boss/TEneLastExBoss.h"
#include "engine/Landscape.h"
#include "stl/math.h"
#include "engine/FxSystem.h"
#include "camera/TCameraCtrl.h"
#include "core/MathUtil.h"
#include "core/VecMath.h"
#include "object/TObjHelios.h"
#include "enemy/boss/TEneLastExBossFeather.h"
#include "core/TMainTask.h"
struct Fx;
struct DrawState;
struct CamView;

extern "C" {
}

typedef char TObjHelios_size_check[(sizeof(TObjHelios) == 0xC4) ? 1 : -1];

// 0x80189CD0
// The set init pair both empty
extern "C" void initTObjHelios()
{
}

// 0x80189CD4
extern "C" void termTObjHelios()
{
}

// 0x80189CD8
// The "sol" event command a helios at the three floats it is given
extern "C" int eventSol(f32* args)
{
	Vec3f pos;
	pos.set(args[0], args[1], args[2]);
	createTObjHelios(gTL_02, &pos);
	return 1;
}

// 0x80189D1C
extern "C" TObjHelios* createTObjHelios(TObject* parent, const Vec3f* pos)
{
	return new TObjHelios(parent, pos);
}

const char* gTObjHeliosName = "TObjHelios";

// 0x80189D70
TObjHelios::TObjHelios(TObject* parent, const Vec3f* pos)
    : TObject(parent)
    , mFlags(0)
    , mZero(0)
    , mPos(*pos)
{
	_00    = (u32)gTObjHeliosName;
	mSize  = 0xC4;
	mFlags = 0x200;
	mColor = 0xFF;
	mTrack.set(gStgObjCommon->mSet.mMotions[12], 1, 0.2f);
	mModel.setModel(getStgObjModels()[51]);
	mModel.mDrawState = (DrawState*)&mFlags;
	mModel.mTexList   = getStgObjTextures()[43];
	mModel.mMatrix    = mMtx;
	mModel.mTrack     = &mTrack;
	mDraw             = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	mEffect           = 0;
	mFlare            = lensFlareCreate(this, &mPos);
}

// 0x80189E8C
TObjHelios::~TObjHelios()
{
	fxStop((Fx*)mEffect);
	removeDrawTask(mDraw);
}

// 0x80189F14
void TObjHelios::draw()
{
	f32 fade = 1.0f - getLandscapeBlend();
	if (fade < 0.5f) {
		fade = 0.0f;
	}
	int alpha = (int)(255.0f * fade);
	setColorAlpha(&mColor, alpha);
	if (mFlare != 0) {
		mFlare->mAlpha = alpha;
	}
	mTrack.update();
}

// 0x80189F90
void TObjHelios::vfn_10()
{
	Vec3f view;
	if (cameraIsVisible(100000.0f, 500.0f, &mPos, &view) != 0) {
		mDraw->mSortKey = drawTaskDepthKey(view.z);
		mDraw->mFlags &= ~1;
		Vec3f dir;
		Vec3f* eye = (Vec3f*)camGetDrawView();
		vecSub(&dir, eye, &mPos);
		vecNormalize(&dir, &dir);
		mtxSetTransV(mMtx, &mPos);
		mtxRotateY(mMtx, mMtx, vecAngleXZ(&dir));
		mtxRotateX(mMtx, mMtx, (int)(10430.38043493439 * (f32)atan(-dir.y)));
		mtxScale(10.0f, 10.0f, 10.0f, mMtx, mMtx);
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x8018A098
void TObjHelios::render()
{
	fogOff();
	mModel.draw();
	fogApply();
}

// 0x8018A0D0
// From the last ex boss's load
extern "C" void lastExBossFeatherInit()
{
	gLastExBossFeatherObject  = modelTEneLastExBoss(0);
	gLastExBossFeatherTexList = textureTEneLastExBoss();
	gLastExBossFeatherMotion  = motionTEneLastExBoss(3);
}

// 0x8018A10C
extern "C" void lastExBossFeatherExit()
{
	gLastExBossFeatherObject  = 0;
	gLastExBossFeatherTexList = 0;
	gLastExBossFeatherMotion  = 0;
}
