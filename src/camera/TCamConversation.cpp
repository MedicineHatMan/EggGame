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

static f32 absf(f32 value)
{
	return value < 0.0f ? -value : value;
}

// 0x8008AC90
void TCamConversation::vfn_20()
{
	Vec3f pos;
	if (getTargetPos(&pos) == 0) {
		return;
	}
	Vec3f aim;
	aim = pos;
	njAddVector(&aim, &mFocus);
	gjScaleVector(0.5f, &aim, &aim);
	Vec3f* view = (Vec3f*)mView;
	view[1]     = aim;

	Vec3f from;
	from = mFocus;
	njSubVector(&from, &pos);
	Vec3f eye;
	eye = view[0];
	njSubVector(&eye, &aim);
	int turn = vecCrossXZ(&from, &eye) <= 0.0f ? mRoll : -mRoll;
	int face = (int)(10430.38043493439 * (double)atan2(from.x, from.z));
	turn += face;

	gjPushUnitMatrix();
	gjRotateX(-mYaw);
	gjRotateY(-turn);
	gjTranslate(-aim.x, -aim.y, -aim.z);
	njCalcPointEx(&pos, &eye);
	njPopMatrixEx();

	f32 scale = *(f32*)((u8*)mView + 0x48) * 0.6000000238418579f;
	f32 wide  = absf(eye.z) + absf(eye.x) / scale;
	f32 tall  = absf(eye.z) + absf(eye.y) / (scale / *(f32*)((u8*)mView + 0x4C));
	if (wide < tall) {
		wide = tall;
	}
	Vec3f fit;
	fit.x = 0.0f;
	fit.y = 0.0f;
	fit.z = -wide;
	gjPushUnitMatrix();
	njTranslateEx(&aim);
	gjRotateY(turn);
	gjRotateX(mYaw);
	njCalcPointEx(&fit, view);
	njPopMatrixEx();
}
