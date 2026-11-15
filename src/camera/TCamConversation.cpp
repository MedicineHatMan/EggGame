#include "camera/TCamera.h"
#include "core/MathUtil.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "stl/math.h"
#include "Ninja/ninja.h"

extern "C" {
TObject* gTL_01;
}

typedef char TCamConversation_size_check[(sizeof(TCamConversation) == 0x68) ? 1 : -1];

// 0x8008AB30
extern "C" TCamConversation* createTCamConversation(int player, int look, const Vec3f* focus, int yaw, int roll)
{
	return new TCamConversation(gTL_01, player, look, focus, yaw, roll);
}

const char* gTCamConversationName = "TCamConversation";

// 0x8008ABA0
TCamConversation::TCamConversation(TObject* parent, int player, int look, const Vec3f* focus, int yaw, int roll)
    : TCamera(parent, player, look)
{
	_00    = (u32)gTCamConversationName;
	mSize  = 0x68;
	mFocus = *focus;
	mYaw   = yaw;
	mRoll  = roll;
}

// 0x8008AC18
TCamConversation::~TCamConversation()
{
}

// 0x8008AC74
void TCamConversation::setFocus(const Vec3f* focus)
{
	mFocus = *focus;
}

// 0x8008AC90
void TCamConversation::vfn_20()
{
	Vec3f pos;
	Vec3f from;
	Vec3f eye;
	Vec3f aim;
	if (getTargetPos(&pos) == 0) {
		return;
	}
	aim = pos;
	njAddVector(&aim, &mFocus);
	gjScaleVector(0.5f, &aim, &aim);
	((CamView*)mView)->mTarget = aim;

	from = mFocus;
	njSubVector(&from, &pos);
	eye = ((CamView*)mView)->mPos;
	njSubVector(&eye, &aim);
	int turn = vecCrossXZ(&from, &eye) > 0.0f ? -mRoll : mRoll;
	int face = (int)(10430.38043493439 * (double)(f32)atan2(from.x, from.z));
	turn += face;

	gjPushUnitMatrix();
	gjRotateX(-mYaw);
	gjRotateY(-turn);
	gjTranslate(-aim.x, -aim.y, -aim.z);
	njCalcPointEx(&pos, &eye);
	njPopMatrixEx();

	f32 scale = ((CamView*)mView)->mSlopeX;
	scale *= 0.6000000238418579f;
	f32 wide = (f32)fabs(eye.z) + (f32)fabs(eye.x) / scale;
	scale    = scale / ((CamView*)mView)->mAspect;
	f32 tall = (f32)fabs(eye.z) + (f32)fabs(eye.y) / scale;
	f32 dist = tall > wide ? tall : wide;
	eye.x    = 0.0f;
	eye.y    = 0.0f;
	eye.z    = -dist;
	gjPushUnitMatrix();
	njTranslateEx(&aim);
	gjRotateY(turn);
	gjRotateX(mYaw);
	njCalcPointEx(&eye, &((CamView*)mView)->mPos);
	njPopMatrixEx();
}
