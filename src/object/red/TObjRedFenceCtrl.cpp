#include "object/red/TObjRedFenceCtrl.h"
#include "engine/DrawTask.h"
#include "object/red/TObjRedFence.h"
#include "egg/ObjEgg.h"
#include "object/ObjArchive.h"
#include "stage/StageObjUtil.h"
#include "enemy/boss/EneBoss.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "core/MathUtil.h"
#include "engine/GroundQuery.h"
#include "engine/TParticle.h"
#include "sound/Sound3DPlay.h"
#include "stage/SetSlot.h"
#include "engine/WorldMesh.h"
#include "engine/MeshPool.h"
#include "object/TObjKey00.h"
struct MeshData;
struct MeshObject;
struct MeshColli;
struct SetLink;

// 0x800CF420
// is the console archive load not this class

typedef char TObjRedFenceCtrl_size_check[(sizeof(TObjRedFenceCtrl) == 0xB0) ? 1 : -1];

static f32 bitsToFloat(u32 bits)
{
	union {
		unsigned long long u;
		double d;
	} pun;
	pun.u = ((unsigned long long)0x43300000 << 32) | (bits ^ 0x80000000);
	return (f32)(pun.d - 4503601774854144.0);
}

static Vec3f zeroOff()
{
	Vec3f off;
	off.x = 0.0f;
	off.y = 0.0f;
	off.z = 0.0f;
	return off;
}

// 0x800CE3AC
extern "C" TObject* createTObjRedFenceCtrl(TObject* parent, SetDesc* desc)
{
	return new TObjRedFenceCtrl(parent, desc);
}

// 0x800CE400
void TObjRedFenceCtrl::initFromDesc(SetDesc* desc)
{
	desc->getPos(&mHome);
	desc->getPos(&mTrans.mPos);
	desc->getRot(&mTrans.mRot);
	mSwitch = (u16)desc->getInt(0);
	mOpenId = (u16)desc->getInt(1);
}

// 0x800CE4AC
void TObjRedFenceCtrl::debugPrint(void*)
{
	njPrintC(0xA0013, ":SET ID");
	njPrintC(0xA0014, ":OPEN ID");
}

const char* gTObjRedFenceCtrlName = "TObjRedFenceCtrl";

// 0x800CE4EC
TObjRedFenceCtrl::TObjRedFenceCtrl(TObject* parent, const SetDesc* desc)
    : TObjAction(parent)
{
	_00   = (u32)gTObjRedFenceCtrlName;
	mSize = 0xB0;
	setObjId(this, desc->mObjId);
	initFromDesc((SetDesc*)desc);
	mLink.get()->attach((SetDesc*)desc);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mLand     = 0;
	build();
}

// 0x800CE5A8
TObjRedFenceCtrl::~TObjRedFenceCtrl()
{
	if (mLand != 0) {
		meshColliFree(mLand);
	}
	notifySetLinkDeath(mLink.get(), this);
	removeDrawTask(mDrawTask);
	releaseObjId(this);
}

// 0x800CE668
void TObjRedFenceCtrl::build()
{
	mState = 0;
	mTrans.mGravity.set(0.0f, -0.16333334f, 0.0f);
	mExtra = -1.0f;
	mAngle = 0;
	mSwing = 0x800;
	mRate  = 0;
	setLinkDone((SetLink*)&mLink);
	for (int i = 0; i < 4; i++) {
		new TObjRedFence(this);
	}
	Vec3f off;
	off.x = 0.0f;
	off.y = 0.0f;
	off.z = 0.0f;
	place(&off);

	MeshData* data = (MeshData*)gGeObjCommon->mSet.mMeshes[1];
	f32 mtx[12];
	mtxIdentity(mtx);
	mtxTranslateV(mtx, mtx, &mTrans.mPos);
	mtxRotateZYX((f32*)mtx, (const f32*)mtx, (const int*)&mTrans.mRot);
	mLand = (MeshObject*)meshColliCreate();
	meshSetFlag2(mLand, 1);
	meshColliSetData(mLand, data);
	meshColliSetMatrix(mLand, (const f32*)mtx);
	fitBox(mtx);
}

// 0x800CE780
void TObjRedFenceCtrl::fitBox(f32* mtx)
{
	f32 box[6];
	Vec3f lo;
	Vec3f hi;
	lo.x = -200.0f;
	lo.y = 0.0f;
	lo.z = -200.0f;
	hi.x = 200.0f;
	hi.y = 100.0f;
	hi.z = 200.0f;
	boxAroundBox(box, mtx, &lo, &hi);
	meshColliSetBox(mLand, (const f32*)box);
}

// 0x800CE7EC
void TObjRedFenceCtrl::watch()
{
	if (keyBitIsSet(mSwitch) == 1) {
		mState = 1;
	}
}

// 0x800CE82C
void TObjRedFenceCtrl::fall()
{
	EggHits hits;
	mTrans.vfn_14();
	Vec3f at;
	at = mTrans.mPos;
	at.y += 1.0f;
	if (meshColliProbe(hits.mHits, (const Vec3f*)&at, (unsigned long)((-1))) != 0) {
		f32 drop = hits.mHits[0].mPos.y - mTrans.mPos.y;
		if (!(drop < mTrans.mVel.y)) {
			mTrans.mVel.y       = drop;
			TObjRedFence* fence = (TObjRedFence*)mChild;
			for (u16 i = 0; i < 4; i++) {
				fence->startFx();
				for (u16 p = 0; p < 3; p++) {
					Vec3f spark;
					fence->pointOff(&spark, p);
					particleOnceById(this, &spark, 1, 0xB);
				}
				sePlay3D(0x225, fence->getPos());
				fence = (TObjRedFence*)fence->mNext;
			}
			mState = 2;
		}
	}
	mTrans.vfn_18();
	mTrans.vfn_0C();
	Vec3f off;
	off.x = 0.0f;
	off.y = 0.0f;
	off.z = 0.0f;
	place(&off);
	f32 mtx[12];
	mtxIdentity(mtx);
	mtxTranslateV(mtx, mtx, &mTrans.mPos);
	mtxRotateZYX((f32*)mtx, (const f32*)mtx, (const int*)&mTrans.mRot);
	meshColliSetMatrix(mLand, (const f32*)mtx);
	fitBox(mtx);
}

