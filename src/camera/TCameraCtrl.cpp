#include "stage/Stage.h"
#include "camera/CamArea.h"
#include "core/Pad.h"
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "camera/TCamZelda5.h"
#include "camera/TCamLockOnCursor.h"
#include "camera/TCameraCtrl.h"
#include "camera/CamPad.h"
#include "stl/math.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "core/TMainTask.h"
#include "animal/RacerMover.h"

struct CamPath;
struct CamSaved;

const char* gTCameraCtrlName = "TCameraCtrl";

// 0x80006114
TCameraCtrl::TCameraCtrl()
    : TObject(gTL_01)
{
	// not matching Need to find start of vtable
	_00   = (u32)gTCameraCtrlName;
	mSize = 0x3EC;
	for (int i = 0; i < 4; i++) {
		mView[i].init();
		mCamera[i] = 0;
		mLock[i]   = new TCamLockOnCursor(gTL_06);
		camAimClear(&mAim[i]);
		camPadInit(&mPath[i]);
		mPad[i]   = i;
		mEditor[i] = 0;
		camSavedClear(&mSaved[i]);
	}
	mAreas     = createCamAreaCtrl();
	mDrawIndex = 0;
	mMode      = 0;
}

// 0x80006244
CamView::CamView() { }

// 0x80006248
TCameraCtrl::~TCameraCtrl()
{
	if (mAreas != 0) {
		delete mAreas;
		mAreas = 0;
	}
	for (int i = 3; i >= 0; i--) {
		if (mLock[i] != 0) {
			mLock[i]->mFlags |= 1;
			mLock[i] = 0;
		}
		if (mCamera[i] != 0) {
			mCamera[i]->mFlags |= 1;
			mCamera[i] = 0;
		}
	}
	if (gCameraCtrl == this) {
		gCameraCtrl = 0;
	}
}

// 0x8000632C
void TCameraCtrl::draw()
{
	for (int i = 0; i < 4; i++) {
		int pad      = mPad[i];
		CamPath* cam; // declared here for its register r27
		u16 right    = (u16)padGet(i)->mTriggerR;
		u16 left     = padGet(i)->mTriggerL;
		s16 x        = padGet(i)->mSubX;
		s16 y        = padGet(i)->mSubY;
		cam = &mPath[pad];
		camPadClear(cam);
		int downB = padGet(pad)->mHold & 0x20;
		camPadUpdate(cam, padGet(pad)->mHold & 0x40, left, downB, right, x, y);
	}
	if (mAreas != 0) {
		mAreas->update();
	}
	for (int j = 0; j < 4; j++) {
		camAimUpdate(&mAim[j]);
	}
}

// 0x80006428
extern "C" int camIndexValid(int i)
{
	return (u8)(i >= 0 && i < 4);
}

// 0x80006448
extern "C" int cameraCtrlExists()
{
	return (u8)(gCameraCtrl != 0);
}

// 0x8000645C
// picks the camera the scene is drawn from
extern "C" void camSetDrawView(int i)
{
	if (gCameraCtrl != 0) {
		gCameraCtrl->setView(i);
	}
}

// 0x80006490
void TCameraCtrl::setView(int i)
{
	if (camIndexValid(i)) {
		mDrawIndex = i;
		mView[i].apply();
		GXInvalidateVtxCache();
	}
}

// 0x800064E4
extern "C" void camAddLink(int player, CamLink* link)
{
	if (cameraCtrlExists() && camIndexValid(player)) {
		camAimAdd(&gCameraCtrl->mAim[player], link);
	}
}

// 0x80006544
extern "C" void camRemoveLink(int player, CamLink* link)
{
	if (cameraCtrlExists() && camIndexValid(player)) {
		camAimRemove(&gCameraCtrl->mAim[player], link);
	}
}

// 0x800065A4
extern "C" void camLoadAreas(const char* name)
{
	loadCamAreas(name);
}

