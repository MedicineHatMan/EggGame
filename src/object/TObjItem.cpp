#include "object/TObjPlayer.h"
#include "object/ObjAction.h"
#include "item/TItemManage.h"
#include "object/ObjCommon.h"
#include "engine/DrawTask.h"
#include "engine/CharaColli.h"
#include "engine/FxSystem.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "item/ItemParts.h"
#include "object/TObjItem.h"
#include "camera/TCamZelda5.h"
#include "camera/TCamera.h"
#include "Ninja/ninja.h"
#include "core/TMainTask.h"
#include "engine/ActorMotion.h"
#include "camera/TCameraCtrl.h"
#include "sound/SoundCtrl.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
struct CharaColli;
struct Fx;

/** An item's message to what touches it 0x19 here is this item */
struct ItemOfferMsg : public ObjMsg {
	ItemSpec mSpec; // _04
	BOOL mOffer;    // _0C
	BOOL mTaken;    // _10
};

typedef char TObjItem_size_check[(sizeof(TObjItem) == 0x194) ? 1 : -1];

// 0x800625D8
// An item dropped by owner it falls where it is
extern "C" void spawnItemDrop(u16 owner, int kind, Vec3f* pos)
{
	TObjItem* item = new TObjItem(gTL_04, kind, pos);
	if (item != 0) {
		item->mHolderId = owner;
		item->setMode(TObjItem::ITEM_DROP);
	}
}

// 0x80062648
// The same thrown the player uses it to let go of what it holds
extern "C" void spawnItemThrown(u16 owner, int kind, Vec3f* pos)
{
	TObjItem* item = new TObjItem(gTL_04, kind, pos);
	if (item != 0) {
		item->mHolderId = owner;
		item->setMode(TObjItem::ITEM_THROWN);
	}
}

const char* gTObjItemName = "TObjItem";

// 0x800626B8
TObjItem::TObjItem(TObject* parent, int kind, const Vec3f* pos)
    : TObjAction(parent), mMode(ITEM_NONE), mFall(5), mLife(0x708), mAngle(0.0f), mSpin(0.0f), mLift(5.0f)
{
	ColliShape* shape;

	_00      = (u32)gTObjItemName;
	mSize    = 0x194;
	mPos.x   = pos->x;
	mPos.y   = pos->y + 5.0f;
	mPos.z   = pos->z;
	mVel.x   = 0.0f;
	mVel.y   = 0.0f;
	mVel.z   = 0.0f;
	mBody.mRadius = 5.0f;
	setObjId(this, 0xFFFF);
	mShape = colliCreate(1, 1);
	if (mShape != 0) {
		colliSetOwner(mShape, (u16)mObjId);
		colliSetMasks((CharaColli*)mShape, 0x10, 0, 0, 0xFE);
		shape = colliGetShape((CharaColli*)mShape, 0);
		colliShapeSetSphere((ColliShape*)shape);
		colliGetShape((CharaColli*)mShape, 0)->mFlags = 0x10;
		shape          = colliGetShape((CharaColli*)mShape, 0);
		colliShapeSetPos(shape, pos);
		colliGetShape((CharaColli*)mShape, 0)->mSizeX = 5.0f;
	}
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mtxSetTransV(mMtx, pos);
	if (itemHolderAttach(kind, &mItem) == 0) {
		mFlags |= 1;
	}
}

// 0x80062874
TObjItem::~TObjItem()
{
	runMode(mMode, ITEM_EXIT);
	if (mItem.mNode.mList != 0) {
		itemHolderRelease(&mItem);
	}
	colliFree((CharaColli*)mShape);
	removeDrawTask(mDraw);
	unregisterId();
}

// 0x80062928
void TObjItem::draw()
{
	runMode(mMode, ITEM_CHECK);
	runMode(mMode, ITEM_UPDATE);
	if (mLife != 0) {
		if (--mLife == 0) {
			mFlags |= 1;
		}
	}
	mGlow.update(&mItem);
	mtxSetTransV(mMtx, &mPos);
	mtxRotateY(mMtx, mMtx, (int)(mAngle * 10430.38043493439));
	mGlow.calc(&mItem, mMtx);
	updateFall();
	mBody.update();
	if (mFade != 0) {
		mSpin = mSpin + (0.034906577f - mSpin) / (f32)mFade;
		mFade = (u16)(mFade - 1);
	}
	mAngle = mAngle + mSpin;
	if (mShape != 0) {
		colliShapeSetPos(colliGetShape(mShape, 0), &mPos);
	}
}

// 0x80062A6C
void TObjItem::updateFall()
{
	switch (mFall) {
	case FALL_RISE:
		if (mTimer != 0) {
			mPos.y = mPos.y + (mRiseTo - mPos.y) / (f32)mTimer;
			if (--mTimer == 0) {
				mTimer = 30;
				mFall  = FALL_HOVER;
			}
		}
		break;
	case FALL_HOVER:
		if (mTimer != 0 && --mTimer == 0) {
			const ItemSpec* spec = findItemSpec(mItem.firstId());
			if (spec->mFlags & 0x80) {
				ItemUser* player = (ItemUser*)findPlayer(mHolderId);
				if (player != 0) {
					player->useSpec(spec);
				}
				mFlags |= 1;
			} else {
				bounce();
				mFall = FALL_DROP;
			}
		}
		break;
	case FALL_DROP:
		mPos = mBody.mPos;
		if (mBody.mRider.mMesh != 0) {
			mFall = FALL_BOB;
		}
		break;
	case FALL_BOB:
		if (mBody.mRider.mMesh == 0) {
			bounce();
			mFall = FALL_DROP;
		} else {
			f32 y = mBody.mPos.y;
			f32 z = mBody.mPos.z;
			mPos.x = mBody.mPos.x;
			mPos.y = -2.0f * njSin(mTimer) + y;
			mPos.z = z;
			mTimer += 0x200;
		}
		break;
	case FALL_FLUNG:
		mPos = mBody.mPos;
		if (mBody.mVel.y < 0.0f) {
			bounce();
			mFall = FALL_DROP;
		}
		break;
	}
}

