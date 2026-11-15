#include "object/ObjAction.h"
#include "core/MathUtil.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "engine/FxSystem.h"
#include "sound/Sound3DPlay.h"
#include "engine/Colli.h"
#include "object/last/TObjLastRipple.h"
struct ColliShape;
struct CharaColli;
struct Fx;

extern "C" {

}

typedef char TObjLastRipple_size_check[(sizeof(TObjLastRipple) == 0x48) ? 1 : -1];

extern "C" TObjLastRipple* createTObjLastRipple(TObject* parent, const Vec3f* pos)
{
	return new TObjLastRipple(parent, pos);
}

extern const char* gTObjLastRippleName;

// 0x801A8F88
TObjLastRipple::TObjLastRipple(TObject* parent, const Vec3f* pos)
    : TObjAction(parent)
    , mPos(*pos)
{
	mRadius = 10.0f;
	mLife   = 0;
	_00     = (u32)gTObjLastRippleName;
	mSize   = 0x48;
	setObjId(this, -1);
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 1, 0xFF, 0xFF, 0xFE);
	colliShapeMakeSphere(colliGetShapes((CharaColli*)mShape), &mPos, 0, mRadius);
	f32 mtx[12];
	mtxSetTransV(mtx, &mPos);
	fxSpawnAt(1, 8, 1, mtx);
	sePlay3D(0x28C, &mPos);
}

// 0x801A9088
TObjLastRipple::~TObjLastRipple()
{
	colliRetire((CharaColli*)mShape);
	releaseObjId(this);
}

// 0x801A90F8
void TObjLastRipple::draw()
{
	mRadius = lerpF(10.0f, 60.0f, (f32)mLife / 80.0f);
	if ((f32)mLife > 80.0f) {
		mFlags |= 1;
	}
	mLife = (u16)(mLife + 1);
	*(f32*)((u8*)colliGetShapes(mShape) + 0x18) = mRadius;
}

// 0x801A91B4
int TObjLastRipple::getKind()
{
	return 3;
}