// 0x800065C4
extern "C" void camSetTarget(int player, TObjAction* obj)
{
	if (cameraCtrlExists()) {
		gCameraCtrl->setTarget(player, obj);
	}
}

// 0x8000660C
void TCameraCtrl::setTarget(int i, TObjAction* obj)
{
	if (camIndexValid(i)) {
		u16 id;
		if (obj == 0) {
			id = 0xFFFF;
		} else {
			id = obj->mObjId;
		}
		TCamera* camera = getCamera(i);
		if (camera != 0) {
			camera->setTarget(id);
		}
		if (mLock[i] != 0) {
			mLock[i]->setOwner(id);
		}
	}
}

// 0x800066A8
extern "C" u16 camGetTarget(int player)
{
	if (cameraCtrlExists()) {
		return gCameraCtrl->getTarget(player);
	}
	return 0xFFFF;
}

// 0x800066F4
u16 TCameraCtrl::getTarget(int i)
{
	TCamera* camera = getCamera(i);
	if (camera != 0) {
		return camera->getTarget();
	}
	return 0xFFFF;
}

// 0x8000672C
extern "C" void camSetCurveLift(f32 value, int player)
{
	TCamera* camera = camGetCamera(player);
	if (camera != 0) {
		((TCamZelda5*)camera)->setCurveLift(value);
	}
}

// 0x80006768
extern "C" void camResetCurveLift(int player)
{
	TCamera* camera = camGetCamera(player);
	if (camera != 0) {
		((TCamZelda5*)camera)->resetCurveLift();
	}
}

// 0x80006794
extern "C" void camSetChaseOffset(f32 a, f32 b, int player)
{
	TCamera* camera = camGetCamera(player);
	if (camera != 0) {
		((TCamZelda5*)camera)->setChaseOffset(a, b);
	}
}

// 0x800067E0
extern "C" void camResetChaseOffset(int player)
{
	TCamera* camera = camGetCamera(player);
	if (camera != 0) {
		((TCamZelda5*)camera)->resetChaseOffset();
	}
}

// 0x8000680C
extern "C" void camSetChasePitch(int player, int value)
{
	TCamera* camera = camGetCamera(player);
	if (camera != 0) {
		((TCamZelda5*)camera)->setChasePitch(value);
	}
}

// 0x80006848
extern "C" void camResetChasePitch(int player)
{
	TCamera* camera = camGetCamera(player);
	if (camera != 0) {
		((TCamZelda5*)camera)->resetChasePitch();
	}
}

// 0x80006874
extern "C" void camSetChaseBase(int player, f32 nearZ, f32 farZ)
{
	TCamera* camera = camGetCamera(player);
	if (camera != 0) {
		((TCamZelda5*)camera)->setChaseBase(nearZ, farZ);
	}
}

// 0x800068C0
extern "C" void camSetFar(f32 far, int player)
{
	TCamera* camera = camGetCamera(player);
	if (camera != 0) {
		((TCamera*)camera)->setBlendStep(far);
	}
}

// 0x800068FC
extern "C" void camSetLookFrom(int player, const Vec3f* pos)
{
	TCamera* camera = camGetCamera(player);
	if (camera != 0) {
		((TCamZelda5*)camera)->setLookFrom(pos);
	}
}

// 0x80006938
extern "C" CamSaved* camGetSaved(int player)
{
	if (cameraCtrlExists() && camIndexValid(player)) {
		return &gCameraCtrl->mSaved[player];
	}
	return 0;
}

// 0x80006994
extern "C" void camSave(int player, f32 a, f32 b, f32 c)
{
	CamSaved* saved = camGetSaved(player);
	if (saved != 0) {
		saved->mSet = 1;
		saved->mA   = a;
		saved->mB   = b;
		saved->mC   = c;
	}
}

// 0x800069EC
extern "C" void camClearSaved(int player)
{
	CamSaved* saved = camGetSaved(player);
	if (saved != 0) {
		camSavedClear(saved);
	}
}

