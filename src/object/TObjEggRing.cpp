#include "object/TObjEggRing.h"
#include "engine/DrawTask.h"
#include "egg/ObjEgg.h"
#include "object/ObjArchive.h"
#include "engine/DrawState.h"
#include "engine/CharaColli.h"
#include "object/ObjAction.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "sound/SoundCtrl.h"
#include "object/ObjShadow.h"
#include "object/TObjLauncher.h"
#include "Ninja/ninja.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCameraCtrl.h"
#include "sound/Sound3DPlay.h"
#include "engine/FxSystem.h"
#include "ui/AeArc.h"
#include "stl/math.h"
#include "engine/GroundQuery.h"
#include "egg/EggCreature.h"
#include "stage/SetSlot.h"
#include "core/VecMath.h"
#include "engine/LightSet.h"
#include "io/NinjaFile.h"
struct NJS_OBJECT;
struct SetLink;
struct Fx;

// The DOL copies the home rotation as one block paired loads which CW does for a struct holding an array but not for
// SetRot's three ints
struct RotBlock {
	int v[3];
};

static inline void resetRot(SetRot* dst, SetRot* src)
{
	*(RotBlock*)dst = *(RotBlock*)src;
}

// 0x8025F6D8
// by kind The first data in the file TRANSFER opens .rodata and the rest open .sdata2
const char* gEggRingKindNames[6] = { "TRANSFER", "SPIN", "SPRING", "ROCKET", "TRAPEZE", "SWING" };

extern "C" {
}

#define RING_ABS(x) ((x) < 0 ? -(x) : (x))

static inline f32 ringSqrt(f32 x)
{
	static const f64 _half  = 0.5;
	static const f64 _three = 3.0;
	if (x > 0.0f) {
		f64 guess         = __frsqrte((f64)x);
		guess             = _half * guess * (_three - guess * guess * x);
		guess             = _half * guess * (_three - guess * guess * x);
		guess             = _half * guess * (_three - guess * guess * x);
		volatile f32 root = (f32)(x * guess);
		return root;
	}
	return x;
}

// How many rings are held right now
static int sHeldCount;

static inline void dropHeld()
{
	int n      = sHeldCount - 1;
	sHeldCount = n & ~(n >> 31);
}

// 0x80137068
void EggRingContent::init()
{
	sHeldCount = 0;
}

// 0x80137074
void EggRingContent::exit()
{
	sHeldCount = 0;
}

// 0x80137080
int EggRingContent::getHeldCount()
{
	return sHeldCount;
}

// 0x80137088
EggRingContent* EggRingContent::create(TObjEggRing* owner, int kind)
{
	if (owner == 0) {
		return 0;
	}
	switch (kind) {
	case EGGRING_TRANSFER:
		return new EggRingTransfer(owner);
	case EGGRING_SPIN:
		return new EggRingSpin(owner);
	case EGGRING_SPRING:
		return new EggRingSpring(owner);
	case EGGRING_ROCKET:
		return new EggRingRocket(owner);
	case EGGRING_TRAPEZE:
		return new EggRingTrapeze(owner);
	case EGGRING_SWING:
		return new EggRingSwing(owner);
	default:
		return new EggRingTransfer(owner);
	}
}

// 0x801371E8
EggRingContent::EggRingContent(TObjEggRing* owner)
    : mDirty(0)
    , mOwner(owner)
{
	mHome.x = 0.0f;
	mHome.y = 0.0f;
	mHome.z = 0.0f;
	mHomeRot.set(0, 0, 0);
	mPos.x = 0.0f;
	mPos.y = 0.0f;
	mPos.z = 0.0f;
	mRot.set(0, 0, 0);
	mRadius           = 15.0f;
	mShadowPos.x      = 0.0f;
	mShadowPos.y      = 0.0f;
	mShadowPos.z      = 0.0f;
	_6C               = 0;
	mPower            = 100.0f;
	mPhase            = 0;
	mFx               = 0;
	mDrawState.mFlags = 0;
	mDrawState._20    = 0;
	mFlags            = 0;
	mState            = 0;
	mHolder.setId(0xFFFF);
	mRing.mRadius = mRadius;
	mModels[0]    = getStgObjModels()[0x30];
	mModels[1]    = getStgObjModels()[0x31];
	mModels[2]    = getStgObjModels()[0x32];
	mTex          = getStgObjTextures()[0x27];
	initColli();
}

// 0x80137300
EggRingContent::~EggRingContent()
{
	freeColli();
	if (mFlags & 1) {
		dropHeld();
	}
}

// 0x8013738C
int EggRingContent::hasHolder()
{
	TObjAction* holder = mHolder.get();
	if (holder == 0) {
		mHolder.setId(0xFFFF);
		return 0;
	}
	return 1;
}

// 0x801373E0
int EggRingContent::launch(f32 x, f32 y, f32 z, int playSe)
{
	RingMsgVec msg;
	f32 mtx[12];

	if (!hasHolder()) {
		return 0;
	}
	msg.mVec.x = x;
	msg.mType  = 0x22;
	msg.mVec.y = y;
	msg.mVec.z = z;
	mtxIdentity(mtx);
	mtxRotateY(mtx, mtx, mRot.y);
	mtxRotateX(mtx, mtx, mRot.x);
	mtxMultVec(mtx, &msg.mVec, &msg.mVec);
	if (mHolder.get()->sendToHandle(&msg) != 0) {
		if (playSe) {
			sePlay3D(0x55, &mPos);
		}
		mHolder.setId(0xFFFF);
		return 1;
	}
	return 0;
}