// 0x80062C9C
void TObjItem::bounce()
{
	Vec3f vel;

	vel.x         = 0.0f;
	vel.y         = -10.0f;
	vel.z         = 0.0f;
	mBody.mPos    = mPos;
	mBody.mRadius = 10.0f;
	mBody.launch(&vel);
}

// 0x80062CFC
void TObjItem::runMode(int mode, int event)
{
	switch (mode) {
	case ITEM_DROP:
		modeDrop(event);
		break;
	case ITEM_IDLE:
		modeIdle(event);
		break;
	case ITEM_THROWN:
		modeThrown(event);
		break;
	}
}

// 0x80062D5C
void TObjItem::setMode(int mode)
{
	if (mMode != -1) {
		runMode(mMode, ITEM_EXIT);
	}
	if (mode != -1) {
		runMode(mode, ITEM_ENTER);
	}
	mMode = mode;
}

// 0x80062DBC
void TObjItem::vfn_10()
{
	if (cameraIsVisible(1000.0f, 10.0f, getPos(), 0)) {
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x80062E30
void TObjItem::render()
{
	f32 mtx[12];

	mtxFromView(mtx, mMtx);
	mGlow.draw(&mItem, mtx);
}

// 0x80062E74
// Offers it to the owner of each contact that isn't a pass-through shape until one takes it
static void itemOfferContact(ColliContact* contact, ItemOfferMsg* msg, int* stop)
{
	if (!(colliContactShape(contact)->mFlags & 0x10)) {
		if (sendMsgById(colliContactOwner(contact), msg)) {
			msg->mTaken = TRUE;
			*stop       = 1;
		}
	}
}

// 0x80062ED8
BOOL TObjItem::offer()
{
	ItemOfferMsg msg;
	f32 mtx[12];

	msg.mOffer = TRUE;
	msg.mType  = 0x19;
	msg.mSpec  = *findItemSpec(mItem.firstId());
	msg.mTaken = FALSE;
	forEachContact((CharaColli*)mShape, (ContactCallback)itemOfferContact, &msg);
	if (msg.mTaken && msg.mOffer) {
		sePlay(0x48);
		mtxSetTransV(mtx, &mPos);
		fxSpawnAt(1, 7, 0x67, mtx);
	}
	return msg.mTaken;
}

// 0x80062FBC
// Rises out of what dropped it with a puff spinning fast
void TObjItem::modeDrop(int event)
{
	if (event == ITEM_ENTER) {
		f32 mtx[12];
		colliDisable(mShape);
		mDelay  = 0x3C;
		mFade   = 100;
		mSpin   = 0.34906578f;
		mFall   = FALL_RISE;
		mRiseTo = 20.0f + mPos.y;
		mTimer  = 20;
		mtxSetTransV(mtx, &mPos);
		fxSpawnAt(1, 7, 0x64, mtx);
	} else if (event == ITEM_CHECK) {
		if (mDelay == 0) {
			setMode(ITEM_IDLE);
		}
	} else if (event == ITEM_UPDATE) {
		if (mDelay != 0) {
			mDelay--;
		}
	}
}

// 0x8006308C
// Can be picked up sparkles while it bobs
void TObjItem::modeIdle(int event)
{
	if (event == ITEM_ENTER) {
		colliEnable(mShape);
		mShine = 0;
	} else if (event == ITEM_EXIT) {
		fxStop(mShine);
	} else if (event == ITEM_CHECK) {
		if (offer()) {
			mFlags |= 1;
		}
	} else if (event == ITEM_UPDATE) {
		if (mShine == 0 && mFall == FALL_BOB) {
			mShine = fxSpawnFollow(1, 7, 0x60, mShineMtx);
		} else if (mShine != 0 && mFall != FALL_BOB) {
			fxStop(mShine);
			mShine = 0;
		}
		if (mShine != 0) {
			mtxSetTrans(mShineMtx, mBody.mPos.x, 0.01f + (mBody.mPos.y - 10.0f), mBody.mPos.z);
		}
	}
}

// 0x80063194
// Flung up and out a random way
void TObjItem::modeThrown(int event)
{
	if (event == ITEM_ENTER) {
		Vec3f vel;
		vecFromAngleXZ(50.0f, &vel, (int)(65536.0f * (3.0517578e-5f * rand())));
		vel.y = 150.0f;
		mBody.start(&vel);
		mBody.mPos = mPos;
		mFall      = FALL_FLUNG;
	} else if (event == ITEM_CHECK) {
		if (mFall == FALL_BOB) {
			setMode(ITEM_IDLE);
		}
	}
}

// 0x80063260
Vec3f* TObjItem::getPos()
{
	return &mPos;
}

// 0x80063268
int TObjItem::getKind()
{
	return 3;
}

// The main camera's hooks and factory here before TCamZelda5.cpp

// 0x80063270
extern "C" void camZeldaInit()
{
}

// 0x80063274
extern "C" void camZeldaTerm()
{
}

// 0x80063278
extern "C" TCamera* createTCamZelda5(int player, int look)
{
	return new TCamZelda5(gTL_01, player, look);
}
