#include "core/VecMath.h"
#include "animal/AniMount.h"
#include "animal/AniAttacks.h"
#include "enemy/boss/EneBoss.h"
#include "engine/Colli.h"
#include "object/TObjEggRing.h"
#include "core/MathUtil.h"
#include "engine/GroundQuery.h"
#include "animal/TAniLionFireBreath.h"

const char* gTAniLionFireBreathName = "TAniLionFireBreath";
struct CharaColli;

// 0x801D1040
TAniLionFireBreath::TAniLionFireBreath(TObject* parent, u16 ownerId)
    : TObject(parent)
{
	mOwnerId = ownerId;
	_00      = (u32)gTAniLionFireBreathName;
	mSize    = 0x58;
	mColli   = (CharaColli*)colliCreate(5, 1);
	if (mColli != 0) {
		colliSetOwner(mColli, (u16)ownerId);
		colliSetMasks(mColli, 0x20, 0, 0xFF, 0x10);
		for (int i = 0; i < 5; i++) {
			ColliShape* shape = (ColliShape*)colliGetShape((CharaColli*)mColli, i);
			colliShapeSetSphere(shape);
			colliShapeSetRadius(shape, 20.0f);
			shape->mFlags |= 0x18;
			shape->mTag = 0;
		}
	}
}

// 0x801D1114
TAniLionFireBreath::~TAniLionFireBreath()
{
	colliFree(mColli);
}

// 0x801D117C
void TAniLionFireBreath::setMatrix(const f32* matrix)
{
	mtxCopy(mMatrix, matrix);
}

// 0x801D11A0
void TAniLionFireBreath::draw()
{
	Vec3f from;
	mtxGetPos(mMatrix, &from);
	Vec3f to;
	mtxMultVec(mMatrix, (const Vec3f*)&sLionBreathReach, &to);
	EggHits hits;
	if (meshColliProbeLine((EggHit*)hits.mHits, (const Vec3f*)&from, (const Vec3f*)&to, (unsigned long)1) == 0) {
		to = hits.mHits[0].mPos;
	}
	Vec3f step;
	vecSub(&step, &to, &from);
	vecScale(0.25f, &step, &step);
	Vec3f at;
	at = from;
	for (int i = 0; i < 5; i++) {
		colliShapeSetPos(colliGetShape(mColli, i), &at);
		vecAdd(&at, &at, &step);
	}
}

struct BreathReach {
	BreathReach(f32 nx, f32 ny, f32 nz)
	{
		x = nx;
		y = ny;
		z = nz;
	}
	f32 x, y, z;
};

// The __sinit at 0x801D12A4 fills this
BreathReach sLionBreathReach(0.0f, 0.0f, 100.0f);