// 0x801374E0
int EggRingContent::launchCounted(f32 x, f32 y, f32 z, int playSe)
{
	ObjMsg ask;
	RingMsgVec msg;
	f32 mtx[12];

	if (!hasHolder()) {
		return 0;
	}
	msg.mVec.x = x;
	msg.mType  = 0x23;
	msg.mVec.y = y;
	msg.mVec.z = z;
	mtxIdentity(mtx);
	mtxRotateY(mtx, mtx, mRot.y);
	mtxRotateX(mtx, mtx, mRot.x);
	mtxMultVec(mtx, &msg.mVec, &msg.mVec);
	if (mHolder.get()->sendToHandle(&msg) != 0) {
		if (playSe) {
			ask.mType = 0x8B;
			int count = mHolder.get()->sendToHandle(&ask);
			if (count > 0) {
				int se = sePlay3D(0x56, &mPos);
				seSetPitch(se, count >= 8 ? 0x1FFF : (s16)((count - 1) * 0x492));
			}
		}
		mHolder.setId(0xFFFF);
		return 1;
	}
	return 0;
}

// 0x8013762C
void EggRingContent::initColli()
{
	ColliShape shape;

	mColli = colliCreate(1, 1);
	colliSetOwner(mColli, ((TObjAction*)mOwner)->mObjId);
	colliSetMasks(mColli, 4, 0xFF, 0xFF, 0xF7);
	colliShapeInit(&shape);
	shape.mType  = 0;
	shape.mSizeX = 15.0f;
	shape.mFlags |= 0x10;
	colliCopyShapes(mColli, &shape);
}

// 0x801376BC
void EggRingContent::placeColli()
{
	colliShapeSetPos(colliGetShape(mColli, 0), &mPos);
}

// 0x801376F8
void EggRingContent::freeColli()
{
	colliRetire(mColli);
	mColli = 0;
}

// 0x80137730
void EggRingContent::touch()
{
	mDirty = 1;
}

// 0x8013773C
void EggRingContent::readDesc(SetDescObj* desc)
{
	desc->getPos(&mHome);
	desc->getRot(&mHomeRot);
	mDescFlag = desc->get3E();
	mPower    = (int)(100.0f + desc->getInt(1));
}

// 0x80137804
void EggRingContent::debugPrint()
{
	njPrintC(0xA0014, ":POWER");
}

// 0x80137830
void EggRingContent::calc()
{
	calcMatrix();
	calcShadow();
	calcGlow();
	placeColli();
}

// 0x80137898
void EggRingContent::calcMatrix()
{
	mtxSetTransV(mMtx, &mPos);
	mtxRotateY(mMtx, mMtx, mRot.y);
	mtxRotateX(mMtx, mMtx, mRot.x);
	mtxCopy(mScaledMtx, mMtx);
	mtxRotateX(mFxMtx, mScaledMtx, -0x4000);
	f32 scale = getSize() / 12.0f;
	mtxScale(scale, scale, scale, mScaledMtx, mScaledMtx);
	mtxScale(scale, scale, scale, mFxMtx, mFxMtx);
}

// 0x80137960
static inline Vec3f ringSub(const Vec3f* a, const Vec3f* b)
{
	Vec3f d;
	gjSubVector(&d, a, b);
	return d;
}

int EggRingContent::calcShadow()
{
	EggHits hits;

	Vec3f from = mPos;
	from.y += 10.0f;
	if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&from, -1) != 0 && (hits.mMask & 1)) {
		Vec3f diff = ringSub(&mPos, &((EggHit*)&hits)->mPos);
		if (njScalor(&diff) < 200.0f) {
			mShadowPos = hits.mHits[0].mPos;
			mtxSetTransV(mShadowMtx, &mShadowPos);
			mtxRotateY(mShadowMtx, mShadowMtx, mRot.y);
			mtxScale(1.0f, 1.0f, (f32)__fabs(njCos(mRot.x)), mShadowMtx, mShadowMtx);
			mFlags &= ~8;
			return 1;
		}
	}
	mtxCopy(mShadowMtx, mMtx);
	mFlags |= 8;
	return 0;
}

// 0x80137AB4
void EggRingContent::calcGlow()
{
	mtxCopy(mGlowMtx, mMtx);
}

// 0x80137AE0
void EggRingContent::onCatch(const Vec3f* hand)
{
}

// 0x80137AE4
void EggRingContent::setState(int state)
{
	if (state == EGGRING_IDLE) {
		stopFx();
		dropHeld();
		mFlags &= ~1;
	}
	mState = state;
}

// 0x80137B3C
void EggRingContent::update()
{
	switch (mState) {
	case EGGRING_IDLE:
		idle();
		break;
	case EGGRING_HELD:
		hold();
		break;
	case EGGRING_RELEASE:
		release();
		break;
	}
	if (mDirty) {
		calc();
		mDirty = 0;
	}
}

// 0x80137BEC
void EggRingContent::idle()
{
	ColliContact* head = ((CharaColli*)mColli)->mContacts;
	if (head != 0) {
		for (ColliContact* contact = head; contact != 0; contact = contact->mNext) {
			ColliShape* shape = colliGetShape(contact->mOther, contact->mOtherShape);
			if (!(shape->mFlags & 0x400)) {
				continue;
			}
			u16 id          = colliContactOwnerU16(contact);
			TObjAction* obj = findObjById((int)id);
			if (obj == 0) {
				continue;
			}

			Vec3f up;
			f32 mtx[12];
			Vec3f diff;
			up.x = 0.0f;
			up.y = 1.0f;
			up.z = 0.0f;
			mtxSetRotY(mtx, mRot.y);
			mtxRotateX(mtx, mtx, mRot.x);
			mtxMultVec(mtx, &up, &up);
			vecSub(&diff, obj->getPos(), &mPos);
			f32 along  = njInnerProduct(&up, &diff);
			f32 height = along - obj->getRadiusV();
			f32 gap    = RING_ABS(vecLength(&diff) - obj->getRadiusV());
			mRadius    = lerpF(obj->getRadiusV(), 15.0f, gap / 15.0f);
			touch();

			if (along < 0.0f) {
				continue;
			}
			RingMsgVec hand;
			hand.mType = 0x43;
			obj->sendToHandle(&hand);
			if (njInnerProduct(&up, &hand.mVec) > 0.0f || height > 4.0f) {
				continue;
			}
			mHolder.setId(id);
			RingMsgGrab grab;
			grab.mType = 0x16;
			grab.mFrom = 0xFFFF;
			grab.mFrom = ((TObjAction*)mOwner)->mObjId;
			grab._06   = 0;
			if (obj->sendToHandle(&grab) == 0) {
				mHolder.setId(0xFFFF);
				continue;
			}
			mRadius = obj->getRadiusV();
			onCatch(&hand.mVec);
			if (mState == EGGRING_IDLE && !(mFlags & 1)) {
				sHeldCount++;
				mFlags |= 1;
			}
			sePlay3D(0x54, &mPos);
			setState(EGGRING_HELD);
			break;
		}
	} else {
		mRadius = 15.0f;
	}
	mRing.mRadius = mRadius;
}

