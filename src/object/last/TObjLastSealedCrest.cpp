#include "stage/SetManager.h"
#include "stage/SetActor.h"
#include "engine/DrawTask.h"
#include "engine/ModelDraw.h"
#include "object/ObjModel.h"
#include "object/ObjArchive.h"
#include "engine/Light.h"
#include "enemy/boss/TEneLastBossDarkCtrl.h"
#include "engine/CharaColli.h"
#include "engine/Color.h"
#include "stage/EventScript.h"
#include "object/ObjAction.h"
#include "enemy/boss/TEneLastBoss.h"
#include "Ninja/ninja.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "core/MathUtil.h"
#include "engine/FxSystem.h"
#include "camera/TCameraCtrl.h"
#include "engine/Colli.h"
#include "object/TObjKey00.h"
#include "sound/SoundCtrl.h"
#include "core/VecMath.h"
#include "core/TSceneManager.h"
#include "object/last/TObjLastSealedCrest.h"
struct CharaColli;
struct SceneRecord;
struct ColliShape;
struct Fx;

extern "C" {

}

const f32 gLastSealedCrestSetRange = 700.0f;

// 0x8019A11C
// The set-file init for this class
extern "C" void initTObjLastSealedCrest()
{
	initLastBossDark();
}

// 0x8019A13C
// The set-file shutdown for this class
extern "C" void shutdownTObjLastSealedCrest()
{
	shutdownLastBossDark();
}

typedef char TObjLastSealedCrest_size_check[(sizeof(TObjLastSealedCrest) == 0x174) ? 1 : -1];

extern "C" TObject* createTObjLastSealedCrest(TObject* parent, SetDesc* desc)
{
	createLastBossDark();
	Vec3f pos;
	pos.set(1016.1500244140625f, 231.5f, 775.1599731445312f);
	darkCtrlSetPos(&pos);
	darkCtrlSetFx(8, 4, 10);
	darkCtrlSetSe(0x288);
	return new TObjLastSealedCrest(parent, desc);
}

const char* gTObjLastSealedCrestName = "TObjLastSealedCrest";

// 0x8019A1EC
TObjLastSealedCrest::TObjLastSealedCrest(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
    , mDarkMap(0)
    , mTimer(30)
    , mFx(0)
{
	mPulse = 0;
	_00   = (u32)gTObjLastSealedCrestName;
	mSize = 0x174;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 4, 0xFF, 0xFF, 0xF6);
	initFromDesc(desc);
	arm();
}

// 0x8019A2E0
TObjLastSealedCrest::~TObjLastSealedCrest()
{
	fxStop((Fx*)mFx);
	darkCtrlClearMap(mDarkMap);
	if (mLinked != 0) {
		void* holder = getSceneRecord();
		((SceneRecord*)holder)->mLights->remove(&mGlow);
	}
	colliRetire(mShape);
	mShape = 0;
	removeDrawTask(mDraw);
}

// 0x8019A3B8
void TObjLastSealedCrest::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mPos.y += 0.1f;
	mtxSetTransV(mMtx, &mPos);
	mtxRotateY(mMtx, mMtx, mRot.y);
	colliShapeMakeCylinder(colliGetShapes(mShape), &mPos, 1, 100.0f, 50.0f);
	*(u32*)((u8*)colliGetShapes(mShape) + 0x24) |= 0x10;
	int id;
	if (desc->getInt(0) < 0) {
		id = 0;
	} else if (desc->getInt(0) > 0x800) {
		id = 0x800;
	} else {
		id = desc->getInt(0);
	}
	mSwitchId = (u16)id;
	mEvent    = desc->getInt(1);
}

// 0x8019A4CC
void TObjLastSealedCrest::debugPrint(void*)
{
	njPrintC(0xA0013, ":SWITCH ID");
	njPrintC(0xA0014, ":CALL EVENT ID");
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, "");
	njPrintC(0xA0013, "");
	njPrintC(0xA0014, "");
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, "");
}

