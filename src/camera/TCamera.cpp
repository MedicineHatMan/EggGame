#include "camera/TCamera.h"
#include "egg/EggCreature.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "camera/TCameraCtrl.h"
#include "object/TObjChickenBoss.h"
#include "core/MathUtil.h"
#include "object/TObjItem.h"
#include "Ninja/ninja.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "stl/math.h"
#include "camera/TCamEditor.h"

struct CamLookMsg : public ObjMsg {
	Vec3f mPos; // _04
};

extern "C" {

}

typedef char TCamera_size_check[(sizeof(TCamera) == 0x50) ? 1 : -1];

// 0x800056C0
void TObject::draw()
{
}

// 0x800056C4
void TObject::vfn_10()
{
}

// 0x800056C8
void TObject::vfn_14()
{
}

// 0x800056CC
void TObject::render()
{
}

// 0x800056D0
// Moves from toward to by t of the short way round
void lerpAngle(int from, int to, int* out, f32 t)
{
	f32 d = angleDiff(from, to);
	d *= t;
	*out = from + (int)d;
}

const char* gTCameraName = "TCamera";

// 0x80005748
TCamera::TCamera(TObject* parent, int player, int look)
    : TObject(parent)
{
	_00        = (u32)gTCameraName;
	mSize      = 0x50;
	mView      = camGetView(player);
	mKind      = camGetTarget(player);
	mPlayer    = player;
	camLinkInit(&mLook, look);
	camAddLink(player, &mLook);
	mLift      = 0.0f;
	mBlendStep = 1.0f;
	mBlend     = 0.0f;
	readFov();
}

// 0x800057EC
TCamera::~TCamera()
{
	release();
}

// 0x80005850
// When its link is newly picked the view starts from where it was and eases over to where this camera puts it round the
// target so the eye swings rather than cutting across
void TCamera::draw()
{
	CamView* view;
	Vec3f oldAt;
	Vec3f d;
	f32 oldEyeY;
	f32 oldEyeZ;
	f32 oldEyeX;
	int oldFov;
	int oldRoll;

	if (camLinkIsPicked(&mLook)) {
		if (vfn_28()) {
			if (canBlend()) {
				mBlend = 1.0f;
			} else {
				mBlend = 0.0f;
			}
		}
		if (mBlend > 0.0f) {
			view    = (CamView*)mView;
			oldAt   = view->mTarget;
			oldEyeX = view->mPos.x;
			oldEyeY = view->mPos.y;
			oldEyeZ = view->mPos.z;
			oldFov  = view->mFov;
			oldRoll = view->mRoll;
		}
		((CamView*)mView)->resetFov();
		((CamView*)mView)->mFov = mFov;
		vfn_20();

		if (mBlend > 0.0f) {
			f32 t;
			f32 newDist;
			f32 oldDist;
			f32 oldYaw;
			f32 u;
			f32 oldY;
			f32 newY;
			f32 newYaw;
			int yaw;

			mBlend -= mBlendStep;
			if (mBlendStep < 0.0f) {
				mBlendStep = 0.0f;
			}
			t   = mBlend;
			t   = t * (t * t);
			u   = 1.0f - t;
			d.x = oldEyeX;
			d.y = oldEyeY;
			d.z = oldEyeZ;
			njSubVector(&d, &oldAt);
			oldY    = d.y;
			oldDist = vecLenXZ(&d);
			oldYaw  = atan2(d.x, d.z);

			view = (CamView*)mView;
			d    = view->mPos;
			njSubVector(&d, &view->mTarget);
			newY    = d.y;
			newDist = vecLenXZ(&d);
			newYaw  = atan2(d.x, d.z);

			oldY    = t * oldY + u * newY;
			oldDist = t * oldDist + u * newDist;
			yaw     = (int)(10430.38043493439 * newYaw);
			lerpAngle(yaw, (int)(10430.38043493439 * oldYaw), &yaw, t);
			vecLerp(t, &((CamView*)mView)->mTarget, &oldAt, &((CamView*)mView)->mTarget);
			njSinCos(yaw, &d.x, &d.z);
			((CamView*)mView)->mPos = ((CamView*)mView)->mTarget;
			((CamView*)mView)->mPos.x = d.x * oldDist + ((CamView*)mView)->mPos.x;
			((CamView*)mView)->mPos.y = ((CamView*)mView)->mPos.y + oldY;
			((CamView*)mView)->mPos.z = d.z * oldDist + ((CamView*)mView)->mPos.z;
			lerpAngle(((CamView*)mView)->mFov, oldFov, &((CamView*)mView)->mFov, t);
			lerpAngle(((CamView*)mView)->mRoll, oldRoll, &((CamView*)mView)->mRoll, t);
		}
	} else {
		vfn_24();
	}
}

// 0x80005B6C
void TCamera::vfn_24()
{
}

// 0x80005B70
BOOL TCamera::isPicked()
{
	return camLinkIsPicked(&mLook);
}

// 0x80005B94
void TCamera::release()
{
	camRemoveLink(mPlayer, &mLook);
}

// 0x80005BC0
// Never 0 which would never finish and at most the whole of it
void TCamera::setBlendStep(f32 step)
{
	if (step <= 0.0f) {
		step = 0.002f;
	} else if (step > 1.0f) {
		step = 1.0f;
	}
	mBlendStep = step;
}