// 0x80137F2C
void EggRingContent::hold()
{
	touch();
	if (mState == EGGRING_HELD && !hasHolder()) {
		setState(EGGRING_RELEASE);
	}
}

// 0x80137F8C
void EggRingContent::release()
{
	touch();
}

// 0x80137FAC
int EggRingContent::onMessage(ObjMsg* msg)
{
	if (msg->mType == 0x27) {
		mHolder.setId(0xFFFF);
		setState(EGGRING_RELEASE);
		return 1;
	}
	return 0;
}

// 0x80138014
void EggRingContent::getMatrix(int node, f32* out)
{
	if (node == 0) {
		mtxCopy(out, mGlowMtx);
	}
}

// 0x80138048
void EggRingContent::spawnFx(int id, f32* mtx)
{
	fxSpawnAt(1, 7, id, mtx);
}

// 0x80138078
void EggRingContent::startFx(int id, f32* mtx)
{
	stopFx();
	mFx = fxSpawnFollow(1, 7, id, mtx);
}

// 0x801380C4
void EggRingContent::stopFx()
{
	if (mFx != 0) {
		fxStop(mFx);
		mFx = 0;
	}
}

// 0x80138104
void EggRingContent::checkVisibleAt(f32 radius, Vec3f* pos)
{
	Vec3f view;

	if (cameraIsVisible(1000.0f, radius, pos, &view) != 0) {
		if (view.z > -600.0f) {
			mRing.mLod = 0;
		} else if (view.z > -900.0f) {
			mRing.mLod = 1;
		} else {
			mRing.mLod = 2;
		}
		mFlags |= 2;
	} else {
		mFlags &= ~2;
	}

	if (mFlags & 8) {
		mFlags &= ~4;
	} else {
		if (cameraIsVisible(1000.0f, getSize(), &mShadowPos, &view) != 0) {
			if (view.z > -600.0f) {
				mRing.mShadowLod = 0;
			} else if (view.z > -900.0f) {
				mRing.mShadowLod = 1;
			} else {
				mRing.mShadowLod = 2;
			}
			mFlags |= 4;
		} else {
			mFlags &= ~4;
		}
	}
}

// 0x80138238
void EggRingContent::checkVisible()
{
	checkVisibleAt(getSize(), &mPos);
}

// 0x8013827C
void EggRingContent::draw()
{
	f32 mtx[12];

	if (mFlags & 2) {
		shadowAlphaBegin();
		mtxConcat(mtx, (f32*)gjGetMatrix(), mMtx);
		njPushMatrix(mtx);
		mRing.draw();
		njPopMatrixEx();
		shadowAlphaEnd();
		if (mOwner != 0 && objDebugEnabled(mOwner) != 0) {
			drawDebug();
		}
	}
}

// 0x8013830C
void EggRingContent::drawShadow()
{
	f32 mtx[12];

	if (mFlags & 4) {
		mtxConcat(mtx, (f32*)gjGetMatrix(), mShadowMtx);
		njPushMatrix(mtx);
		mRing.drawShadow();
		njPopMatrixEx();
	}
}

// 0x80138368
void EggRingContent::drawDebug()
{
	Vec3f home;
	Vec3f end;
	SetRot rot;
	f32 mtx[12];
	u32 colorA;
	u32 colorB;

	f32 size = getSize();
	getHome(&home);
	getEnd(&end);
	getRot(&rot);
	mtxTranslateV(mtx, (f32*)gjGetMatrix(), &home);
	mtxRotateZYX((f32*)mtx, (const f32*)mtx, (const int*)&rot);
	mtxScale(size, 2.0f, size, mtx, mtx);
	colorA = 0xFF7F1FC4;
	debugWireCylinder(mtx, &colorA);
	mtxTranslateV(mtx, (f32*)gjGetMatrix(), &end);
	mtxRotateZYX((f32*)mtx, (const f32*)mtx, (const int*)&rot);
	mtxScale(size, 2.0f, size, mtx, mtx);
	colorB = 0xFF7F1FC4;
	debugWireCylinder(mtx, &colorB);
}

// ---- EggRingTransfer ----

// 0x801384A0
EggRingTransfer::EggRingTransfer(TObjEggRing* owner)
    : EggRingContent(owner)
{
	mKind                = EGGRING_TRANSFER;
	mRing.mColor         = 0xFF8040FF;
	mDrawState.mFlags    = DRAWSTATE_MATERIAL;
	mDrawState.mMaterial = mRing.mColor;
}

// 0x80138500
EggRingTransfer::~EggRingTransfer()
{
}

// 0x80138558
void EggRingTransfer::readDesc(SetDescObj* desc)
{
	EggRingContent::readDesc(desc);
	mPos = mHome;
	resetRot(&mRot, &mHomeRot);
	mTag      = desc->getInt(2);
	mOffset.x = 0.0f;
	mOffset.y = desc->getFloat(1);
	mOffset.z = desc->getFloat(2);
	mDir      = (int)(182.04445f * desc->getFloat(3));
	vecRotateY(&mOffset, &mOffset, mDir);
	vecAdd(&mEnd, &mHome, &mOffset);
	f32 speed = desc->getFloat(0);
	mSpeed    = (int)(speed * (65535.0f / vecLength(&mOffset)));
	calc();
}

// 0x801386C8
void EggRingTransfer::debugPrint()
{
	EggRingContent::debugPrint();
	njPrint(0xA0015, ":TaG %s", mTag ? "YES" : "NO");
	njPrintC(0xA0017, ":SPD");
	njPrintC(0xA0018, ":Y-MOVE");
	njPrintC(0xA0019, ":Z-MOVE");
	njPrintC(0xA001A, ":DIR-XZ");
}