// 0x80006A18
extern "C" void camRestoreSaved(int player)
{
	CamSaved* saved = camGetSaved(player);
	if (saved != 0 && saved->mSet != 0) {
		camSetCurveLift(saved->mA, player);
		camSetChaseOffset(saved->mB, saved->mC, player);
	}
}

// 0x80006A74
extern "C" void camUndoSaved(int player)
{
	CamSaved* saved = camGetSaved(player);
	if (saved != 0 && saved->mSet != 0) {
		camResetCurveLift(player);
		camResetChaseOffset(player);
	}
}

// 0x80006AC4
void TCameraCtrl::getPos(int i, Vec3f* out)
{
	if (camIndexValid(i)) {
		out->x = mView[i].mPos.x;
		out->y = mView[i].mPos.y;
		out->z = mView[i].mPos.z;
	}
}

// 0x80006B24
extern "C" void camGetPos(int player, Vec3f* out)
{
	if (cameraCtrlExists()) {
		gCameraCtrl->getPos(player, out);
	}
}

// 0x80006B6C
// where the camera being drawn from is
extern "C" void camGetDrawPos(Vec3f* out)
{
	if (cameraCtrlExists()) {
		gCameraCtrl->getPos(camGetDrawIndex(), out);
	}
}

// 0x80006BB4
void TCameraCtrl::getFlat(int i, Vec3f* out)
{
	if (camIndexValid(i)) {
		out->x = mView[i].mFlat.x;
		out->y = mView[i].mFlat.y;
		out->z = mView[i].mFlat.z;
	}
}

// 0x80006C14
extern "C" void camGetFlat(int player, Vec3f* out)
{
	if (cameraCtrlExists()) {
		gCameraCtrl->getFlat(player, out);
	}
}

// 0x80006C5C
// the way the camera being drawn from faces along the ground
extern "C" void camGetDrawFlat(Vec3f* out)
{
	if (cameraCtrlExists()) {
		gCameraCtrl->getFlat(camGetDrawIndex(), out);
	}
}

// 0x80006CA4
extern "C" int camGetYaw(int player)
{
	if (cameraCtrlExists()) {
		return gCameraCtrl->getYaw(player);
	}
	return 0;
}

// 0x80006CEC
extern "C" int camGetDrawYaw()
{
	return camGetYaw(camGetDrawIndex());
}

// 0x80006D10
int TCameraCtrl::getYaw(int i)
{
	if (camIndexValid(i)) {
		return mView[i].mYaw;
	}
	return 0;
}

// 0x80006D60
extern "C" void camGetAt(int player, Vec3f* out)
{
	if (cameraCtrlExists()) {
		gCameraCtrl->getAt(player, out);
	}
}

// 0x80006DA8
void TCameraCtrl::getAt(int i, Vec3f* out)
{
	if (camIndexValid(i)) {
		out->x = mView[i].mTarget.x;
		out->y = mView[i].mTarget.y;
		out->z = mView[i].mTarget.z;
	}
}

// 0x80006E08
extern "C" CamView* camGetView(int player)
{
	if (cameraCtrlExists()) {
		return gCameraCtrl->getView(player);
	}
	return 0;
}

// 0x80006E50
CamView* TCameraCtrl::getView(int i)
{
	if (camIndexValid(i)) {
		return &mView[i];
	}
	return 0;
}

// 0x80006EA0
extern "C" CamView* camGetDrawView()
{
	if (cameraCtrlExists()) {
		return gCameraCtrl->getDrawView();
	}
	return 0;
}

// 0x80006ED8
CamView* TCameraCtrl::getDrawView()
{
	return getView(mDrawIndex);
}

// 0x80006EFC
extern "C" TCamera* camGetCamera(int player)
{
	if (cameraCtrlExists()) {
		return gCameraCtrl->getCamera(player);
	}
	return 0;
}

// 0x80006F44
TCamera* TCameraCtrl::getCamera(int i)
{
	if (camIndexValid(i)) {
		return mCamera[i];
	}
	return 0;
}

// 0x80006F94
f32* TCameraCtrl::getMtx(int i)
{
	return mView[i].mMtx;
}

