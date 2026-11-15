#include "object/purple/TObjPurpleBlizzard.h"
#include "io/Resource.h"
#include "animal/AniMount.h"
#include "egg/EggCreature.h"
#include "camera/TCameraCtrl.h"
#include "object/TObjWindColli.h"
#include "stage/Stage.h"
#include "ui/AeArc.h"
#include "Ninja/ninja.h"
#include "engine/FxSystem.h"
#include "core/MathUtil.h"
#include "object/TObjEggRing.h"
#include "stl/math.h"
#include "core/VecMath.h"
#include "object/TObjSnowman.h"
#include "core/TMainTask.h"
struct NJS_TEXLIST;
struct Fx;

typedef char TObjPurpleBlizzard_size_check[(sizeof(TObjPurpleBlizzard) == 0x2C) ? 1 : -1];

// 0x800FBF8C
// The set table's init
extern "C" void blizzardInit()
{
	gBlizzard = 0;
}

// 0x800FBF98
// The set table's exit
extern "C" void blizzardExit()
{
	gBlizzard = 0;
}

// 0x800FBFA4
extern "C" void blizzardCreate(int kind)
{
	gBlizzard = new TObjPurpleBlizzard(gTL_07, kind);
}

// 0x800FBFF4
// The event script's blizzard
extern "C" int scriptBlizzard(int* args)
{
	blizzardCreate(args[0]);
	return 1;
}

// 0x800FC01C
TObjPurpleBlizzard::TObjPurpleBlizzard(TObject* parent, int kind)
    : TObject(parent)
{
	mKind = kind;
	_00   = gTObjPurpleBlizzardName;
	mSize = 0x2C;

	switch (kind) {
	case 0:
		mFx  = fxSpawn(1, 8, 0);
		mFx2 = fxSpawn(1, 8, 1);
		break;
	case 1:
		mFx  = fxSpawn(1, 8, 2);
		mFx2 = fxSpawn(1, 8, 3);
		break;
	case 2:
		mFx  = fxSpawn(1, 8, 4);
		mFx2 = 0;
		break;
	default:
		mFx2 = 0;
		mFx  = 0;
		break;
	}
}

// 0x800FC118
// Clears gBlizzard when it points at this
TObjPurpleBlizzard::~TObjPurpleBlizzard()
{
	fxStop((Fx*)mFx);
	fxStop((Fx*)mFx2);
	if (gBlizzard == this) {
		gBlizzard = 0;
	}
}

// 0x800FC19C
void TObjPurpleBlizzard::freeEffects()
{
	fxStop(mFx);
	fxStop(mFx2);
	mFx  = 0;
	mFx2 = 0;
}

// 0x800FC1E0
// camGetDrawPos and camGetDrawFlat are the pair TObjGreenSpore uses to sit 100 ahead of the camera
void TObjPurpleBlizzard::draw()
{
	Vec3f pos;
	Vec3f at;
	f32 matrix[12];
	Vec3f dir;
	f32 sinA;
	f32 cosA;
	f32 sinB;
	f32 cosB;

	mtxIdentity(matrix);
	camGetDrawPos(&pos);
	camGetDrawFlat(&dir);
	vecScaleAdd(100.0f, &pos, &dir, &pos);
	if (mKind == 2) {
		if (mFx != 0) {
			pos.y += 50.0f;
			mtxSetTransV(matrix, &pos);
			mtxCopy(mFx->mMatrix, matrix);
		}
	} else {
		windGet(&dir);
		if (0.0f == vecNormalize(&dir, &dir)) {
			dir.x = 0.0f;
			dir.y = 0.0f;
			dir.z = 1.0f;
		}

		njSinCos((int)(10430.38043493439 * (f32)atan2(dir.y, dir.z)), &sinA, &cosA);
		njSinCos((int)(10430.38043493439 * (f32)atan2(dir.x, dir.y * sinA + dir.z * cosA)), &sinB, &cosB);

		matrix[0]  = cosB;
		matrix[1]  = 0.0f;
		matrix[2]  = sinB;
		matrix[4]  = -sinA * sinB;
		matrix[5]  = cosA;
		matrix[6]  = sinA * cosB;
		matrix[8]  = -cosA * sinB;
		matrix[9]  = -sinA;
		matrix[10] = cosA * cosB;

		if (mFx != 0) {
			vecScaleAdd(-200.0f, &at, &dir, &pos);
			matrix[3]  = at.x;
			matrix[7]  = 50.0f + at.y;
			matrix[11] = at.z;
			mtxCopy(mFx->mMatrix, matrix);
		}
		if (mFx2 != 0) {
			vecScaleAdd(-20.0f, &at, &dir, &pos);
			matrix[3]  = at.x;
			matrix[7]  = at.y;
			matrix[11] = at.z;
			mtxCopy(mFx2->mMatrix, matrix);
		}
	}
}