// 0x8013875C
void EggRingTransfer::calc()
{
	f32 reach = 8.0f + getSize();
	EggRingContent::calc();
	mtxRotateY(mBlade[0], mMtx, mPhase << 1);
	mtxCopy(mBlade[1], mBlade[0]);
	mtxRotateY(mBlade[1], mBlade[0], 0x5555);
	mtxRotateY(mBlade[2], mBlade[0], 0xAAAA);
	mtxTranslate((float*)mBlade[0], (const float*)mBlade[0], 0.0f, 0.0f, reach);
	mtxTranslate((float*)mBlade[1], (const float*)mBlade[1], 0.0f, 0.0f, reach);
	mtxTranslate((float*)mBlade[2], (const float*)mBlade[2], 0.0f, 0.0f, reach);
}

// 0x80138840
int EggRingTransfer::move()
{
	u16 old = mPhase;
	mPhase += mSpeed;
	int wrapped;
	if (mPhase <= old) {
		wrapped = 1;
		mPhase  = 0;
	} else {
		wrapped = 0;
	}
	mPos = mOffset;
	gjScaleVector(0.5f * (1.0f + njSin(mPhase - 0x4000)), &mPos, &mPos);
	vecAdd(&mPos, &mPos, &mHome);
	return wrapped;
}

// 0x801388EC
void EggRingTransfer::setState(int state)
{
	EggRingContent::setState(state);
	if (state == EGGRING_RELEASE && mTag == 0) {
		EggRingContent::setState(EGGRING_IDLE);
	}
}

// 0x8013893C
void EggRingTransfer::idle()
{
	EggRingContent::idle();
	if (mTag == 0) {
		move();
		touch();
	}
}

// 0x80138984
void EggRingTransfer::hold()
{
	move();
	EggRingContent::hold();
}

// 0x801389B8
void EggRingTransfer::release()
{
	EggRingContent::idle();
	if (move()) {
		setState(EGGRING_IDLE);
	}
	EggRingContent::release();
}

// 0x80138A14
void EggRingTransfer::onCatch(const Vec3f* hand)
{
	startFx(0xC, mFxMtx);
}

// 0x80138A3C
int EggRingTransfer::onMessage(ObjMsg* msg)
{
	if (msg->mType == 0x34) {
		if (launch(0.0f, mPower, 0.0f, 1)) {
			spawnFx(0x11, mFxMtx);
			setState(EGGRING_RELEASE);
			return 1;
		}
		return 0;
	}
	return EggRingContent::onMessage(msg);
}

// 0x80138AC8
void EggRingTransfer::checkVisible()
{
	checkVisibleAt(10.0f + getSize(), &mPos);
}

// 0x80138B14
void EggRingTransfer::draw()
{
	int lod = mRing.mLod;
	beginDrawState(getDrawStateSlot(), &mDrawState);
	njSetTexture(mTex);
	njPushMatrixEx();
	njMultiMatrix(0, mBlade[0]);
	gjDrawObject((NJS_OBJECT*)mModels[lod]);
	njPopMatrixEx();
	njPushMatrixEx();
	njMultiMatrix(0, mBlade[1]);
	gjDrawObject((NJS_OBJECT*)mModels[lod]);
	njPopMatrixEx();
	njPushMatrixEx();
	njMultiMatrix(0, mBlade[2]);
	gjDrawObject((NJS_OBJECT*)mModels[lod]);
	njPopMatrixEx();
	endDrawState(getDrawStateSlot());
	EggRingContent::draw();
}

// ---- EggRingSpin ----

// 0x80138BC0
EggRingSpin::EggRingSpin(TObjEggRing* owner)
    : EggRingContent(owner)
{
	mKind        = EGGRING_SPIN;
	mTop         = getStgObjModels()[0xC];
	mPole        = getStgObjModels()[0xD];
	mTopTex      = getStgObjTextures()[0xB];
	mPoleTex     = getStgObjTextures()[0xC];
	mRing.mColor = 0xFFD639FF;
}

// 0x80138C50
EggRingSpin::~EggRingSpin()
{
	seStop3D(&mSound);
}

// 0x80138CB4
void EggRingSpin::readDesc(SetDescObj* desc)
{
	EggRingContent::readDesc(desc);
	mPos       = mHome;
	mRot.y     = mHomeRot.y;
	mRot.z     = mHomeRot.z;
	mSpinSpeed = RING_ABS((int)(182.04445f * desc->getFloat(0)));
	if (((int (*)(void*))debugInfoActive)(mOwner) != 0) {
		spin();
	}
	calc();
}

// 0x80138DD0
void EggRingSpin::debugPrint()
{
	EggRingContent::debugPrint();
	njPrintC(0xA0017, ":SPD");
	njPrintC(0xA0018, "");
	njPrintC(0xA0019, "");
	njPrintC(0xA001A, "");
}

// 0x80138E30
int EggRingSpin::spin()
{
	// The DOL reads the angle again after loading the speed so this read is not shared with the add
	u16 old = *(volatile int*)&mRot.x;
	mRot.x += mSpinSpeed;
	if ((u16)mRot.x < old) {
		mRot.x = 0;
		return 1;
	}
	return 0;
}

// 0x80138E70
void EggRingSpin::idle()
{
	EggRingContent::idle();
}

// 0x80138E90
void EggRingSpin::hold()
{
	spin();
	sePlay3DHeld(500.0f, &mSound, 0x57, 0, (const Vec3f*)&mPos);
	EggRingContent::hold();
}

// 0x80138EDC
void EggRingSpin::release()
{
	EggRingContent::idle();
	if (spin()) {
		seStop3D(&mSound);
		setState(EGGRING_IDLE);
	}
	EggRingContent::release();
}

// 0x80138F40
void EggRingSpin::onCatch(const Vec3f* hand)
{
	spawnFx(5, mScaledMtx);
}

