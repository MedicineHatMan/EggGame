#include "object/TObjBridgeIta.h"
#include "camera/TCamera.h"
#include "object/ObjScript.h"
#include "object/ObjModel.h"
#include "engine/DrawTask.h"
#include "enemy/EnemyLight.h"
#include "object/ObjCommon.h"
#include "object/TObjLauncher.h"
#include "engine/DrawState.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "ui/AeArc.h"
struct NJS_QUATERNION;
struct NJS_OBJECT;
struct DrawStateSlot;

extern "C" {
// 0x802C40D0
ObjClassName gTObjBridgeItaName = { "TObjBridgeIta", 0 };
}

typedef char TObjBridgeIta_size_check[(sizeof(TObjBridgeIta) == 0x154) ? 1 : -1];

// 0x800D4DAC
// At the origin flat with the default plank size
void TObjBridgeIta::init()
{
	mPos.x     = 0.0f;
	mPos.y     = 0.0f;
	mPos.z     = 0.0f;
	mPrevPos.x = 0.0f;
	mPrevPos.y = 0.0f;
	mPrevPos.z = 0.0f;
	mTilt      = 0;
	mRotY      = 0;
	_A4        = 0;
	mWidth     = 50.0f;
	mScale     = 1.0f;
	mDepth     = 40.0f;
}

// 0x800D4DF4
TObjBridgeIta::TObjBridgeIta(TObject* parent)
    : TObjAction(parent)
{
	_00   = (u32)gTObjBridgeItaName.mName;
	mSize = 0x154;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	init();
}

// 0x800D4E90
TObjBridgeIta::~TObjBridgeIta()
{
	removeDrawTask(mDraw);
}

// 0x800D4F1C
void TObjBridgeIta::setPlankKind(int kind)
{
	switch (kind) {
	case 0:
		mPlankModel = (NJS_OBJECT*)findGeObjData("MODEL_TURIBASHI_ITA01");
		mPlankTex   = findGeObjData("TLS_MODEL_TURIBASHI_ITA01");
		break;
	case 1:
		mPlankModel = (NJS_OBJECT*)findGeObjData("MODEL_TURIBASHI_ITA02");
		mPlankTex   = findGeObjData("TLS_MODEL_TURIBASHI_ITA02");
		break;
	case 2:
		mPlankModel = (NJS_OBJECT*)findGeObjData("MODEL_TURIBASHI_ITA03");
		mPlankTex   = findGeObjData("TLS_MODEL_TURIBASHI_ITA03");
		break;
	}
}

// 0x800D4FC0
void TObjBridgeIta::setRope(int rope)
{
	if (rope != 0) {
		mSideModel = (NJS_OBJECT*)findGeObjData("MODEL_TURIBASHI_ROPE");
		mSideTex   = findGeObjData("TLS_MODEL_TURIBASHI_ROPE");
	} else {
		mSideModel = (NJS_OBJECT*)findGeObjData("MODEL_TURIBASHI_HASHIRA01");
		mSideTex   = findGeObjData("TLS_MODEL_TURIBASHI_HASHIRA01");
	}
}

// 0x800D502C
void TObjBridgeIta::draw()
{
	mPlace.mPos.x = mPrevPos.x;
	mPlace.mPos.y = mPrevPos.y;
	mPlace.mPos.z = mPrevPos.z;
	njXYZAngleToQuaternion((const s32*)&mTilt, (NJS_QUATERNION*)mPlace.mRot);
	mPlace.update();
}

// 0x800D5080
void TObjBridgeIta::vfn_10()
{
	if (mLight.isVisible(mDraw, 800.0f, getPos(), mDepth, 0) != 0) {
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x800D5100
void TObjBridgeIta::render()
{
	DrawStateSlot* slot = getDrawStateSlot();
	ObjDrawState buf;

	beginDrawState(slot, mLight.drawState(&buf));
	njPushMatrixEx();
	njTranslateEx(&mPrevPos);
	gjRotateY(mRotY);
	gjRotateX(mTilt);
	njSetTexture(mPlankTex);
	gjDrawObject(mPlankModel);
	njSetTexture(mSideTex);
	gjDrawObject(mSideModel);
	njPopMatrixEx();
	endDrawState(slot);
}

// 0x800D519C
Vec3f* TObjBridgeIta::getPos() { return &mPrevPos; }

// 0x800D51A4
void TObjBridgeIta::setPos(const Vec3f* pos)
{
	mPrevPos.x = pos->x;
	mPrevPos.y = pos->y;
	mPrevPos.z = pos->z;
}

// 0x800D51C0
int TObjBridgeIta::getKind() { return 3; }

// 0x800D51C8
int TObjBridgeIta::getRotY() { return mRotY; }