// 0x80006FA8
extern "C" f32* camGetDrawMtx()
{
	if (cameraCtrlExists()) {
		return gCameraCtrl->getDrawMtx();
	}
	return 0;
}

// 0x80006FE0
f32* TCameraCtrl::getDrawMtx()
{
	return getMtx(mDrawIndex);
}

// 0x80007004
int TCameraCtrl::getRoll(int i)
{
	return mView[i].mRoll;
}

// 0x80007014
extern "C" int camGetDrawRoll()
{
	if (cameraCtrlExists()) {
		return gCameraCtrl->getDrawRoll();
	}
	return 0;
}

// 0x8000704C
int TCameraCtrl::getDrawRoll()
{
	return getRoll(mDrawIndex);
}

// 0x80007070
// Whether a sphere is in view of the camera being drawn from
extern "C" int cameraIsVisible(f32 far, f32 radius, const Vec3f* pos, Vec3f* out)
{
	if (cameraCtrlExists()) {
		return gCameraCtrl->isVisible(pos, out, far, radius);
	}
	return 0;
}

// 0x800070E0
// The sphere must be within far along the view's depth and within the view's slopes across it
int TCameraCtrl::isVisible(const Vec3f* pos, Vec3f* out, f32 far, f32 radius)
{
	Vec3f view;
	njCalcPointEx(pos, &view);
	if (out != 0) {
		*out = view;
	}

	f64 abs   = __fabs(view.z);
	f32 depth = abs;
	if (depth > far) {
		return 0;
	}
	f32 scale  = 1.0f / depth;
	f32 slope  = mView[mDrawIndex].mSlopeX;
	f32 dx     = (f32)__fabs(view.x) - radius;
	if (dx * scale > slope) {
		return 0;
	}
	f32 dy = (f32)__fabs(view.y) - radius;
	if (mView[mDrawIndex].mAspect * (dy * scale) > slope) {
		return 0;
	}
	return 1;
}

// 0x800071E8
extern "C" int camGetDrawIndex()
{
	if (cameraCtrlExists()) {
		return gCameraCtrl->getDrawIndex();
	}
	return 0;
}

// 0x80007220
int TCameraCtrl::getDrawIndex()
{
	return mDrawIndex;
}

// 0x80007228
extern "C" int camGetPad(int player)
{
	if (cameraCtrlExists()) {
		return gCameraCtrl->getPad(player);
	}
	return 0;
}

// 0x80007270
int TCameraCtrl::getPad(int i)
{
	if (camIndexValid(i)) {
		return mPad[i];
	}
	return 0;
}

// 0x800072C0
extern "C" CamPath* camGetPath(int player)
{
	if (cameraCtrlExists()) {
		return gCameraCtrl->getPath(player);
	}
	return 0;
}

// 0x80007308
CamPath* TCameraCtrl::getPath(int i)
{
	if (camIndexValid(i)) {
		return &mPath[i];
	}
	return 0;
}

// 0x80007358
extern "C" void camResetCamera(int player)
{
	if (cameraCtrlExists()) {
		gCameraCtrl->resetCamera(player);
	}
}

// 0x80007398
void TCameraCtrl::resetCamera(int i)
{
	TCamera* camera = getCamera(i);
	if (camera != 0) {
		camera->vfn_30();
	}
}

// 0x800073D0
extern "C" void camShake(int player, f32 power)
{
	if (cameraCtrlExists()) {
		gCameraCtrl->shake(player, power);
	}
}

// 0x80007420
void TCameraCtrl::shake(int i, f32 power)
{
	TCamera* camera = getCamera(i);
	if (camera != 0) {
		camera->shake(power);
	}
}

// 0x80007468
extern "C" void camShakeHard(int player)
{
	camShake(player, 5.0f);
}

// 0x8000748C
extern "C" void camShakeNear(f32 range, f32 power, const Vec3f* pos)
{
	if (cameraCtrlExists()) {
		gCameraCtrl->shakeNear(pos, range, power);
	}
}