// 0x80138F68
int EggRingSpin::onMessage(ObjMsg* msg)
{
	if (msg->mType == 0x34) {
		SetRot rot = mRot;
		mRot.x |= 0x1FFF;
		if (launchCounted(0.0f, mPower, 0.0f, 1)) {
			spawnFx(0x75, mFxMtx);
			setState(EGGRING_RELEASE);
			mRot = rot;
			return 1;
		}
		mRot = rot;
		return 0;
	}
	return EggRingContent::onMessage(msg);
}

// 0x80139044
void EggRingSpin::draw()
{
	f32 mtx[12];

	shadowAlphaBegin();
	mtxConcat(mtx, (f32*)gjGetMatrix(), mMtx);
	njPushMatrix(mtx);
	f32 radius = mRing.mRadius;
	beginDrawState(getDrawStateSlot(), getDefaultDrawState());
	njTranslate(0, 0.0f, radius, 0.0f);
	njSetTexture(mPoleTex);
	gjDrawObject(mPole);
	njTranslate(0, 0.0f, 10.0f, 0.0f);
	gjDrawObject(mPole);
	njTranslate(0, 0.0f, 10.0f, 0.0f);
	njRotateX(0, 0x4000);
	njSetTexture(mTopTex);
	gjDrawObject(mTop);
	endDrawState(getDrawStateSlot());
	njPopMatrixEx();
	shadowAlphaEnd();
	EggRingContent::draw();
}

// ---- EggRingSpring ----

// 0x8013913C
EggRingSpring::EggRingSpring(TObjEggRing* owner)
    : EggRingContent(owner)
    , mVel(gVecZero)
{
	mKind                = EGGRING_SPRING;
	mRing.mColor         = 0x80FF40FF;
	mDrawState.mFlags    = DRAWSTATE_MATERIAL;
	mDrawState.mMaterial = mRing.mColor;
}

// 0x801391BC
EggRingSpring::~EggRingSpring()
{
}

// 0x80139214
void EggRingSpring::readDesc(SetDescObj* desc)
{
	EggRingContent::readDesc(desc);
	mPos = mHome;
	resetRot(&mRot, &mHomeRot);
	calc();
}

// 0x80139284
void EggRingSpring::debugPrint()
{
	EggRingContent::debugPrint();
	njPrintC(0xA0017, "");
	njPrintC(0xA0018, "");
	njPrintC(0xA0019, "");
	njPrintC(0xA001A, "");
}

// 0x801392E4
void EggRingSpring::calc()
{
	f32 reach = 8.0f + getSize();
	f32 run   = 8.0f + RingDraw::getWidth();
	f32 dy    = mPos.y - mHome.y;
	mStretch  = ringSqrt(run * run + dy * dy);
	int angle = (int)(10430.38043493439 * (f32)atan2(dy, run));
	EggRingContent::calc();
	mtxSetTransV(mRope[0], &mHome);
	mtxRotateY(mRope[0], mRope[0], mRot.y);
	mtxRotateY(mRope[1], mRope[0], 0x4000);
	mtxRotateY(mRope[2], mRope[0], 0x8000);
	mtxRotateY(mRope[3], mRope[0], 0xC000);
	mtxTranslate((float*)mRope[0], (const float*)mRope[0], 0.0f, 0.0f, reach);
	mtxTranslate((float*)mRope[1], (const float*)mRope[1], 0.0f, 0.0f, reach);
	mtxTranslate((float*)mRope[2], (const float*)mRope[2], 0.0f, 0.0f, reach);
	mtxTranslate((float*)mRope[3], (const float*)mRope[3], 0.0f, 0.0f, reach);
	mtxRotateX(mRope[0], mRope[0], angle);
	mtxRotateX(mRope[1], mRope[1], angle);
	mtxRotateX(mRope[2], mRope[2], angle);
	mtxRotateX(mRope[3], mRope[3], angle);
}

// 0x801394CC
int EggRingSpring::bounce()
{
	f32 dy = mPos.y - mHome.y;
	mVel.y = -0.03f * dy + mVel.y;
	vecAdd(&mPos, &mPos, &mVel);
	gjScaleVector(0.98f, &mVel, &mVel);
	if (dy < 0.0f && mPos.y - mHome.y > 0.0f) {
		return 1;
	}
	return 0;
}

// 0x8013956C
void EggRingSpring::idle()
{
	EggRingContent::idle();
}

// 0x8013958C
void EggRingSpring::hold()
{
	bounce();
	EggRingContent::hold();
}

// 0x801395C0
void EggRingSpring::release()
{
	EggRingContent::idle();
	bounce();
	if ((f32)__fabs(mVel.y) < 0.0005f) {
		mVel.x = 0.0f;
		mVel.y = 0.0f;
		mVel.z = 0.0f;
		mPos   = mHome;
		setState(EGGRING_IDLE);
	}
	EggRingContent::release();
}

// 0x80139654
void EggRingSpring::onCatch(const Vec3f* hand)
{
	mVel.y = 0.5f * (0.016666668f * hand->y) + -1.0f;
	spawnFx(5, mScaledMtx);
}

// 0x80139698
int EggRingSpring::onMessage(ObjMsg* msg)
{
	if (msg->mType == 0x34) {
		if (launch(0.0f, mPower, 0.0f, 1)) {
			gjScaleVector(-1.0f, &mVel, &mVel);
			spawnFx(10, mFxMtx);
			setState(EGGRING_RELEASE);
			return 1;
		}
		return 0;
	}
	return EggRingContent::onMessage(msg);
}

// 0x80139734
void EggRingSpring::checkVisible()
{
	checkVisibleAt(10.0f + getSize(), &mPos);
}

// 0x80139780
void EggRingSpring::draw()
{
	f32 mtx[12];
	u32* color = &mRing.mColor;
	int lod    = mRing.mLod;

	for (int i = 0; i < 4; i++) {
		mtxConcat(mtx, (f32*)gjGetMatrix(), mRope[i]);
		drawTube(mtx, 4, color, -mStretch, 0.0f, 1.0f);
	}
	beginDrawState(getDrawStateSlot(), &mDrawState);
	njSetTexture(mTex);
	for (int i = 0; i < 4; i++) {
		njPushMatrixEx();
		njMultiMatrix(0, mRope[i]);
		gjDrawObject((NJS_OBJECT*)mModels[lod]);
		njPopMatrixEx();
	}
	endDrawState(getDrawStateSlot());
	EggRingContent::draw();
}

