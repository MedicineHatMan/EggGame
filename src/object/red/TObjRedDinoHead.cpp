#include "object/red/TObjRedDinoHead.h"
#include "engine/DrawTask.h"
#include "Ninja/ninja.h"
#include "engine/CharaColli.h"
#include "object/ObjArchive.h"
#include "engine/Colli.h"
#include "enemy/ObjEnemy.h"
#include "engine/TParticle.h"
#include "object/red/TObjRedDHeadInvokeArea.h"
#include "core/Mem.h"
#include "camera/TCameraCtrl.h"
#include "object/ObjAction.h"
#include "sound/Sound3DPlay.h"
#include "core/MathUtil.h"
#include "egg/EggCreature.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "object/TObjKey00.h"
#include "core/VecMath.h"
#include "engine/TFrameBlur.h"
#include "core/SdkCalls.h"
#include "egg/ObjEgg.h"
#include "object/red/RedHeadListImpl.h"
struct ColliShape;
struct NJS_OBJECT;

extern HitVec sDinoPoints[16];
extern f32 sDinoNodeMtx[12];

typedef char TObjRedDinoHead_size_check[(sizeof(TObjRedDinoHead) == 0x194) ? 1 : -1];

// Bit 2 of the particle's flags hides it
static inline void setPartHide(TParticleGenerator* part, int hide)
{
	u16* flags = &part->mFlags;
	if (hide != 0) {
		*flags |= 4;
	} else {
		*flags &= ~4;
	}
}

static inline GjModel* firstGeoNode(void* geo)
{
	return ((NJS_OBJECT*)geo)->child->model;
}

static void linkHead(RedHeadList* list, RedHeadLink* link);
static void unlinkHead(RedHeadList* list, RedHeadLink* link);

// 0x800C568C
// The set table's init every point starts at the same value
extern "C" void dinoHeadSetInit()
{
	for (int i = 0; i < 16; i++) {
		sDinoPoints[i].x = 0.0f;
		sDinoPoints[i].y = 0.0f;
		sDinoPoints[i].z = 0.0f;
	}
}

// 0x800C56C0
// The set table's exit
extern "C" void dinoHeadSetExit()
{
}

// 0x800C56C4
extern "C" TObject* createTObjRedDinoHead(TObject* parent, SetDesc* desc)
{
	return new TObjRedDinoHead(parent, desc);
}

const char* gTObjRedDinoHeadName = "TObjRedDinoHead";

const MotionEntry sRedDinoHeadMotions[] = {
	{ 0, 6, -1, 0, 0.0f, 30.0f, 0.2f, 0.0f, 0 },
	{ 0, 7, -1, 0, 0.0f, 30.0f, 0.2f, 0.0f, 0 },
	{ 1, 7, -1, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
};

// 0x800C5718
TObjRedDinoHead::TObjRedDinoHead(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00          = (u32)gTObjRedDinoHeadName;
	mSize        = 0x194;
	mLink.mOwner = this;
	linkHead(&gRedHeadList, &mLink);
	mGeo      = getGeObjModels()[8];
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	setupColli();
	initFromDesc(desc);
	setup();
}

// 0x800C581C
TObjRedDinoHead::~TObjRedDinoHead()
{
	seStop3D((SoundHandle*)&mLoop);
	freeShape();
	removeDrawTask(mDrawTask);
	unlinkHead(&gRedHeadList, &mLink);
}

// 0x800C58B0
void TObjRedDinoHead::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mGroupId  = (u16)desc->getInt(0);
	mSwitchId = (u16)desc->getInt(3);
	mLife     = (s16)desc->getInt(1);
	mScale    = 1.0f + desc->getFloat(0);
	setupShape();
	buildMatrix();
}

// 0x800C5968
void TObjRedDinoHead::debugPrint(void*)
{
	njPrint(0xA0013, ":GROUP ID");
	njPrint(0xA0014, ":LIFE");
	njPrint(0xA0016, ":SW ID");
	njPrint(0xA0017, "+1.0:Scale");
}

// 0x800C59DC
// The model's centre scaled
static inline Vec3f scaledCentre(void* geo, f32 s)
{
	GjModel* model = firstGeoNode(geo);
	Vec3f r;
	Vec3f centre;
	centre.set(model->center.x, model->center.y, model->center.z);
	gjScaleVector(s, &r, &centre);
	return r;
}