// 0x800074EC
// shakes every camera whose eye is within range of pos
void TCameraCtrl::shakeNear(const Vec3f* pos, f32 range, f32 power)
{
	TCamera* camera;
	int i;

	for (i = 0; i < 4; i++) {
		camera = mCamera[i];
		if (camera != 0) {
			Vec3f eye;
			camera->getEye(&eye);
			njSubVector(&eye, pos);
			if (njScalor(&eye) < range) {
				camera->shake(power);
			}
		}
	}
}

// 0x800075A4
extern "C" void camShakeNearSoft(f32 range, const Vec3f* pos)
{
	camShakeNear(range, 1.2f, pos);
}

// 0x800075C8
extern "C" void camShakeNearHard(f32 range, const Vec3f* pos)
{
	camShakeNear(range, 5.0f, pos);
}

// 0x800075EC
extern "C" void camSetModeOne()
{
	if (cameraCtrlExists()) {
		gCameraCtrl->setMode(1);
	}
}

// 0x80007620
void TCameraCtrl::setMode(int mode)
{
	int i;

	mMode = mode;
	if (mMode == 0) {
		for (i = 0; i < 4; i++) {
			if (mLock[i] != 0) {
				mLock[i]->useNormalRange();
			}
		}
	} else if (mMode == 1) {
		for (i = 0; i < 4; i++) {
			if (mLock[i] != 0) {
				mLock[i]->useAltRange();
			}
		}
	}
	for (i = 0; i < 4; i++) {
		if (mCamera[i] != 0) {
			mCamera[i]->setMode(mMode);
		}
	}
}

// 0x800076F8
extern "C" int camLockA(int player)
{
	if (cameraCtrlExists()) {
		return gCameraCtrl->lockA(player);
	}
	return 0;
}

// 0x80007740
int TCameraCtrl::lockA(int i)
{
	if (camIndexValid(i)) {
		return mLock[i]->hasMarks();
	}
	return 0;
}

// 0x80007794
extern "C" void camLockB(int player)
{
	if (cameraCtrlExists()) {
		gCameraCtrl->lockB(player);
	}
}

// 0x800077D4
void TCameraCtrl::lockB(int i)
{
	if (camIndexValid(i)) {
		mLock[i]->lock();
	}
}

// 0x80007820
extern "C" void camLockC(int player)
{
	if (cameraCtrlExists()) {
		gCameraCtrl->lockC(player);
	}
}

// 0x80007860
void TCameraCtrl::lockC(int i)
{
	if (camIndexValid(i)) {
		mLock[i]->unlock();
	}
}

// 0x800078AC
extern "C" void camLockD(int player)
{
	if (cameraCtrlExists()) {
		gCameraCtrl->lockD(player);
	}
}

// 0x800078EC
void TCameraCtrl::lockD(int i)
{
	if (camIndexValid(i)) {
		mLock[i]->nextMark();
	}
}

// 0x80007938
extern "C" u16 camLockGetId(int player)
{
	if (cameraCtrlExists()) {
		return gCameraCtrl->lockGetId(player);
	}
	return 0xFFFF;
}

// 0x80007984
u16 TCameraCtrl::lockGetId(int i)
{
	if (camIndexValid(i)) {
		return mLock[i]->getLockId();
	}
	return 0xFFFF;
}

// 0x800079DC
extern "C" void camLockGetTarget(int player, void* out)
{
	if (cameraCtrlExists() && camIndexValid(player)) {
		gCameraCtrl->mLock[player]->getCurrent((LockOnMark*)out);
	}
}

// 0x80007A3C
extern "C" void camLockE(int player)
{
	if (cameraCtrlExists()) {
		gCameraCtrl->lockE(player);
	}
}

// 0x80007A7C
void TCameraCtrl::lockE(int i)
{
	if (camIndexValid(i)) {
		mLock[i]->enable();
	}
}