// 0x800CE9EC
void TObjRedFenceCtrl::swing()
{
	if (mSwing > 0) {
		mAngle = (int)((f32)mSwing * njSin(mRate));
		mSwing -= 0x40;
		mRate += 0x1800;
		mTrans.mRot.x = mAngle;
		Vec3f off;
		off.x = 0.0f;
		off.y = 0.0f;
		off.z = 0.0f;
		place(&off);
	} else {
		mAngle        = 0;
		mTrans.mRot.x = mAngle;
	}
	if (keyBitIsSet(mOpenId) == 1) {
		mTrans.mVel.x = 0.0f;
		mTrans.mVel.y = 0.0f;
		mTrans.mVel.z = 0.0f;
		mSwing        = 0x2800;
		mRate         = 10;
		meshColliRefresh(mLand);
		TObjRedFence* fence = (TObjRedFence*)mChild;
		for (u16 i = 0; i < 4; i++) {
			fence->stopFx();
			sePlay3D(0x226, fence->getPos());
			fence = (TObjRedFence*)fence->mNext;
		}
		mState = 3;
	}
}

// 0x800CEB34
void TObjRedFenceCtrl::open()
{
	Vec3f off;
	off.x     = 0.0f;
	off.y     = 0.0f;
	off.z     = 0.0f;
	int angle = mAngle;
	if (angle < 0x4000) {
		mAngle += mRate;
		mRate *= 1.1f;
		mTrans.mRot.x = -mAngle;
	} else if (angle > 0x4000) {
		mFlags |= 1;
	}
	place(&off);
}

// 0x800CEBD8
static inline Vec3f fenceAdd(const Vec3f& a, const Vec3f& b)
{
	Vec3f r;
	gjAddVector(&r, &a, &b);
	return r;
}

void TObjRedFenceCtrl::place(const Vec3f* off)
{
	TObjRedFence* fence;
	Vec3f local;
	SetRot rot;

	local.x = off->x;
	local.y = off->y;
	local.z = off->z;
	local.z = off->z - 150.0f;
	rot     = mTrans.mRot;
	rot.y   = mTrans.mRot.y;
	fence   = (TObjRedFence*)mChild;
	{
		Vec3f placed = fenceAdd(local, mTrans.mPos);
		fence->setPos(&placed);
	}
	fence->mRot = rot;

	local.x = off->x;
	local.y = off->y;
	local.z = 150.0f + off->z;
	rot     = mTrans.mRot;
	rot.y   = mTrans.mRot.y + 0x8000;
	fence   = (TObjRedFence*)fence->mNext;
	{
		Vec3f placed = fenceAdd(local, mTrans.mPos);
		fence->setPos(&placed);
	}
	fence->mRot = rot;

	local.x = off->z - 150.0f;
	local.y = off->y;
	local.z = off->x;
	rot     = mTrans.mRot;
	rot.y   = mTrans.mRot.y + 0x4000;
	fence   = (TObjRedFence*)fence->mNext;
	{
		Vec3f placed = fenceAdd(local, mTrans.mPos);
		fence->setPos(&placed);
	}
	fence->mRot = rot;

	local.x = 150.0f + off->z;
	local.y = off->y;
	local.z = off->x;
	rot     = mTrans.mRot;
	rot.y   = mTrans.mRot.y + 0xC000;
	fence   = (TObjRedFence*)fence->mNext;
	{
		Vec3f placed = fenceAdd(local, mTrans.mPos);
		fence->setPos(&placed);
	}
	fence->mRot = rot;
}

// 0x800CEE7C
void TObjRedFenceCtrl::draw()
{
	if (objDebugEnabled(this) != 0) {
		Vec3f off;
		off.x = 0.0f;
		off.y = 0.0f;
		off.z = 0.0f;
		place(&off);
		return;
	}
	if (isInRange(this, &mTrans.mPos, 800.0f) == 0) {
		if (mState != 3) {
			setLinkFree((SetLink*)&mLink);
		}
		mFlags |= 1;
		return;
	}
	switch (mState) {
	case 0:
		watch();
		break;
	case 1:
		fall();
		break;
	case 2:
		swing();
		break;
	case 3:
		open();
		break;
	}
}

// 0x800CEF64
void TObjRedFenceCtrl::vfn_10()
{
}

// 0x800CEF68
void TObjRedFenceCtrl::render()
{
}

// 0x800CEF6C
Vec3f* TObjRedFence::getPos()
{
	return &mPos;
}

// 0x800CEF74
void TObjRedFence::setPos(const Vec3f* pos)
{
	mPos = *pos;
}

// 0x800CEF90
void TObjRedFenceCtrl::setPos(const Vec3f* pos)
{
	mTrans.mPos = *pos;
}

// 0x800CEFAC
int TObjRedFenceCtrl::getKind()
{
	return 3;
}

// 0x800CEFB4
int TObjRedFenceCtrl::getRotY()
{
	return mTrans.mRot.y;
}

// 0x800CEFBC
Vec3f* TObjRedFenceCtrl::getPos()
{
	return &mTrans.mPos;
}