// ---- EggRingRocket ----

// 0x80139864
EggRingRocket::EggRingRocket(TObjEggRing* owner)
    : EggRingContent(owner)
{
	mKind                = EGGRING_ROCKET;
	mRing.mColor         = 0x4080FFFF;
	mDrawState.mFlags    = DRAWSTATE_MATERIAL;
	mDrawState.mMaterial = mRing.mColor;
	mFlame[1]            = 0;
	mFlame[0]            = 0;
}

// 0x801398D0
EggRingRocket::~EggRingRocket()
{
	fxRemove((Fx*)mFlame[1]);
	fxRemove((Fx*)mFlame[0]);
}

// 0x8013993C
void EggRingRocket::readDesc(SetDescObj* desc)
{
	f32 mtx[12];

	EggRingContent::readDesc(desc);
	mPos = mHome;
	resetRot(&mRot, &mHomeRot);
	mRot.x -= 0x4000;
	mOffset.x = 0.0f;
	mOffset.y = desc->getFloat(1);
	mOffset.z = desc->getFloat(2);
	mDir      = (int)(182.04445f * desc->getFloat(3));
	vecRotateY(&mOffset, &mOffset, mDir);
	mtxSetRotY(mtx, mHomeRot.y);
	mtxRotateX(mtx, mtx, mHomeRot.x);
	mtxMultVec(mtx, &mOffset, &mOffset);
	vecAdd(&mEnd, &mHome, &mOffset);
	f32 speed = desc->getFloat(0);
	mSpeed    = (int)(speed * (65535.0f / vecLength(&mOffset)));
	if (((int (*)(void*))debugInfoActive)(mOwner) != 0) {
		move(0);
	}
	calc();
}

// 0x80139AE4
void EggRingRocket::debugPrint()
{
	EggRingContent::debugPrint();
	njPrintC(0xA0015, "");
	njPrintC(0xA0017, ":SPD");
	njPrintC(0xA0018, ":Y-MOVE");
	njPrintC(0xA0019, ":Z-MOVE");
	njPrintC(0xA001A, ":DIR-XZ");
}

// 0x80139B54
void EggRingRocket::calc()
{
	f32 reach = 8.0f + getSize();
	EggRingContent::calc();
	mtxRotateY(mJet[0], mMtx, mPhase << 1);
	mtxCopy(mJet[1], mJet[0]);
	mtxTranslate((float*)mJet[0], (const float*)mJet[0], 0.0f, 0.0f, reach);
	mtxTranslate((float*)mJet[1], (const float*)mJet[1], 0.0f, 0.0f, -reach);
}

// 0x80139BFC
int EggRingRocket::move(int back)
{
	int done;
	u16 old = mPhase;
	mPhase += mSpeed;
	if (back == 0) {
		if ((s16)mPhase < (s16)old) {
			done = 1;
		} else {
			done = 0;
		}
	} else if (mPhase < old) {
		done   = 1;
		mPhase = 0;
	} else {
		done = 0;
	}
	mPos = mOffset;
	gjScaleVector(0.5f * (1.0f + njSin(mPhase - 0x4000)), &mPos, &mPos);
	vecAdd(&mPos, &mPos, &mHome);
	return done;
}

// 0x80139CD4
void EggRingRocket::setState(int state)
{
	if (state == EGGRING_HELD) {
	} else if (state == EGGRING_RELEASE) {
		stopFx();
		fxStop((Fx*)mFlame[1]);
		mFlame[1] = 0;
		fxStop((Fx*)mFlame[0]);
		mFlame[0] = 0;
		colliDisable(mColli);
	}
	EggRingContent::setState(state);
}

// 0x80139D48
void EggRingRocket::idle()
{
	EggRingContent::idle();
}

// 0x80139D68
void EggRingRocket::hold()
{
	if (move(0) && launchCounted(0.0f, -mPower, 0.0f, 1)) {
		spawnFx(0x15, mFxMtx);
		setState(EGGRING_RELEASE);
	}
	EggRingContent::hold();
}

// 0x80139DF4
void EggRingRocket::release()
{
	if (move(1)) {
		EggRingContent::idle();
		colliEnable(mColli);
		setState(EGGRING_IDLE);
	}
	EggRingContent::release();
}

// 0x80139E5C
void EggRingRocket::onCatch(const Vec3f* hand)
{
	startFx(0xB, mFxMtx);
	mFlame[0] = fxSpawnFollow(1, 7, 0x37, mJet[0]);
	mFlame[1] = fxSpawnFollow(1, 7, 0x37, mJet[1]);
}

// 0x80139EC0
int EggRingRocket::onMessage(ObjMsg* msg)
{
	return EggRingContent::onMessage(msg);
}

// 0x80139EE0
void EggRingRocket::checkVisible()
{
	checkVisibleAt(10.0f + getSize(), &mPos);
}

// 0x80139F2C
void EggRingRocket::draw()
{
	int lod = mRing.mLod;
	EggRingContent::draw();
	beginDrawState(getDrawStateSlot(), &mDrawState);
	njSetTexture(mTex);
	njPushMatrixEx();
	njMultiMatrix(0, mJet[0]);
	gjDrawObject((NJS_OBJECT*)mModels[lod]);
	njPopMatrixEx();
	njPushMatrixEx();
	njMultiMatrix(0, mJet[1]);
	gjDrawObject((NJS_OBJECT*)mModels[lod]);
	njPopMatrixEx();
	endDrawState(getDrawStateSlot());
}

// ---- EggRingTrapeze ----