void TObjRedDinoHead::setup()
{
	mHitFlags  = 0;
	mOpen      = 0;
	mOpenCount = 0;
	colliDisable(mShape);
	mModel.setModel(mGeo);
	mModel.mState   = &mDrawState;
	mModel.mTexList = getGeObjTextures()[8];
	mModel.mMotion  = &mMotion;

	Vec3f scaled = scaledCentre(mGeo, mScale);
	mTint.reset();

	mPartL = particleGenByIdRot(this, &gVecZero, 1, 8, 0, mRot.y + 0x4000, 0.0f);
	mPartR = particleGenByIdRot(this, &gVecZero, 1, 8, 0, mRot.y - 0x4000, 0.0f);
	mFxPos.set(0.0f, 0.5f * scaled.y, scaled.z);
	vecRotateY(&mFxPos, &mFxPos, mRot.y);
	vecAdd(&mFxPos, &mFxPos, &mPos);
	mPartFront = particleGenByIdRot(this, &mFxPos, 1, 10, 0, mRot.y + 0x8000, 0.0f);
	setPartHide(mPartL, 1);
	setPartHide(mPartR, 1);
	setPartHide(mPartFront, 1);

	void** motions = gGeObjCommon->mSet.mMotions;
	mMotion.init(sRedDinoHeadMotions, motions);
	if (switchIsClear() == 0) {
		notifyAreas();
		mMotion.play(2);
		setAct(2);
	} else {
		sePlay3DHeld(500.0f, (SoundHandle*)&mLoop, 0x21C, 0, (const Vec3f*)&mFxPos);
		mMotion.play(0);
		setAct(0);
	}
}

// 0x800C5C08
void TObjRedDinoHead::setAct(int act)
{
	mAct = act;
}

// 0x800C5C10
void TObjRedDinoHead::draw()
{
	if (isInRange(this, &mPos, 2100.0f) == 0) {
		mFlags |= 1;
		return;
	}
	if (objDebugEnabled(this) != 0) {
		updateShapePos();
	}
	switch (mAct) {
	case 0:
		stateAlive();
		break;
	case 1:
		stateDying();
		break;
	case 2:
		stateDead();
		break;
	}
}

// 0x800C5CBC
void TObjRedDinoHead::stateAlive()
{
	TObjAction* other;
	if ((mHitFlags & 1) == 0) {
		for (ColliContact* hit = mShape->mContacts; hit != 0; hit = hit->mNext) {
			if (colliContactHasBit8(hit) == 0) {
				continue;
			}
			other = (TObjAction*)findObjById((u16)colliContactOwner(hit));
			if (other == 0) {
				continue;
			}
			HitInfo info;
			other->onHit(this, (u16)mObjId, (int*)&info);
			if (info.mFlags.testBit(5) == 0) {
				continue;
			}
			ObjMsg msg;
			msg.mType = 0x47;
			other->sendToHandle(&msg);
			mLife = (s16)(mLife - info.mDamage);
			if (mLife <= 0) {
				colliDisable(mShape);
				mHitFlags &= ~1u;
				mDrawState.mFlags = 0;
				mTint.clear();
				notifyAreas();
				setPartHide(mPartFront, 1);
				setPartHide(mPartL, 1);
				setPartHide(mPartR, 1);
				mMotion.playBlend(2, 0.01f);
				frameBlurStart(3000, 0.8f);
				seStop3D((SoundHandle*)&mLoop);
				sePlay3D(0x21F, &mFxPos);
				setAct(1);
			} else {
				mTint.start();
				setPartHide(mPartFront, 0);
				mHitFlags |= 1;
			}
			break;
		}
	} else if (mTint.update() != 0) {
		mTint.clear();
		mDrawState.mFlags = 0;
		setPartHide(mPartFront, 1);
		mHitFlags &= ~1u;
	} else {
		u32 level = mTint.mLevel;
		if (level != 0) {
			mDrawState.mFlags |= 0x20;
			mDrawState.mTev2 = level << 24;
		}
	}
	pose();
	Vec3f* left  = &mPartL->mPos;
	Vec3f at     = sDinoPoints[2];
	*left        = at;
	mPartR->mPos = at;
}

// 0x800C5F28
void TObjRedDinoHead::stateDying()
{
	if (mMotion.isDone() != 0) {
		setKeyBit(mSwitchId);
		setAct(2);
	}
	mMotion.update();
}

// 0x800C5F7C
void TObjRedDinoHead::stateDead()
{
}

// 0x800C5F80
// Read through a getter comparing the field directly swaps the cmpw operands
static inline s16 areaGroup(void* area)
{
	return ((TObjRedDHeadInvokeArea*)area)->mGroupId;
}
void TObjRedDinoHead::notifyAreas()
{
	for (RedAreaLink* link = gRedAreaList.mTail; link != 0; link = link->mNext) {
		if (mGroupId == areaGroup(link->mOwner)) {
			((TObjRedDHeadInvokeArea*)link->mOwner)->shut();
		}
	}
}

// 0x800C5FDC
int TObjRedDinoHead::switchIsClear()
{
	return keyBitIsSet(mSwitchId) == 0;
}

// 0x800C6008
void TObjRedDinoHead::open()
{
	if (switchIsClear() == 0) {
		return;
	}
	if (mMotion.mCurrent != 1) {
		mMotion.playBlend(1, 0.05f);
	}
	setPartHide(mPartL, 0);
	setPartHide(mPartR, 0);
	colliEnable(mShape);
	mOpen = 1;
	if (mOpenCount == 0) {
		sePlay3D(0x21E, &mFxPos);
	}
	mOpenCount += 1;
}

