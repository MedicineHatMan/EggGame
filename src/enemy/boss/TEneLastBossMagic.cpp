#include "object/ObjAction.h"
#include "sound/SeHolder.h"
#include "enemy/boss/TEneLastBossMagic.h"
#include "io/ResBank.h"
#include "enemy/ObjEnemy.h"
#include "engine/Colli.h"
#include "enemy/EnemyFx.h"
#include "object/last/TObjLastEggLight.h"
struct ColliShape;
struct CharaColli;

extern "C" {
}

typedef char TEneLastBossMagic_size_check[(sizeof(TEneLastBossMagic) == 0x4C) ? 1 : -1];

const char* gTEneLastBossMagicName = "TEneLastBossMagic";

// 0x8015EA14
f32 lastBossMagicGrowFrames()
{
	return 35.0f;
}

// 0x8015EA1C
TEneLastBossMagic::TEneLastBossMagic(TObject* parent, const Vec3f* pos, f32 life)
    : TObjAction(parent)
{
	_00   = (u32)gTEneLastBossMagicName;
	mSize = 0x4C;
	setObjId(this, 0xFFFF);
	mPos   = *pos;
	mLife  = life;
	mStep  = mLife / 35.0f;
	mScale = 0.0f;
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 2, 255, 255, 0x16);
	colliShapeMakeCylinder(colliGetShapes((CharaColli*)mShape), &mPos, 1, mStep, 100.0f);
	colliEnable((CharaColli*)mShape);
	colliShapeSetBit8((CharaColli*)mShape, 0);
	colliShapeSetBit10((CharaColli*)mShape, 0);
	colliShapeSetTag(mShape, 0, 0);
	fxSpawnScaled(mLife / 77.0f, mLife / 77.0f, 1.0f, 5, (void*)0xB, (const Vec3f*)&mPos, (int)0xFFFFC000, 0);
	seStartAtFar(0x195, &mPos);
}

// 0x8015EB7C
TEneLastBossMagic::~TEneLastBossMagic()
{
	colliRetire((CharaColli*)mShape);
	mShape = 0;
	releaseObjId(this);
}

// 0x8015EBF4
int TEneLastBossMagic::onHit(const TObjAction* other, int, int* result)
{
	HitInfo* hit = (HitInfo*)result;
	int found    = 0;

	if (isPlayerOrKind4(other) != 0) {
		found = 1;
		hit->mFlags.clear();
		hit->mPos.x  = 0.0f;
		hit->mPos.y  = 0.0f;
		hit->mPos.z  = 0.0f;
		hit->mDamage = 10;
	} else if (isEggObj(other) != 0) {
		found = 1;
		hit->mFlags.clear();
		hit->mPos.x  = 0.0f;
		hit->mPos.y  = 0.0f;
		hit->mPos.z  = 0.0f;
		hit->mDamage = 1;
		hit->mFlags.setBit(0xD);
	}
	return found;
}

// 0x8015ECA0
void TEneLastBossMagic::resize()
{
	ColliShape* shapes = colliGetShapes(mShape);
	colliShapeSetPos(shapes, &mPos);
	colliShapeSetCylRadius(shapes, mScale);
	colliRefreshBounds(mShape);
}

// 0x8015ECF0
void TEneLastBossMagic::draw()
{
	mScale += mStep;
	if (mScale >= mLife) {
		mScale = mLife;
		mFlags |= 1;
	}
	resize();
}

// 0x8015ED44
int TEneLastBossMagic::getKind()
{
	return 2;
}

// 0x8015ED4C
Vec3f* TEneLastBossMagic::getPos()
{
	return &mPos;
}