// 0x80139FB8
EggRingTrapeze::EggRingTrapeze(TObjEggRing* owner)
    : EggRingContent(owner)
{
	mKind                = EGGRING_TRAPEZE;
	mRing.mColor         = 0x00FFFFFF;
	mDrawState.mFlags    = DRAWSTATE_MATERIAL;
	mDrawState.mMaterial = mRing.mColor;
}

// 0x8013A018
EggRingTrapeze::~EggRingTrapeze()
{
}

// 0x8013A070
void EggRingTrapeze::readDesc(SetDescObj* desc)
{
	EggRingContent::readDesc(desc);
	mHomeRot.x = -0x4000;
	mPos       = mHome;
	resetRot(&mRot, &mHomeRot);
	mSpeed = -RING_ABS((int)(182.04445f * desc->getFloat(0)));
	mPivot = mHome;
	mPivot.y += desc->getFloat(1);
	mMaxAngle = (int)(182.04445f * desc->getFloat(2));
	if (((int (*)(void*))debugInfoActive)(mOwner) != 0) {
		swing();
	}
	calc();
}

// 0x8013A228
void EggRingTrapeze::debugPrint()
{
	EggRingContent::debugPrint();
	njPrintC(0xA0015, "");
	njPrintC(0xA0017, ":SPD");
	njPrintC(0xA0018, ":ROT RAD");
	njPrintC(0xA0019, ":MAX DEG");
}

// 0x8013A290
void EggRingTrapeze::calc()
{
	EggRingContent::calc();
	mtxSetTransV(mBarMtx, &mPivot);
	mtxRotateY(mBarMtx, mBarMtx, mRot.y);
	mtxRotateX(mBarMtx, mBarMtx, mRot.x);
}

// 0x8013A2E8
int EggRingTrapeze::swing()
{
	Vec3f arm;
	f32 mtx[12];

	u16 old = mPhase;
	mPhase += mSpeed;
	int angle = (int)(mMaxAngle * njSin(mPhase));
	mRot.x    = angle - 0x4000;
	vecSub(&arm, &mHome, &mPivot);
	mtxSetRotY(mtx, mRot.y);
	mtxRotateX(mtx, mtx, angle);
	mtxMultVec(mtx, &arm, &arm);
	vecAdd(&mPos, &mPivot, &arm);
	return old < mPhase;
}

// 0x8013A39C
void EggRingTrapeze::idle()
{
	EggRingContent::idle();
}

// 0x8013A3BC
void EggRingTrapeze::hold()
{
	swing();
	EggRingContent::hold();
}

// 0x8013A3F0
void EggRingTrapeze::release()
{
	if (swing()) {
		EggRingContent::idle();
		mPos = mHome;
		resetRot(&mRot, &mHomeRot);
		colliEnable(mColli);
		setState(EGGRING_IDLE);
	}
	EggRingContent::release();
}

// 0x8013A484
void EggRingTrapeze::onCatch(const Vec3f* hand)
{
}

// 0x8013A488
int EggRingTrapeze::onMessage(ObjMsg* msg)
{
	if (msg->mType == 0x34) {
		if (launchCounted(0.0f, -mPower, 0.0f, 1)) {
			colliDisable(mColli);
			setState(EGGRING_RELEASE);
			return 1;
		}
		return 0;
	}
	return EggRingContent::onMessage(msg);
}

// 0x8013A510
void EggRingTrapeze::checkVisible()
{
	Vec3f mid;

	vecSub(&mid, &mPivot, &mPos);
	gjScaleVector(0.5f, &mid, &mid);
	f32 size   = getSize();
	f32 radius = vecLength(&mid);
	radius += size;
	vecAdd(&mid, &mid, &mPos);
	checkVisibleAt(radius, &mid);
}

// 0x8013A5A4
void EggRingTrapeze::draw()
{
	Vec3f arm;
	f32 mtx[12];

	int lod = mRing.mLod;
	vecSub(&arm, &mPivot, &mHome);
	mtxConcat(mtx, (f32*)gjGetMatrix(), mBarMtx);
	f32 width = RingDraw::getWidth();
	drawTube(mtx, 4, &mRing.mColor, -((vecLength(&arm) - mRadius) - width), 0.0f, 1.0f);
	beginDrawState(getDrawStateSlot(), &mDrawState);
	njSetTexture(mTex);
	njPushMatrixEx();
	njMultiMatrix(0, mBarMtx);
	gjDrawObject((NJS_OBJECT*)mModels[lod]);
	njPopMatrixEx();
	endDrawState(getDrawStateSlot());
	EggRingContent::draw();
}

// ---- EggRingSwing ----

// 0x8013A684
EggRingSwing::EggRingSwing(TObjEggRing* owner)
    : EggRingContent(owner)
{
	mKind                = EGGRING_SWING;
	mRing.mColor         = 0xFF00FFFF;
	mDrawState.mFlags    = DRAWSTATE_MATERIAL;
	mDrawState.mMaterial = mRing.mColor;
}

// 0x8013A6E4
EggRingSwing::~EggRingSwing()
{
}

// 0x8013A73C
void EggRingSwing::readDesc(SetDescObj* desc)
{
	EggRingContent::readDesc(desc);
	mHomeRot.x = -0x4000;
	mPos       = mHome;
	resetRot(&mRot, &mHomeRot);
	mSpeed = -RING_ABS((int)(182.04445f * desc->getFloat(0)));
	mPivot = mHome;
	mPivot.y += desc->getFloat(1);
	if (((int (*)(void*))debugInfoActive)(mOwner) != 0) {
		swing();
	}
	calc();
}

// 0x8013A8A8
void EggRingSwing::debugPrint()
{
	EggRingContent::debugPrint();
	njPrintC(0xA0015, "");
	njPrintC(0xA0017, ":SPD");
	njPrintC(0xA0018, ":ROT RAD");
}

// 0x8013A8FC
void EggRingSwing::calc()
{
	EggRingContent::calc();
	mtxSetTransV(mBarMtx, &mPivot);
	mtxRotateY(mBarMtx, mBarMtx, mRot.y);
	mtxRotateX(mBarMtx, mBarMtx, mRot.x);
}