// 0x800C60AC
void TObjRedDinoHead::close()
{
	int count  = mOpenCount - 1;
	mOpenCount = count < 0 ? 0 : count;
	if (mOpenCount != 0) {
		return;
	}
	if ((s16)mMotion.mCurrent != 0) {
		mMotion.playBlend(0, 0.05f);
	}
	setPartHide(mPartL, 1);
	setPartHide(mPartR, 1);
	colliDisable(mShape);
	mOpen = 0;
}

// 0x800C6140
void TObjRedDinoHead::buildMatrix()
{
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateZYX((f32*)mMatrix, (const f32*)mMatrix, (const int*)&mRot);
	mtxScale(mScale, mScale, mScale, mMatrix, mMatrix);
}

// 0x800C619C
void TObjRedDinoHead::pose()
{
	mMotion.update();
	njPushMatrixEx();
	njSetMatrix(0, mMatrix);
	if (mMotion.isBlending() != 0) {
		njStartMotionLink((const NJS_MOTION_LINK*)mMotion.getBlend(), mMotion.mBlendTime);
		drawNodes(mGeo, 0);
	} else {
		int cur         = mMotion.mCurrent;
		f32 frame       = mMotion.mFrame;
		NJS_MOTION* mot = mMotion.getMotion(cur);
		njStartMotionObj(mot, frame);
		drawNodes(mGeo, 0);
	}
	njPopMatrixEx();
}

// 0x800C624C
int TObjRedDinoHead::drawNodes(void* node, int index)
{
	do {
		njMotionTransformEx((NJS_OBJECT*)node);
		njGetMatrix(sDinoNodeMtx);
		mtxMultVec(sDinoNodeMtx, &gVecZero, &sDinoPoints[index]);
		index += 1;
		void* child = *(void**)((u8*)node + 0x2C);
		if (child != 0) {
			index = drawNodes(child, index);
		}
		node = (NJS_OBJECT*)(*(void**)((u8*)node + 0x30));
	} while (node != 0);
	return index;
}

// 0x800C62E8
void TObjRedDinoHead::setupColli()
{
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 0xFF, 0xFF, 0xDF);
}

// 0x800C6344
void TObjRedDinoHead::setupShape()
{
	GjModel* model = firstGeoNode(mGeo);
	ColliShape shape;
	colliShapeInit(&shape);
	shape.mType  = COLLISHAPE_SPHERE;
	shape.mSizeX = model->r * mScale;
	shape.mFlags |= 0x10;
	colliCopyShapes(mShape, &shape);
	updateShapePos();
}

// 0x800C63B8
void TObjRedDinoHead::updateShapePos()
{
	GjModel* model = firstGeoNode(mGeo);
	Vec3f centre;
	centre.set(model->center.x, model->center.y, model->center.z);
	f32 mtx[12];
	ColliShape shape;
	mtxSetRotY(mtx, mRot.y);
	mtxScale(mScale, mScale, mScale, mtx, mtx);
	mtxMultVec(mtx, &centre, &centre);
	shape.mPos = centre;
	colliPlaceShapes(mShape, &mPos, &shape);
}

// 0x800C6460
void TObjRedDinoHead::freeShape()
{
	colliRetire(mShape);
	mShape = 0;
}

// Not called Pools 2000 ahead of vfn_10's 2.0
f32 dinoHeadFar()
{
	return 2000.0f;
}

// 0x800C6498
void TObjRedDinoHead::vfn_10()
{
	ColliShape* shape = colliGetShape(mShape, 0);
	f32 radius        = shape->mSizeX;
	radius            = 2.0f * radius;
	if (cameraIsVisible(2000.0f, radius, &mPos, 0) != 0) {
		mDrawTask->mFlags &= 0xFFFE;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x800C6514
void TObjRedDinoHead::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	mModel.draw();
	njPopMatrixEx();
}

// 0x800C6558
static void linkHead(RedHeadList* list, RedHeadLink* link)
{
	if (list->mTail == 0) {
		list->mHead = link;
		list->mTail = link;
	} else {
		link->mPrev        = list->mHead;
		list->mHead->mNext = link;
		list->mHead        = link;
	}
	link->mList = list;
	list->mCount += 1;
}

// 0x800C6598
static void unlinkHead(RedHeadList* list, RedHeadLink* link)
{
	if (link->mPrev != 0) {
		link->mPrev->mNext = link->mNext;
	} else {
		list->mTail = link->mNext;
	}
	if (link->mNext != 0) {
		link->mNext->mPrev = link->mPrev;
	} else {
		list->mHead = link->mPrev;
	}
	link->mNext = 0;
	link->mPrev = 0;
	link->mList = 0;
	list->mCount -= 1;
}

// 0x800C65F8
// is this file's __sinit
RedHeadList gRedHeadList;
RedHeadSlots sRedHeadSlots;
HitVec sDinoPoints[16];
// drawNodes' scratch matrix
f32 sDinoNodeMtx[12];