// 0x8019A570
void TObjLastSealedCrest::arm()
{
	mLinked              = 0;
	mGlow.mFlags  = 2;
	mGlow.mPos    = mPos;
	mGlow.mKa[0]  = 1.0f;
	mGlow.mKa[1]  = 0.0010000000474974513f;
	mGlow.mKa[2]  = 0.0010000000474974513f;
	mGlow.mColor  = 0xFFFF00FF;
	mMesh.setModel(*getGeObjModels());
	mMesh.mDrawState = &mState;
	mMesh.mTexList   = *getGeObjTextures();
	mMesh.mMatrix    = mMtx;
	if (keyBitIsSet(mSwitchId) != 0) {
		breakOpen();
		mMode = 2;
	} else {
		mState.mFlags   = 0x200;
		mState.mAmbient = 0xFFFFFF20;
		mDarkMap          = darkCtrlAddMap(80.0f, &mPos, 300);
		mMode           = 0;
	}
}

// 0x8019A660
// The broken seal's effect once
void TObjLastSealedCrest::breakOpen()
{
	if (mFx == 0) {
		f32 mtx[12];
		mtxTranslate((float*)mtx, (const float*)mMtx, 0.0f, 0.1f, 0.0f);
		mFx = fxSpawnAt(1, 8, 0xC, mtx);
	}
}

// 0x8019A6C4
void TObjLastSealedCrest::draw()
{
	switch (mMode) {
	case 0:
		stateSealed();
		break;
	case 1:
		stateOpening();
		break;
	case 2:
		stateOpen();
		break;
	}
}

// 0x8019A71C
// Breaks once the dark map is gone
void TObjLastSealedCrest::stateSealed()
{
	if (darkCtrlHasMap(mDarkMap) == 0) {
		darkCtrlClearMap(mDarkMap);
		breakOpen();
		sePlay(0x289);
		mState.mFlags = 0;
		mDarkMap      = 0;
		mMode         = 1;
	}
}

// 0x8019A780
void TObjLastSealedCrest::stateOpening()
{
	if (mTimer == 0) {
		setKeyBit(mSwitchId);
		eventScriptRequest(mEvent);
		mMode = 2;
	}
	mTimer--;
}

// 0x8019A7D8
// Glows circling while something stands on it
void TObjLastSealedCrest::stateOpen()
{
	CharaColli* colli = (CharaColli*)mShape;
	if (colli->mContacts != 0) {
		for (ColliContact* contact = colli->mContacts; contact != 0; contact = contact->mNext) {
			if (colliContactNoBit10(contact) != 0) {
				if (mLinked == 0) {
					getSceneRecord()->mLights->add(&mGlow);
					mLinked = 1;
				}
				Vec3f at;
				vecFromAngleXZ(15.0f, &at, mPulse);
				vecAdd(&at, &at, &mPos);
				at.y -= 15.0f;
				mGlow.mPos.x = at.x;
				mGlow.mPos.y = at.y;
				mGlow.mPos.z = at.z;
				u32 color;
				u32 dim;
				u32 bright;
				bright = 0xFFFF00FF;
				dim    = 0x808000FF;
				lerpColor(0.5f * (1.0f + njSin(mPulse << 2)), &color, &dim, &bright);
				mGlow.mColor = color;
				mPulse += 0x400;
				return;
			}
		}
	}
	if (mLinked != 0) {
		getSceneRecord()->mLights->remove(&mGlow);
		mLinked = 0;
	}
}

// 0x8019A920
void TObjLastSealedCrest::vfn_10()
{
	Vec3f view;
	if (cameraIsVisible(750.0f, 80.0f, &mPos, &view) != 0) {
		mDraw->mSortKey = drawTaskDepthKey(view.z);
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x8019A998
void TObjLastSealedCrest::render()
{
	mMesh.draw();
}
