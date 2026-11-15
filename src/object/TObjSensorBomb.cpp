#include "object/TObjSensorBomb.h"
struct DrawTask;
#include "io/Resource.h"
#include "item/TItemManage.h"
#include "engine/DrawTask.h"
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "egg/ObjEgg.h"
#include "object/TObjExplode.h"
#include "object/ObjShadow.h"
#include "egg/EggCreature.h"
#include "enemy/boss/EneBoss.h"
#include "object/TObjLauncher.h"
#include "engine/DrawState.h"
#include "engine/Colli.h"
#include "camera/TCameraCtrl.h"
#include "core/MathUtil.h"
#include "ui/AeArc.h"
#include "Ninja/ninja.h"
#include "engine/ActorMotion.h"
#include "object/TObjPlayer.h"
struct DrawStateSlot;
struct NJS_OBJECT;
struct ColliContact;
struct CharaColli;
struct DrawState;

typedef char TObjSensorBomb_size_check[(sizeof(TObjSensorBomb) == 0x154) ? 1 : -1];

static void drawForOwner(void* obj);
static void drawForOthers(void* obj);

const char* gTObjSensorBombName = "TObjSensorBomb";

// 0x8018B0C8
TObjSensorBomb::TObjSensorBomb(TObject* parent)
    : TItem(parent)
    , mState(0)
    , mColli(0)
{
	_00           = (u32)gTObjSensorBombName;
	mSize         = 0x154;
	mBody.mRadius = 7.5f;
	mDrawTask     = addDrawTask(this, (void (*)(void*))drawForOwner, 33, 0);
	mShadow._30   = 15.0f;
	mShadow._34   = 15.0f;
	shadowDisable(&mShadow);
	if (itemHolderAttach(0x15, &mItem) == 0) {
		mFlags |= 1;
	} else {
		mBody.start(&gVecZero);
	}
	mModel   = (NJS_OBJECT*)*(void**)((u8*)findArcExport(gEggArc, "models") + 4);
	mTexList = *(NJS_TEXLIST**)findArcExport(gEggArc, "texlists");
}

// 0x8018B1D8
TObjSensorBomb::~TObjSensorBomb()
{
	if (mItem.mNode.mList != 0) {
		itemHolderRelease(&mItem);
	}
	removeDrawTask(mDrawTask);
	colliFree((CharaColli*)mColli);
}

// 0x8018B280
void TObjSensorBomb::place(const Vec3f* pos)
{
	mColli = colliCreate(1, 1);
	if (mColli != 0) {
		colliSetOwner(mColli, (u16)mObjId);
		colliSetMasks(mColli, 4, 0xF3, 0xF0, 0);
		colliShapeSetSphere(colliGetShape(mColli, 0));
		colliGetShape(mColli, 0)->mFlags = 0x400;
		colliGetShape(mColli, 0)->mSizeX = 7.5f;
	}
	mPos       = *pos;
	mState     = 1;
	mBody.mPos = mPos;
}

// 0x8018B368
// Keeps the id of a contact unless the shape has flag 0x10
static void noteSensorContact(void* hit, u16* id)
{
	if ((colliContactShape((ColliContact*)hit)->mFlags & 0x10) == 0) {
		*id = colliContactOwner((ColliContact*)hit);
	}
}

// 0x8018B3B0
void TObjSensorBomb::draw()
{
	if (!isInRange(this, &mPos, 400.0f)) {
		mFlags |= 1;
		return;
	}
	if (mState == 1) {
		if (mBody.mRider.mMesh != 0) {
			mState = 2;
			mTimer = 120;
		}
	} else if (mState == 2) {
		if (mTimer != 0) {
			mTimer--;
		}
		if (mTimer == 0) {
			u16 id = 0xFFFF;
			forEachContact((CharaColli*)mColli, (ContactCallback)noteSensorContact, &id);
			if (id != 0xFFFF) {
				spawnExplode(&mPos, getOwner()->mObjId, 10);
				mFlags |= 1;
			}
		}
	}

	mBody.update();
	mPos = mBody.mPos;
	if (mColli != 0) {
		colliShapeSetPos(colliGetShape(mColli, 0), &mPos);
	}

	Vec3f ground;
	if (mBody.getGround(&ground) != 0) {
		shadowSetGround(&mShadow, &ground, 0);
		shadowEnable(&mShadow);
	} else {
		shadowDisable(&mShadow);
	}
	mGlow.update(&mItem);
}

// 0x8018B528
void TObjSensorBomb::vfn_10()
{
	if (cameraIsVisible(1000.0f, 7.5f, &mPos, 0) == 0) {
		mDrawTask->mFlags |= 1;
	} else {
		mDrawTask->mFlags &= ~1;
	}
	TObjAction* owner = getOwner();
	u16 view          = playerGetDrawView();
	if (view == owner->mObjId) {
		mDrawTask->mFunc = drawForOwner;
	} else {
		mDrawTask->mFunc = drawForOthers;
	}
}

// 0x8018B5D4
static void drawForOwner(void* obj)
{
	TObjSensorBomb* bomb = (TObjSensorBomb*)obj;
	f32 mtx[12];
	mtxSetTransV(mtx, &bomb->mPos);
	mtxFromView(mtx, mtx);
	bomb->mGlow.draw(&bomb->mItem, mtx);
}

// 0x8018B624
static void drawForOthers(void* obj)
{
	TObjSensorBomb* bomb = (TObjSensorBomb*)obj;
	f32 mtx[12];
	mtxSetTransV(mtx, &bomb->mPos);
	mtxScale(1.5f, 1.5f, 1.5f, mtx, mtx);
	mtxFromView(mtx, mtx);
	DrawStateSlot* slot = getDrawStateSlot();
	njPushMatrix(mtx);
	shadowAlphaBegin();
	njSetTexture(bomb->mTexList);
	beginDrawState(slot, getDefaultDrawState());
	gjDrawObject(bomb->mModel);
	endDrawState((DrawStateSlot*)slot);
	shadowAlphaEnd();
	njPopMatrixEx();
}

// 0x8018B6C4
Vec3f* TObjSensorBomb::getPos()
{
	return &mPos;
}
