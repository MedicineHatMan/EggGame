#include "egg/TEggCreature.h"
#include "object/ObjCommon.h"
#include "enemy/boss/EneBoss.h"
#include "egg/EggCreature.h"
#include "animal/TAniManager.h"
struct TEggCreatureTypeE;

// 0x80050700
TEggCreature::TEggCreature(TObject* parent, int kind)
    : TObjAction(parent)
{
	mLink.mNext   = NULL;
	mLink.mPrev   = NULL;
	mLink.mList   = NULL;
	mCreatureKind = kind;
	_44           = 0;
	setObjId(this, 0xFFFF);
	mLink.mOwner = (TEggCreatureTypeE*)this;
	aniManagerAdd((TEggCreatureTypeE*)this);
}

// 0x80050774
TEggCreature::~TEggCreature()
{
	releaseObjId(this);
	aniManagerRemove((TEggCreatureTypeE*)this);
}

// 0x800507E0
TEggCreature* TEggCreature::find(int id)
{
	TObjAction* obj = findObjById((u16)id);
	if (obj != NULL) {
		return cast(obj);
	}
	return NULL;
}

// 0x80050818
TEggCreature* TEggCreature::cast(TObjAction* obj)
{
	if (obj != NULL && obj->getKind() == 4) {
		return (TEggCreature*)obj;
	}
	return NULL;
}

// 0x80050868
int TEggCreature::onMsg44(ObjMsg* msg)
{
	vfn_70();
	return 1;
}

// 0x80050898
int TEggCreature::onMsg46(ObjMsg* msg)
{
	vfn_78();
	return 1;
}

// 0x800508C8
int TEggCreature::onMsg47(ObjMsg* msg)
{
	return vfn_74();
}

// 0x800508F4
int TEggCreature::onMsg23(ObjMsg* msg)
{
	vfn_80((u8*)msg + 4);
	return 1;
}

// 0x80050928
void TEggCreature::vfn_70()
{
}

// 0x8005092C
void TEggCreature::vfn_78()
{
	mFlags |= 1;
}

// 0x8005093C
int TEggCreature::vfn_74()
{
	return 0;
}

// 0x80050944
void TEggCreature::vfn_80(const void* body)
{
}

// 0x80050948
void TEggCreature::setPose(const Vec3f* pos, int rot)
{
}

// 0x8005094C
void TEggCreature::vfn_7C()
{
	mFlags |= 1;
}

// 0x8005095C
int TEggCreature::vfn_84()
{
	return -1;
}

// 0x80050964
int TEggCreature::vfn_88()
{
	return 0;
}

// 0x8005096C
void TEggCreature::vfn_8C(u32 flags)
{
}

// 0x80050970
void TEggCreature::vfn_90(u32 flags)
{
}

// 0x80050974
int TEggCreature::getKind()
{
	return 4;
}
