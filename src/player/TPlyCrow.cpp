#include "player/TPlyCrow.h"
#include "engine/CharaColli.h"
#include "engine/Light.h"
#include "engine/Color.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "core/Mem.h"
#include "object/TObjEggRing.h"
#include "engine/Colli.h"
#include "engine/FxSystem.h"
#include "core/MathUtil.h"
struct CharaColli;
struct ColliShape;

extern "C" {

extern const ColliShape sPlyCrowShape[] = {
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 15.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

typedef char TPlyCrow_size_check[(sizeof(TPlyCrow) == 0xDC) ? 1 : -1];

const char* gTPlyCrowName = "TPlyCrow";

// 0x8003F738
TPlyCrow::TPlyCrow(TObject* parent, u16 id, const f32* mtx)
    : TObject(parent)
    , mId(id)
{
	Vec3f pos;

	_00    = (u32)gTPlyCrowName;
	mSize  = 0xDC;
	mState = 0;
	mLife  = 0x32;
	mtxCopy(mMtx, mtx);
	mColli = colliCreate(1, 1);
	if (mColli != 0) {
		colliSetOwner((CharaColli*)mColli, id);
		colliCopyShapes((CharaColli*)mColli, (const ColliShape*)sPlyCrowShape);
		colliSetMasks((CharaColli*)mColli, 1, 0, 0xFF, 0x10);
		colliGetShape((CharaColli*)mColli, 0)->mFlags = 0x58;
		pos.x                                         = 0.0f;
		pos.y                                         = 0.0f;
		pos.z                                         = 10.0f;
		mtxMultVec(mtx, &pos, &pos);
		colliPlaceShapes((CharaColli*)mColli, &pos, (const ColliShape*)sPlyCrowShape);
	}
	mLight.mFlags = 2;
	mPulse        = 0;
}

// 0x8003F8B8
TPlyCrow::~TPlyCrow()
{
	colliFree((CharaColli*)mColli);
	playerRemoveLight(mId, &mLight);
}

// 0x8003F954
void TPlyCrow::draw()
{
	u32 color;
	u32 grey;
	u32 white;
	Vec3f pos;

	switch (mState) {
	case 0:
		playerAddLight(mId, &mLight);
		pos.x = 0.0f;
		pos.y = pos.x;
		pos.z = 10.0f;
		mtxMultVec(mMtx, &pos, &pos);
		mLight.mPos = pos;
		mLit        = 0x14;
		fxSpawnAt(1, 0, 0x1B, mMtx);
		mState = 1;
	case 1:
		white = 0xFFFFFFFF;
		grey  = 0xC8C8C8FF;
		lerpColor(0.5f * njSin(mPulse) + 0.5f, &color, &grey, &white);
		mLight.mColor = color;
		mPulse += 0x1000;
		if (mLit != 0) {
			if (--mLit == 0) {
				playerRemoveLight(mId, &mLight);
			}
		}
		if (--mLife == 0) {
			mFlags |= 1;
		}
		break;
	}
}