// 0x80007AC8
extern "C" void camLockF(int player)
{
	if (cameraCtrlExists()) {
		gCameraCtrl->lockF(player);
	}
}

// 0x80007B08
void TCameraCtrl::lockF(int i)
{
	if (camIndexValid(i)) {
		mLock[i]->disable();
	}
}

// 0x80007B54
extern "C" void camLockAll()
{
	if (cameraCtrlExists()) {
		gCameraCtrl->lockAll();
	}
}

// 0x80007B84
void TCameraCtrl::lockAll()
{
	for (int i = 0; i < 4; i++) {
		mLock[i]->useUnlimitedRange();
	}
}

// 0x80007BD0
void CamView::init()
{
	mPos.x    = 0.0f;
	mPos.y    = 0.0f;
	mPos.z    = 0.0f;
	mTarget.x = 0.0f;
	mTarget.y = 0.0f;
	mTarget.z = 0.0f;
	mTarget.z = 1.0f;
	mFront.x  = 0.0f;
	mFront.y  = 0.0f;
	mFront.z  = 0.0f;
	mSide.x   = 0.0f;
	mSide.y   = 0.0f;
	mSide.z   = 0.0f;
	mUp.x     = 0.0f;
	mUp.y     = 0.0f;
	mUp.z     = 0.0f;
	mRoll     = 0;
	mAspect   = 1.3333334f;
	mBaseFov  = 0x2AAA;
	setFov(mBaseFov);
	mNear   = 1.0f;
	mFar    = 10000.0f;
	mFlat.x = 0.0f;
	mFlat.y = 0.0f;
	mFlat.z = 0.0f;
	mYaw    = 0;
}

// 0x80007C88
void CamView::setFov(int fov)
{
	mFov    = fov;
	f32 cos = njCos(mFov / 2);
	mSlopeX = njSin(mFov / 2) / cos;
}

// 0x80007CF0
void CamView::resetFov()
{
	setFov(mBaseFov);
	mRoll = 0;
}

// 0x80007D28
void CamView::apply()
{
	Vec3f up = { 0.0f, 1.0f, 0.0f };
	Vec3f dir;

	gjSetPerspective(mFov, mAspect, mNear, mFar);
	if (mRoll != 0) {
		dir.x = mTarget.x;
		dir.y = mTarget.y;
		dir.z = mTarget.z;
		njSubVector(&dir, &mPos);
		if (njUnitVector(&dir) < gUnitVectorMin) {
			dir = gVecZero;
		}
		f32 yaw   = atan2(dir.x, dir.z);
		f32 pitch = asin(-dir.y);
		gjPushUnitMatrix();
		gjRotateY(10430.38043493439 * yaw);
		gjRotateX(10430.38043493439 * pitch);
		gjRotateZ(mRoll);
		njCalcVectorEx(&up, &up);
		njPopMatrixEx();
	} else {
		up.x = 0.0f;
		up.y = 1.0f;
		up.z = 0.0f;
	}
	njCameraExLookAt(&mPos, &mTarget, &up);
	njPushMatrixEx();
	njInvertMatrix(0);
	mFront.x = 0.0f;
	mFront.y = 0.0f;
	mFront.z = -1.0f;
	mSide.x  = -1.0f;
	mSide.y  = 0.0f;
	mSide.z  = 0.0f;
	mUp.x    = 0.0f;
	mUp.y    = 1.0f;
	mUp.z    = 0.0f;
	njCalcVector(0, &mFront, &mFront);
	njCalcVector(0, &mSide, &mSide);
	njCalcVector(0, &mUp, &mUp);
	njGetMatrix(mMtx);
	njPopMatrixEx();
	mFlat.x = mFront.x;
	mFlat.y = mFront.y;
	mFlat.z = mFront.z;
	if (normalizeVector(&mFlat) <= 0.0f) {
		mFlat.x = mUp.x;
		mFlat.y = mUp.y;
		mFlat.z = mUp.z;
	}
	mYaw = 10430.38043493439 * (f32)atan2(mFlat.x, mFlat.z);
}