// 0x80005BF0
// Not when it would finish at once nor on a cut
int TCamera::canBlend() const
{
	u8 can = FALSE;

	if (mBlendStep < 1.0f && !camLinkIsCut(&mLook)) {
		can = TRUE;
	}
	return can;
}

// 0x80005C40
int TCamera::vfn_28()
{
	return camLinkIsEntered(&mLook);
}

// 0x80005C64
void TCamera::setCut()
{
	camLinkSetCut(&mLook);
}

// 0x80005C88
u16 TCamera::getTarget()
{
	return mKind;
}

// 0x80005C90
void TCamera::setTarget(u16 id)
{
	mKind = id;
}

// 0x80005C98
// The object with an id and where a camera should look at it where it says or its position
extern "C" TObjAction* camFindTarget(int id, Vec3f* pos)
{
	TObjAction* obj = findObjById(id);

	if (obj != 0) {
		CamLookMsg msg;
		msg.mType = 0x3F;
		if (obj->sendToHandle(&msg)) {
			*pos = msg.mPos;
		} else {
			*pos = *obj->getPos();
		}
	}
	return obj;
}

// 0x80005D40
// Objects with no place are at the origin
Vec3f* TObjAction::getPos()
{
	return &gVecZero;
}

// 0x80005D4C
// Objects take no messages unless they say so
int TObjAction::sendToHandle(ObjMsg*)
{
	return 0;
}

// 0x80005D54
BOOL TCamera::getTargetPos(Vec3f* out)
{
	if (camFindTarget(mKind, out) != 0) {
		out->y += mLift;
		return TRUE;
	}
	return FALSE;
}

// 0x80005DAC
void TCamera::setLift(f32 lift)
{
	mLift = lift;
}

// 0x80005DB4
void TCamera::setFov(int fov)
{
	mFov = fov;
}

// 0x80005DBC
void TCamera::readFov()
{
	if (mView != 0) {
		mFov = ((CamView*)mView)->mBaseFov;
	} else {
		mFov = 0x2AAA;
	}
}

// 0x80005DE0
void TCamera::vfn_30()
{
}

// 0x80005DE4
void TCamera::setEye(const Vec3f* eye)
{
	((CamView*)mView)->mPos = *eye;
}

// 0x80005E04
void TCamera::getEye(Vec3f* out)
{
	*out = ((CamView*)mView)->mPos;
}

// 0x80005E24
void TCamera::setAt(const Vec3f* at)
{
	((CamView*)mView)->mTarget = *at;
}

// 0x80005E44
void TCamera::getAt(Vec3f* out)
{
	*out = ((CamView*)mView)->mTarget;
}

// 0x80005E64
void TCamera::vfn_48()
{
}

// 0x80005E68
void TCamera::vfn_4C()
{
}

// 0x80005E6C
int TCamera::getYaw()
{
	if (mView != 0) {
		return ((CamView*)mView)->mYaw;
	}
	return 0;
}

// 0x80005E88
void TCamera::shake(f32 power)
{
}

// 0x80005E8C
void TCamera::setMode(int mode)
{
}

// 0x80005E90
extern "C" void camSavedClear(CamSaved* saved)
{
	saved->mSet = 0;
	saved->mA   = 0.0f;
	saved->mB   = 20.0f;
	saved->mC   = 75.0f;
}

// 0x80005EB4
extern "C" void camCtrlInit()
{
	gCameraCtrl = 0;
	clearCamAreas();
	camZeldaInit();
}

// 0x80005EE0
extern "C" void camCtrlTerm()
{
	camZeldaTerm();
	freeCamAreas();
	if (gCameraCtrl != 0) {
		delete gCameraCtrl;
		gCameraCtrl = 0;
	}
}

// 0x80005F30
extern "C" TCameraCtrl* createCameraCtrl()
{
	gCameraCtrl = new TCameraCtrl();
	if (gCameraCtrl != 0) {
		gCameraCtrl->makeGameCameras();
	}
	return gCameraCtrl;
}

// 0x80005F80
extern "C" TCameraCtrl* createEditCameraCtrl()
{
	gCameraCtrl = new TCameraCtrl();
	if (gCameraCtrl != 0) {
		gCameraCtrl->makeEditCameras();
	}
	return gCameraCtrl;
}

// 0x80005FD0
void TCameraCtrl::makeGameCameras()
{
	makeCameras(0);
}

// 0x80005FF4
void TCameraCtrl::makeEditCameras()
{
	makeCameras(1);
}

// 0x80006018
// Swaps every player's camera for a new one which follows what the old one did
void TCameraCtrl::makeCameras(int mode)
{
	u16 target;
	TCamera* made;
	int i;

	for (i = 0; i < 4; i++) {
		target = 0xFFFF;

		if (mEditor[i] != 0) {
			mEditor[i]->mFlags |= 1;
			mEditor[i] = 0;
		}
		if (mCamera[i] != 0) {
			target = mCamera[i]->getTarget();
			mCamera[i]->mFlags |= 1;
		}
		switch (mode) {
		case 0:
			mCamera[i] = createTCamZelda5(i, 0);
			made       = mCamera[i];
			break;
		case 1:
			mEditor[i] = createTCamEditor(i, 0, (void*)1);
			made       = mEditor[i];
			break;
		}
		if (made != 0) {
			made->setTarget(target);
		}
	}
}
