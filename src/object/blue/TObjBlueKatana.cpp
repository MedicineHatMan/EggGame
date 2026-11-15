#include "object/blue/TObjBlueKatana.h"
struct DrawTask;
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "io/ResBank.h"
#include "engine/FrameTex.h"
#include "object/TObjLauncher.h"
#include "engine/DrawState.h"
#include "egg/EggCreature.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "engine/Colli.h"
#include "camera/TCameraCtrl.h"
#include "Ninja/ninja.h"
#include "ui/AeArc.h"
#include "engine/TParticle.h"
#include "stage/SetSlot.h"
#include "egg/TMorphEggAnim.h"
struct CharaColli;
struct ColliShape;
struct NJS_OBJECT;
struct DrawStateSlot;

extern "C" {

const char* gTObjBlueKatanaName = "TObjBlueKatana";

extern const ColliShape sBlueKatanaShape[] = {
	{ { 0.0f, 10.0f, 0.0f }, 0x0, 0, 0x0, 5.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

typedef char TObjBlueKatana_size_check[(sizeof(TObjBlueKatana) == 0xA4) ? 1 : -1];

// 0x8002DE24
extern "C" void initBlueKatana()
{
	resBankLoad(&sBlueKatanaBank, "ar_obj_blue_katana.arc");
	resBankAddTextures(&sBlueKatanaBank, 0, "obj_blue_katana.gvm");
	sBlueKatanaModels = resBankGetModels(&sBlueKatanaBank);
	sBlueKatanaTextures = resBankGetTextures(&sBlueKatanaBank);
}

// 0x8002DE8C
extern "C" void termBlueKatana()
{
	resBankRemoveTextures(&sBlueKatanaBank, 0);
	resBankRelease(&sBlueKatanaBank);
	sBlueKatanaModels = 0;
	sBlueKatanaTextures = 0;
}

// 0x8002DED0
extern "C" TObject* createTObjBlueKatana(TObject* parent, SetDesc* desc)
{
	return new TObjBlueKatana(parent, desc);
}

// 0x8002DF24
TObjBlueKatana::TObjBlueKatana(TObject* parent, SetDesc* desc)
    : TSetObj(parent, desc)
{
	_00        = (u32)gTObjBlueKatanaName;
	mSize      = 0xA4;
	initFromDesc(desc);
	mAct       = 0;
	mRecord3E  = (u8)((SetDescObj*)desc)->get3E();
	mDrawTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mDrawState = getDefaultDrawState();
	_80        = 0;
	_80        = (int)colliCreate(1, 1);
	colliCopyShapes((CharaColli*)_80, (const ColliShape*)sBlueKatanaShape);
	colliSetOwner((CharaColli*)_80, (u16)mObjId);
	colliSetMasks((CharaColli*)_80, 4, 255, 0, 0);
	colliPlaceShapesRotY((CharaColli*)_80, &mTrans.mPos, mTrans.mRot.y, (const ColliShape*)sBlueKatanaShape);
	setLinkTimed(mLink.get(), 0x708);
}

// 0x8002E034
TObjBlueKatana::~TObjBlueKatana()
{
	colliRetire((CharaColli*)_80);
	_80 = 0;
	removeDrawTask(mDrawTask);
	mDrawTask = 0;
}

// 0x8002E0B4
void TObjBlueKatana::take()
{
	ColliContact* hit = ((CharaColli*)_80)->mContacts;
	while (hit != 0) {
		if (colliContactHasBit40(hit) != 0) {
			mEffect    = new TMorphEggAnim(this, getPos(), 1.0f);
			mDrawState = &mEffect->mState;
			mHitId     = colliContactOwner(hit);
			mAct       = 1;
			return;
		}
		hit = hit->mNext;
	}
}

// 0x8002E16C
void TObjBlueKatana::stateWait()
{
	take();
}

// 0x8002E18C
void TObjBlueKatana::stateHeld()
{
	if ((mEffect->mScript.mDone & 1) != 0) {
		Vec3f at;
		at.x = mTrans.mPos.x;
		at.y = mTrans.mPos.y;
		at.z = mTrans.mPos.z;
		at.y += 10.0f;
		particleOnceByIdDefault(&at, 0, 0x142);
		mFlags |= 1;
	}
}

// 0x8002E204
void TObjBlueKatana::draw()
{
	if (objDebugEnabled(this) != 0) {
		frameTexRequestCur();
		return;
	}
	if (isInRange(this, &mTrans.mPos, 500.0f) == 0) {
		mFlags |= 1;
		return;
	}
	frameTexRequestCur();
	switch (mAct) {
	case 0:
			stateWait();
			break;
	case 1:
			stateHeld();
			break;
	}
}

// 0x8002E29C
void TObjBlueKatana::vfn_10()
{
	if (cameraIsVisible(500.0f, 20.0f, &mTrans.mPos, 0) != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x8002E304
void TObjBlueKatana::render()
{
	DrawStateSlot* slot = getDrawStateSlot();
	beginDrawState(slot, mDrawState);
	njPushMatrixEx();
	njTranslateEx(&mTrans.mPos);
	gjRotateX(mTrans.mRot.x);
	gjRotateY(mTrans.mRot.y);
	njSetTexture(sBlueKatanaTextures[0]);
	gjDrawObject(sBlueKatanaModels[1]);
	frameTexSetCur();
	_nj_control_3d_flag_ |= 0x100000;
	gjDrawObject(sBlueKatanaModels[0]);
	_nj_control_3d_flag_ &= ~0x100000;
	njPopMatrixEx();
	endDrawState(slot);
}
