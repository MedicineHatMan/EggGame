#include "stage/SetManager.h"
#include "stage/SetActor.h"
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "core/MathUtil.h"
#include "engine/FxSystem.h"
#include "object/green/TObjGreenRay.h"
struct Fx;

typedef char TObjGreenRay_size_check[(sizeof(TObjGreenRay) == 0x98) ? 1 : -1];

const char* gTObjGreenRayName = "TObjGreenRay";
const f32 gGreenRaySetRange   = 2000.0f;

// 0x801A2B34
extern "C" TObject* createTObjGreenRay(TObject* parent, SetDesc* desc)
{
	return new TObjGreenRay(parent, desc);
}

// 0x801A2B88
TObjGreenRay::TObjGreenRay(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjGreenRayName;
	mSize = 0x98;
	initFromDesc(desc);
	mFx = fxSpawnFollow(1, 8, 7, mMatrix);
}

// 0x801A2C08
TObjGreenRay::~TObjGreenRay()
{
	fxRemove((Fx*)mFx);
}

// 0x801A2C70
void TObjGreenRay::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateZYX((f32*)mMatrix, (const f32*)mMatrix, (const int*)&mRot);
}

// 0x801A2CB8
void TObjGreenRay::draw()
{
	if (!isInRange(this, &mPos, 2100.0f)) {
		mFlags |= 1;
	}
}

// 0x801A2D00
void TObjGreenRay::vfn_10()
{
	if (isInRange(this, &mPos, 2000.0f)) {
		if (mFx != 0) {
			fxResume(mFx);
		}
	} else if (mFx != 0) {
		fxPause(mFx);
	}
}
