#include "item/TItemOnce.h"
#include "engine/DrawTask.h"
#include "item/TItemManage.h"
#include "egg/EggCreature.h"
#include "animal/AniMount.h"
#include "engine/FxSystem.h"
#include "sound/Sound3DPlay.h"
#include "core/MathUtil.h"
#include "engine/ActorMotion.h"
struct Fx;

typedef char TItemOnce_size_check[(sizeof(TItemOnce) == 0x104) ? 1 : -1];

const char* gTItemOnceName = "TItemOnce";

// 0x801AFAAC
TItemOnce::TItemOnce(TObject* parent, const ItemSpec* spec)
    : TItem(parent)
    , mActive(0)
    , mState(0)
    , mTimer(0xC)
    , mSpec(*spec)
    , mHolder()
    , mGlow()
{
	_00   = (u32)gTItemOnceName;
	mSize = 0x104;
	itemHolderAttach(spec->mKind, &mHolder);
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mGlow.stop(&mHolder);
}

// 0x801AFB64
TItemOnce::~TItemOnce()
{
	removeDrawTask(mDraw);
	itemHolderRelease(&mHolder);
}

// 0x801AFBE0
void TItemOnce::draw()
{
	if (mActive != 0) {
		switch (mState) {
		case 0:
			if (--mTimer == 0) {
				mState = 1;
				mTimer = 0x1E;
				if ((int)mSpec.mType == 1) {
					mGlow.play(&mHolder, 1);
				}
			} else {
				mPos.y += gItemOnceRise;
			}
			break;
		case 1:
			if (--mTimer == 0) {
				apply(&mSpec, getOwner());
				fxSpawnAt(1, 0, 0x71, mMtx);
				sePlay3D(0x41, &mPos);
				mFlags |= 1;
			}
			break;
		}
		mtxSetTransV(mMtx, &mPos);
		mtxRotateY(mMtx, mMtx, mAngle);
		mGlow.update(&mHolder);
	}
}

// 0x801AFD08
void TItemOnce::vfn_10()
{
}

// 0x801AFD0C
void TItemOnce::render()
{
	f32 mtx[12];

	if (mActive != 0) {
		mtxFromView(mtx, mMtx);
		mGlow.draw(&mHolder, mtx);
	}
}

// 0x801AFD5C
void TItemOnce::vfn_6C()
{
	TObjAction* owner;
	Vec3f* pos;

	mActive = 1;
	owner   = getOwner();
	pos     = owner->getPos();
	mPos    = *pos;
	mPos.y += gItemOnceStartHeight;
	owner  = getOwner();
	mAngle = owner->getRotY();
	mtxSetTransV(mMtx, &mPos);
	mtxRotateY(mMtx, mMtx, mAngle);
}

// 0x801AFE00
Vec3f* TItemOnce::getPos()
{
	return &mPos;
}
