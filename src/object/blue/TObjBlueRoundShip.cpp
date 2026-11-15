#include "object/blue/TObjBlueRoundShip.h"
#include "engine/DrawTask.h"
#include "object/ObjArchive.h"
#include "object/TObjLauncher.h"
#include "object/ObjAction.h"
#include "egg/EggCreature.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "camera/TCameraCtrl.h"
#include "stage/SetSlot.h"
#include "engine/TParticle.h"
struct SetLink;

// 0x800E9CE4
// is the message window's loader so this file stops before it

extern "C" {

extern const int sRoundShipNodes[3] = {
	1,
	2,
	3,
}; // 1 2 3
}
const char* gTObjBlueRoundShipName = "TObjBlueRoundShip";
char* sRoundShipTypeNames[2]       = { "FIX", "PATH" };
extern "C" {
extern const MotionEntry sBlueRoundShipMotions[] = {
	{ MOTION_LOOP, 5, -1, 0, 0.0f, 30.0f, 0.0125f, 0.0f, 0 },
};
}

typedef char TObjBlueRoundShip_size_check[(sizeof(TObjBlueRoundShip) == 0x128) ? 1 : -1];

// 0x800E9620
extern "C" TObject* createTObjBlueRoundShip(TObject* parent, SetDesc* desc)
{
	return new TObjBlueRoundShip(parent, desc);
}

// 0x800E9674
// The set int is clamped to 0 or 1 then vfn_74 builds that kind
void TObjBlueRoundShip::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mType = CLAMP_RANGE(desc->getInt(0), 0, 1);
	vfn_74();
}

// 0x800E9724
void TObjBlueRoundShip::debugPrint(void*)
{
	njPrint(0xA0013, ":TYPE %s", sRoundShipTypeNames[mType]);
}

// 0x800E9768
TObjBlueRoundShip::TObjBlueRoundShip(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjBlueRoundShipName;
	mSize = 0x128;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	initFromDesc(desc);
}

// 0x800E9804
TObjBlueRoundShip::~TObjBlueRoundShip()
{
	removeDrawTask(mDraw);
}

// 0x800E9884
void TObjBlueRoundShip::vfn_74()
{
	if (mType == 0) {
		setupFixed();
	} else {
		setupPath();
	}
}

// 0x800E98B8
void TObjBlueRoundShip::setupFixed()
{
	_E0                 = 0;
	NJS_OBJECT** models = getGeObjModels();
	mModel.setModelNodes(models[0xE], 0x10);
	NJS_TEXLIST** tex = getGeObjTextures();
	mModel.mTexList   = tex[0xE];
	mModel.mState     = getDefaultDrawState();
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
	setLinkFree((SetLink*)&mLink);
}

// 0x800E9938
void TObjBlueRoundShip::setupPath()
{
	_E0            = 0;
	mAimYaw        = 0;
	RailPath* path = railFind(0);
	if (path != 0) {
		Vec3f dir;
		mRail.setPath(path);
		mRail.getDir(&dir);
		mAimYaw = vecAngleXZ(&dir);
		mRot.y  = mAimYaw;
	}
	ridePath();
	void** motions = gGeObjCommon->mSet.mMotions;
	mMotion.init((const MotionEntry*)sBlueRoundShipMotions, motions);
	mMotion.play(0);

	NJS_OBJECT** models = getGeObjModels();
	mModel.setModelNodes(models[0xE], 0x10);
	NJS_TEXLIST** tex = getGeObjTextures();
	mModel.mTexList   = tex[0xE];
	mModel.mState     = getDefaultDrawState();
	mModel.mMotion    = &mMotion;

	for (int i = 0; i < 3; i++) {
		mPart[i]                       = particleGenById(this, &gVecZero, 1, 0xE);
		ObjPlace* node                 = mModel.mNodes + sRoundShipNodes[i];
		*(f32**)((u8*)mPart[i] + 0x58) = node->mMatrix;
	}
	setLinkFree((SetLink*)&mLink);
}

// 0x800E9A70
void TObjBlueRoundShip::draw()
{
	if (objDebugEnabled(this) != 0) {
		return;
	}
	if (mType != 1) {
		return;
	}
	mMotion.update();
	mModel.calcMotion();
	ridePath();

	f32 mtx[12];
	mtxSetTransV(mtx, &mPos);
	mtxRotateY(mtx, mtx, mRot.y);
	mtxTranslate((float*)mModel.mNodes[sRoundShipNodes[0]].mMatrix, (const float*)mtx, 0.0f, 0.0f, 262.5f);
	mtxTranslate((float*)mModel.mNodes[sRoundShipNodes[1]].mMatrix, (const float*)mtx, 222.0f, 0.0f, -180.0f);
	mtxTranslate((float*)mModel.mNodes[sRoundShipNodes[2]].mMatrix, (const float*)mtx, -222.0f, 0.0f, -180.0f);
}

// 0x800E9B70
// Turns mRot.y toward the path by 4 then steps the rail by 0.2
void TObjBlueRoundShip::ridePath()
{
	Vec3f dir;
	mRail.getPos(&mPos);
	mRail.getDir(&dir);
	mAimYaw      = vecAngleXZ(&dir);
	int delta    = angleDist(mRot.y, mAimYaw);
	int absDelta = delta < 0 ? -delta : delta;
	if (absDelta < 4) {
		mRot.y = mAimYaw;
	} else if (delta > 0) {
		mRot.y -= 4;
	} else {
		mRot.y += 4;
	}
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
	mRail.advance(0.2f);
}

// 0x800E9C34
// Also publishes the result at sRoundShipVisible
void TObjBlueRoundShip::vfn_10()
{
	int visible       = cameraIsVisible(5000.0f, 1000.0f, &mPos, 0);
	sRoundShipVisible = visible;
	if (visible != 0) {
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x800E9CA0
void TObjBlueRoundShip::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	mModel.draw();
	njPopMatrixEx();
}
