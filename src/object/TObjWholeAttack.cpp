#include "object/ObjAction.h"
#include "engine/CharaColli.h"
#include "camera/TCamera.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "engine/Colli.h"
#include "object/TObjWholeAttack.h"
#include "core/TMainTask.h"

const char* gTObjWholeAttackName = "TObjWholeAttack";
struct CharaColli;

typedef char TObjWholeAttack_size_check[(sizeof(TObjWholeAttack) == 0x44) ? 1 : -1];

// 0x801CE0A0
extern "C" void createWholeAttack(const Vec3f* pos)
{
	new TObjWholeAttack(gTL_06, pos);
}

// 0x801CE0E8
TObjWholeAttack::TObjWholeAttack(TObject* parent, const Vec3f* pos)
    : TObjAction(parent)
    , mPos(*pos)
{
	mTimer = 10;
	_00    = (u32)gTObjWholeAttackName;
	mSize  = 0x44;
	setObjId(this, 0xFFFF);

	mShape = colliCreate(1, 1);
	if (mShape != 0) {
		colliSetOwner((CharaColli*)mShape, (u16)mObjId);
		colliSetMasks((CharaColli*)mShape, 4, 0, 0, 0xFD);
		colliShapeSetSphere(colliGetShape((CharaColli*)mShape, 0));
		colliShapeSetRadius(colliGetShape((CharaColli*)mShape, 0), 1000.0f);
		colliShapeSetPos(colliGetShape((CharaColli*)mShape, 0), pos);
		colliGetShape((CharaColli*)mShape, 0)->mFlags |= 0x18;
	}
}

// 0x801CE1F4
TObjWholeAttack::~TObjWholeAttack()
{
	colliFree((CharaColli*)mShape);
	unregisterId();
}

// 0x801CE264
void TObjWholeAttack::draw()
{
	if (mTimer != 0) {
		if (--mTimer == 0) {
			mFlags |= 1;
		}
	}
}

// 0x801CE290
int TObjWholeAttack::onHit(const TObjAction*, int, int* result)
{
	*result = 50;
	return 1;
}

// 0x801CE2A0
Vec3f* TObjWholeAttack::getPos()
{
	return &mPos;
}

// 0x801CE2A8
int TObjWholeAttack::getKind()
{
	return 3;
}
