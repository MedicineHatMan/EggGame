#include "engine/WaterSurface.h"
#include "object/ObjAction.h"
#include "object/green/TObjGreenPondScene.h"
#include "camera/TCamera.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "core/MathUtil.h"
#include "engine/FxSystem.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "object/green/TObjGreenBossRay.h"
struct Fx;

typedef char TObjGreenBossRay_size_check[(sizeof(TObjGreenBossRay) == 0x98) ? 1 : -1];

// 0x80193268
extern "C" TObject* createTObjGreenBossRay(TObject* parent, SetDesc* desc)
{
	return new TObjGreenBossRay(parent, desc);
}

const char* gTObjGreenBossRayName = "TObjGreenBossRay";

// 0x801932BC
TObjGreenBossRay::TObjGreenBossRay(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjGreenBossRayName;
	mSize = 0x98;
	initFromDesc(desc);
	vfn_74();
	updateMatrix();
	mFx = fxSpawnFollow(1, 8, 2, mMatrix);
}

// 0x80193358
TObjGreenBossRay::~TObjGreenBossRay()
{
	fxRemove((Fx*)mFx);
}

// 0x801933C0
void TObjGreenBossRay::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
}

// 0x801933E0
void TObjGreenBossRay::vfn_74()
{
}

// 0x801933E4
void TObjGreenBossRay::updateMatrix()
{
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateZYX((f32*)mMatrix, (const f32*)mMatrix, (const int*)&mRot);
}

// 0x80193428
void TObjGreenBossRay::draw()
{
}

// 0x8019342C
void TObjGreenBossRay::vfn_10()
{
	if (objDebugEnabled(this) != 0) {
		updateMatrix();
		fxSetFollow(mFx, (f32*)mMatrix);
	}
}

// 0x80193474
void TObjGreenBossRay::render()
{
}
