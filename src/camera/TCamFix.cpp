#include "camera/TCamera.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"

extern const f32 gUnitVectorMin;

extern "C" {

}

typedef char TCamFix_size_check[(sizeof(TCamFix) == 0x68) ? 1 : -1];

const char* gTCamFixName = "TCamFix";

// 0x8008A418
TCamFix::TCamFix(TObject* parent, int player, int look, const Vec3f* at, const Vec3f* eye)
    : TCamera(parent, player, look)
{
	_00   = (u32)gTCamFixName;
	mSize = 0x68;
	mAt   = *at;
	mEye  = *eye;
	Vec3f dir;
	dir = *eye;
	njSubVector(&dir, at);
	f32 length = njUnitVector(&dir);
	if (length < gUnitVectorMin) {
		dir = gVecZero;
	}
	if (length < 5.0f) {
		gjMulAddVector(5.0f, &mEye, &mAt, &dir);
	}
}

// 0x8008A504
TCamFix::~TCamFix()
{
}

// 0x8008A560
void TCamFix::setAt(const Vec3f* at)
{
	mAt = *at;
}

// 0x8008A57C
void TCamFix::setEye(const Vec3f* eye)
{
	mEye = *eye;
}

// 0x8008A598
void TCamFix::vfn_20()
{
	((Vec3f*)mView)[0] = mEye;
	((Vec3f*)mView)[1] = mAt;
}

// 0x8008A5D4
void TCamFix::vfn_10()
{
}
