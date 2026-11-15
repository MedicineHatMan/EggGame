#include "object/ObjAction.h"
#include "engine/CharaColli.h"
#include "object/TObjChickenBomb.h"
#include "enemy/boss/EneBoss.h"
#include "egg/EggCreature.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "engine/FxSystem.h"
#include "camera/TCameraCtrl.h"
#include "sound/Sound3DPlay.h"
#include "object/TObjExplode.h"
#include "core/TMainTask.h"
struct CharaColli;

extern "C" {
}

typedef char TObjExplode_size_check[(sizeof(TObjExplode) == 0x44) ? 1 : -1];

// 0x8018B6CC
// The bombs blow up through this the blast goes on the effect layer
extern "C" void spawnExplode(const Vec3f* pos, u16 id, int kind)
{
	new TObjExplode(gTL_06, pos, id, (u16)kind);
}

const char* gTObjExplodeName = "TObjExplode";

// 0x8018B724
TObjExplode::TObjExplode(TObject* parent, const Vec3f* pos, u16 id, u16 kind)
    : TObjAction(parent)
    , mPos(*pos)
{
	f32 mtx[12];
	ColliShape* shape;

	_00       = (u32)gTObjExplodeName;
	mSize     = 0x44;
	mHolderId = id;
	mKind     = kind;
	setObjId(this, mObjId);
	mShape = colliCreate(1, 1);
	if (mShape != 0) {
		colliSetOwner((CharaColli*)mShape, (u16)mObjId);
		colliSetMasks((CharaColli*)mShape, 1, 255, 255, 0xF0);
		colliShapeSetSphere(colliGetShape((CharaColli*)mShape, 0));
		colliGetShape((CharaColli*)mShape, 0)->mFlags = 8;
		colliGetShape((CharaColli*)mShape, 0)->mSizeX = 30.0f;
		colliShapeSetPos(colliGetShape((CharaColli*)mShape, 0), pos);
	}
	mLife = 120;
	mtxSetTransV(mtx, &mPos);
	fxSpawnAt(1, 7, 0x33, mtx);
	sePlay3DRange(1000.0f, 0x4D, pos, 0xF0);
	camShakeNearHard(1000.0f, pos);
}

// 0x8018B888
TObjExplode::~TObjExplode()
{
	colliFree((CharaColli*)mShape);
	unregisterId();
}

// 0x8018B8F8
void TObjExplode::draw()
{
	if (mLife != 0) {
		if (--mLife == 0) {
			mFlags |= 1;
		}
	}
}

// 0x8018B924
int TObjExplode::onHit(const TObjAction*, int, int* result)
{
	HitInfo* hit = (HitInfo*)result;
	resetHitInfo(hit);
	hit->mDamage = mKind;
	hit->mFlags.setBit(0xE)->setBit(0xD);
	return 1;
}

// 0x8018B978
int TObjExplode::getKind()
{
	return 3;
}

// 0x8018B980
Vec3f* TObjExplode::getPos()
{
	return &mPos;
}

// 0x8018B988
extern "C" void spawnChickenBomb(s16 owner, const Vec3f* pos)
{
	new TObjChickenBomb(gTL_04, owner, pos);
}