// 0x8013A954
int EggRingSwing::swing()
{
	Vec3f arm;
	f32 mtx[12];

	u16 old = mPhase;
	mPhase += mSpeed;
	mRot.x = mPhase - 0x4000;
	vecSub(&arm, &mHome, &mPivot);
	mtxSetRotY(mtx, mRot.y);
	mtxRotateX(mtx, mtx, mPhase);
	mtxMultVec(mtx, &arm, &arm);
	vecAdd(&mPos, &mPivot, &arm);
	return old < mPhase;
}

// 0x8013A9F0
void EggRingSwing::idle()
{
	EggRingContent::idle();
}

// 0x8013AA10
void EggRingSwing::hold()
{
	swing();
	EggRingContent::hold();
}

// 0x8013AA44
void EggRingSwing::release()
{
	if (swing()) {
		mPos = mHome;
		resetRot(&mRot, &mHomeRot);
		colliEnable(mColli);
		setState(EGGRING_IDLE);
	}
	EggRingContent::release();
}

// 0x8013AAD0
void EggRingSwing::onCatch(const Vec3f* hand)
{
}

// 0x8013AAD4
int EggRingSwing::onMessage(ObjMsg* msg)
{
	if (msg->mType == 0x34) {
		if (launchCounted(0.0f, -mPower, 0.0f, 1)) {
			colliDisable(mColli);
			setState(EGGRING_RELEASE);
			return 1;
		}
		return 0;
	}
	return EggRingContent::onMessage(msg);
}

// 0x8013AB5C
void EggRingSwing::checkVisible()
{
	Vec3f mid;

	vecSub(&mid, &mPivot, &mPos);
	gjScaleVector(0.5f, &mid, &mid);
	f32 size   = getSize();
	f32 radius = vecLength(&mid);
	radius += size;
	vecAdd(&mid, &mid, &mPos);
	checkVisibleAt(radius, &mid);
}

// 0x8013ABF0
void EggRingSwing::draw()
{
	Vec3f arm;
	f32 mtx[12];

	int lod = mRing.mLod;
	vecSub(&arm, &mPivot, &mHome);
	mtxConcat(mtx, (f32*)gjGetMatrix(), mBarMtx);
	f32 width = RingDraw::getWidth();
	drawTube(mtx, 4, &mRing.mColor, -((vecLength(&arm) - mRadius) - width), 0.0f, 1.0f);
	beginDrawState(getDrawStateSlot(), &mDrawState);
	njSetTexture(mTex);
	njPushMatrixEx();
	njMultiMatrix(0, mBarMtx);
	gjDrawObject((NJS_OBJECT*)mModels[lod]);
	njPopMatrixEx();
	endDrawState(getDrawStateSlot());
	EggRingContent::draw();
}

// ---- TObjEggRing ----

// 0x8013ACD0
// From the set manager's load
extern "C" void eggRingInit()
{
	EggRingContent::init();
}

// 0x8013ACF0
extern "C" void eggRingExit()
{
	EggRingContent::exit();
}

static void drawShadowTask(TObjEggRing* ring);

// 0x8013AD10
extern "C" TObject* createTObjEggRing(TObject* parent, SetDesc* desc)
{
	return new TObjEggRing(parent, desc);
}

// 0x8013AD64
// The kind comes from int 0
void TObjEggRing::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mKind    = desc->getInt(0);
	int kind = mKind;
	if (kind < 0) {
		kind = 0;
	} else if (kind > 5) {
		kind = 5;
	}
	mKind = kind;
	if (mHeld == 0) {
		mHeld = EggRingContent::create(this, mKind);
	}
	if (mKind != mHeld->getKind()) {
		delete mHeld;
		mHeld = EggRingContent::create(this, mKind);
	}
	mHeld->readDesc((SetDescObj*)desc);
}

const char* gTObjEggRingName = "TObjEggRing";

// 0x8013AE48
void TObjEggRing::debugPrint(void*)
{
	if (mHeld != 0) {
		mHeld->debugPrint();
		njPrint(0xA0013, ":TYPE %s", gEggRingKindNames[mHeld->mKind]);
	}
}

// 0x8013AEBC
TObjEggRing::TObjEggRing(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	mHeld = 0;
	_00   = (u32)gTObjEggRingName;
	mSize = 0x98;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mGlow = addDrawTask(this, (void (*)(void*))drawShadowTask, 0x23, 0);
	initFromDesc(desc);
	findLight();
}

// 0x8013AF80
TObjEggRing::~TObjEggRing()
{
	delete mHeld;
	removeDrawTask(mGlow);
	removeDrawTask(mDraw);
}

// 0x8013B01C
void TObjEggRing::findLight()
{
	mLightSet = lightSetFindArea(&mPos);
	setLinkFree((SetLink*)&mLink);
}

// 0x8013B058
// Out of range and idle it goes
void TObjEggRing::draw()
{
	Vec3f pos;
	mHeld->getPos(&pos);
	if (!isInRange((void*)this, (const Vec3f*)&pos, 1100.0f) && mHeld->mState == EGGRING_IDLE) {
		mFlags |= 1;
	} else {
		mHeld->update();
	}
}

// 0x8013B0D8
int TObjEggRing::sendToHandle(ObjMsg* msg)
{
	return mHeld->onMessage(msg);
}

// 0x8013B108
void TObjEggRing::getNodeMatrix(int node, f32* mtx)
{
	mHeld->getMatrix(node, mtx);
}

// 0x8013B138
void TObjEggRing::vfn_10()
{
	mHeld->checkVisible();
}

// 0x8013B168
void TObjEggRing::render()
{
	if (mHeld->mFlags & 2) {
		lightSetSelect(mLightSet);
		beginDrawState(getDrawStateSlot(), getDefaultDrawState());
		mHeld->draw();
		endDrawState(getDrawStateSlot());
	}
}

// 0x8013B1D8
void TObjEggRing::drawShadow()
{
	if (mHeld->mFlags & 4) {
		mHeld->drawShadow();
	}
}

// 0x8013B214
static void drawShadowTask(TObjEggRing* ring)
{
	ring->drawShadow();
}
