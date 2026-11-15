#include "object/TObjTitleChiken.h"
#include "engine/DrawTask.h"
#include "object/TObjLauncher.h"
#include "object/ObjAction.h"
#include "object/ObjShadow.h"
#include "ui/TitleObjArc.h"
#include "animal/AniMount.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "core/MathUtil.h"
#include "mode/TModeTitle.h"
struct NJS_TEXLIST;

typedef char TObjTitleChiken_size_check[(sizeof(TObjTitleChiken) == 0x158) ? 1 : -1];

const char* gTObjTitleChikenName = "TObjTitleChiken";

const MotionEntry sTitleChikenMotions[] = {
	{ 0, 4, -1, 0, 0.0f, 20.0f, 0.6f, 0.0f, 0 },
	{ 0, 5, -1, 0, 0.0f, 20.0f, 0.8f, 0.0f, 0 },
};

// 0x801AC83C
extern "C" TObject* createTObjTitleChiken(TObject* parent, SetDesc* desc)
{
	return new TObjTitleChiken(parent, desc);
}

// 0x801AC890
void TObjTitleChiken::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mScale[0] = 1.0f + CLAMP_RANGE(desc->getFloat(0), -1.0f, 10.0f);
	mScale[1] = 1.0f + CLAMP_RANGE(desc->getFloat(1), -1.0f, 10.0f);
}

// 0x801AC9B0
void TObjTitleChiken::debugPrint(void*)
{
	njPrint(0xA0017, ": CHIKEN SCALE(+1.0f)");
	njPrint(0xA0018, ": CROW SCALE(+1.0f)");
}

// 0x801AC9FC
TObjTitleChiken::TObjTitleChiken(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjTitleChikenName;
	mSize = 0x158;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	initFromDesc(desc);
	vfn_74();
}

// 0x801ACAB4
TObjTitleChiken::~TObjTitleChiken()
{
	removeDrawTask(mDraw);
}

// 0x801ACB40
void TObjTitleChiken::vfn_74()
{
	setup();
}

// 0x801ACB60
void TObjTitleChiken::setup()
{
	mYaw           = 0;
	RailPath* path = railFind(0);
	if (path != 0) {
		Vec3f dir;
		mPath.setPath(path);
		mPath.getDir(&dir);
		mYaw   = vecAngleXZ(&dir);
		mRot.y = mYaw;
	}
	facePath();
	mMotion.init(sTitleChikenMotions, getTitleObjMotion(1));
	shadowSetGround(&mShadow, &mPos, 0);
	shadowEnable(&mShadow);
	shadowVolumesDisable();

	if (gTitleScene == 3) {
		mMotion.play(1);
		mModel.setModel(getTitleObjModels()[5]);
		mCrow       = 1;
		mShadow._30 = 7.0f;
		mShadow._34 = 7.0f;
	} else {
		mMotion.play(0);
		mModel.setModel(getTitleObjModels()[0]);
		mCrow       = 0;
		mShadow._30 = 10.0f;
		mShadow._34 = 10.0f;
	}
	void** textures = (void**)getTitleObjTexLists();
	mModel.mTexList = (NJS_TEXLIST*)textures[0];
	mModel.mState   = getDefaultDrawState();
	mModel.mMotion  = &mMotion;
}

// 0x801ACCA0
void TObjTitleChiken::draw()
{
	mMotion.update();
	f32 mtx[12];
	mModel.calcMotion();
	facePath();
	mtxSetTransV(mtx, &mPos);
	mtxRotateY(mtx, mtx, mRot.y);
	shadowSetGround(&mShadow, &mPos, 0);
}

// 0x801ACD0C
void TObjTitleChiken::facePath()
{
	Vec3f dir;
	mPath.getPos(&mPos);
	mPath.getDir(&dir);
	mYaw = vecAngleXZ(&dir);
	angleDist(mRot.y, mYaw);
	mRot.y = mYaw;
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
	mPath.advance(0.2f);
}

// 0x801ACD94
void TObjTitleChiken::vfn_10()
{
	if (isInRange((void*)this, (const Vec3f*)&mPos, 2000.0f) != 0) {
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x801ACDF4
void TObjTitleChiken::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	njScale(0, mScale[mCrow], mScale[mCrow], mScale[mCrow]);
	mModel.draw();
	njPopMatrixEx();
}
