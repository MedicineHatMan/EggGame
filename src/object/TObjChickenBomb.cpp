#include "object/ObjAction.h"
#include "item/TItemManage.h"
#include "engine/CharaColli.h"
#include "object/ObjCommon.h"
#include "engine/DrawTask.h"
#include "item/ItemParts.h"
#include "stage/SetActor.h"
#include "egg/ObjEgg.h"
#include "stage/EventQueue.h"
#include "item/TItem.h"
#include "object/TObjChickenBomb.h"
#include "object/TObjExplode.h"
#include "camera/TCameraCtrl.h"
#include "core/MathUtil.h"
#include "object/ObjShadow.h"
#include "engine/ActorMotion.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "engine/Colli.h"
#include "egg/EggCreature.h"
#include "engine/GroundQuery.h"
#include "core/VecMath.h"
#include "object/last/TObjLastTorch.h"
struct CharaColli;

// 0x8018C074
// on is still the original bytes

extern "C" {
}

const char* gTObjChickenBombName = "TObjChickenBomb";

// 0x8018B9D8
TObjChickenBomb::TObjChickenBomb(TObject* parent, s16 id, const Vec3f* pos)
    : TObjAction(parent)
{
	ColliShape* shape;

	_00       = (u32)gTObjChickenBombName;
	mHolderId = id;
	mSize     = 0x190;
	mState    = 3;
	mDir.x    = 0.0f;
	mDir.y    = 0.0f;
	mDir.z    = 0.0f;
	mPos      = *pos;
	mtxSetTransV(mMtx, pos);
	mBomb.mRadius = 4.0f;
	mBomb.mPos    = *pos;
	mFuse         = 0x1C20;
	setObjId(this, 0xFFFF);
	mShape = colliCreate(1, 1);
	if (mShape != 0) {
		colliSetOwner(mShape, (u16)mObjId);
		colliSetMasks((CharaColli*)mShape, 4, 0xF3, 0xF0, 0);
		colliShapeSetSphere(colliGetShape((CharaColli*)mShape, 0));
		colliGetShape((CharaColli*)mShape, 0)->mFlags = 4;
		colliGetShape((CharaColli*)mShape, 0)->mSizeX = 4.0f;
	}
	mDraw       = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mShadow._30 = 8.0f;
	mShadow._34 = 8.0f;
	shadowDisable(&mShadow);
	if (itemHolderAttach(0x14, &mHolder) == 0) {
		mFlags |= 1;
	} else {
		mBomb.start(&gVecZero);
		setState(BOMB_LYING);
	}
}

// 0x8018BBC8
TObjChickenBomb::~TObjChickenBomb()
{
	if (mHolder.mNode.mList != 0) {
		itemHolderRelease(&mHolder);
	}
	removeDrawTask(mDraw);
	colliFree((CharaColli*)mShape);
	unregisterId();
}

// 0x8018BC84
void TObjChickenBomb::draw()
{
	Vec3f ground;

	run(mState, BombBody::BOMB_CHECK);
	run(mState, BombBody::BOMB_UPDATE);
	if (mShape != 0) {
		colliShapeSetPos(colliGetShape(mShape, 0), &mPos);
	}
	if (mBomb.getGround(&ground)) {
		shadowSetGround(&mShadow, &ground, 0);
		shadowEnable(&mShadow);
	} else {
		shadowDisable(&mShadow);
	}
	mGlow.update(&mHolder);
}

// 0x8018BD30
void TObjChickenBomb::vfn_10()
{
	if (!cameraIsVisible(1000.0f, 4.0f, &mPos, 0)) {
		mDraw->mFlags |= 1;
	} else {
		mDraw->mFlags &= ~1;
	}
}

// 0x8018BD98
void TObjChickenBomb::render()
{
	f32 mtx[12];

	mtxFromView(mtx, mMtx);
	mGlow.draw(&mHolder, mtx);
}

// 0x8018BDDC
void TObjChickenBomb::explode()
{
	spawnExplode(&mPos, mHolderId, 0x32);
	mFlags |= 1;
}

// 0x8018BE20
// The player's lock-on or else the nearest egg or player within 45 degrees of where the holder faces
u16 TObjChickenBomb::findTarget() const
{
	TObjAction* holder;
	u16 best;
	TObjAction* obj;
	int found;
	f32 bestDist;

	holder = findObjById(mHolderId);
	if (holder == 0) {
		return 0xFFFF;
	}
	best = camLockGetId(camGetDrawIndex());
	if (best != 0xFFFF) {
		return best;
	}
	ObjIdIter it = objIdIter();
	found        = 0;
	for (it.begin(); !it.atEnd(); it.next()) {
		obj = (TObjAction*)it.getEgg();
		if ((obj->getKind() == 2 || obj->getKind() == 0) && obj->canAct() && obj->mObjId != mHolderId) {
			if (angleDist(angleToXZ(holder->getPos(), obj->getPos()), holder->getRotY()) <= 0x2000) {
				if (!found) {
					Vec3f d;
					best = (u16)obj->mObjId;
					vecSub(&d, obj->getPos(), &mPos);
					bestDist = vecLength(&d);
					found    = 1;
				} else {
					Vec3f d;
					f32 dist;
					vecSub(&d, obj->getPos(), &mPos);
					dist = vecLength(&d);
					if (dist < bestDist) {
						best     = obj->mObjId;
						bestDist = dist;
					}
				}
			}
		}
	}
	return best;
}

// 0x8018C014
void TObjChickenBomb::run(int state, int event)
{
	switch (state) {
	case BOMB_LYING:
		stateLying(event);
		break;
	case BOMB_HELD:
		stateHeld(event);
		break;
	case BOMB_THROWN:
		stateThrown(event);
		break;
	}
}

// 0x8018C074
void TObjChickenBomb::setState(int state)
{
	if (mState != -1) {
		run(mState, BombBody::BOMB_EXIT);
	}
	if (state != -1) {
		run(state, BombBody::BOMB_ENTER);
	}
	mState = state;
}

// 0x8018C0D4
void TObjChickenBomb::stateLying(int event)
{
	if (event == BombBody::BOMB_UPDATE) {
		mBomb.update();
		mPos.x = mBomb.mPos.x;
		mPos.y = mBomb.mPos.y;
		mPos.z = mBomb.mPos.z;
		mtxSetTransV(mMtx, &mPos);
		if (--mFuse == 0) {
			explode();
		}
	}
}

// 0x8018C14C
void TObjChickenBomb::stateHeld(int event)
{
	if (event == BombBody::BOMB_ENTER) {
		colliDisable(mShape);
	} else if (event == BombBody::BOMB_EXIT) {
		colliEnable(mShape);
	} else if (event == BombBody::BOMB_UPDATE) {
		TObjAction* holder = findObjById(mHolderId);
		if (holder != 0 && (mHang.mBits & 1)) {
			holder->getNodeMatrix(mNode, mMtx);
			mtxGetPos(mMtx, &mPos);
			mtxRotateY(mMtx, mMtx, holder->getRotY());
		}
	}
}

// 0x8018C208
// Each time it lands it hops again at the target or straight on until the fuse or a hit
void TObjChickenBomb::stateThrown(int event)
{
	if (event == BombBody::BOMB_ENTER) {
		mTarget.setId(findTarget());
		colliGetShape(mShape, 0)->mFlags |= 8;
		mFuse = 600;
	} else if (event == BombBody::BOMB_UPDATE) {
		Vec3f from;
		Vec3f vel;
		EggHits hits;

		if (mBomb.mRider.mMesh != 0) {
			if (mTarget.get() != 0) {
				Vec3f was;
				Vec3f hop;

				was.x = mBomb.mVel.x;
				was.y = mBomb.mVel.y;
				was.z = mBomb.mVel.z;
				vecSub(&hop, mTarget.get()->getPos(), &mPos);
				vecNormalize(&hop, &hop);
				vecScale(120.0f, &hop, &hop);
				vecAdd(&hop, &hop, &was);
				hop.y = 0.0f;
				if (vecLength(&hop) > 120.0f) {
					vecNormalize(&hop, &hop);
					vecScale(120.0f, &hop, &hop);
				}
				hop.y = 66.66667f;
				mBomb.stop();
				mBomb.start(&hop);
			} else {
				Vec3f hop;

				vecFromAngleXZ(120.0f, &hop, vecAngleXZ(&mDir));
				hop.y = 66.66667f;
				mBomb.stop();
				mBomb.start(&hop);
			}
		}
		from.x = mBomb.mPos.x;
		from.y = mBomb.mPos.y;
		from.z = mBomb.mPos.z;
		mBomb.update();
		mPos.x = mBomb.mPos.x;
		mPos.y = mBomb.mPos.y;
		mPos.z = mBomb.mPos.z;
		vel.x  = mBomb.mVel.x;
		vel.y  = mBomb.mVel.y;
		vel.z  = mBomb.mVel.z;
		if (meshColliProbeLine((EggHit*)&hits, (const Vec3f*)&from, (const Vec3f*)&mPos, -1) != 0) {
			mPos.x = hits.mHits[0].mPos.x;
			mPos.y = hits.mHits[0].mPos.y;
			mPos.z = hits.mHits[0].mPos.z;
			explode();
		}
		if (mFuse-- == 0) {
			explode();
		}
		mtxSetTransV(mMtx, &mPos);
		mtxRotateY(mMtx, mMtx, vecAngleXZ(&vel));
	} else if (event == BombBody::BOMB_EXIT) {
		colliGetShape(mShape, 0)->mFlags &= ~8;
	}
}

// 0x8018C494
// Anything but its holder sets it off
int TObjChickenBomb::onHit(const TObjAction* other, int, int*)
{
	if (other == 0) {
		return 0;
	}
	if (other->mObjId != mHolderId) {
		explode();
	}
	return 0;
}

// 0x8018C4D8
// Only off the ground
int TObjChickenBomb::onGrab(ChickenBombMsg* msg)
{
	if (mState != BOMB_LYING) {
		return 0;
	}
	if (mBomb.mRider.mMesh == 0) {
		return 0;
	}
	mHolderId = msg->mId;
	mBomb.stop();
	mHang.mBits &= ~1;
	setState(BOMB_HELD);
	return 1;
}

// 0x8018C554
int TObjChickenBomb::onHang(ChickenBombMsg* msg)
{
	if (mState != BOMB_HELD) {
		return 0;
	}
	if (msg->mId != mHolderId) {
		return 0;
	}
	mNode = msg->mNode;
	mHang.mBits |= 1;
	return 1;
}

// 0x8018C59C
int TObjChickenBomb::onDrop(ChickenBombMsg* msg)
{
	if (mState != BOMB_HELD) {
		return 0;
	}
	if (msg->mId != mHolderId) {
		return 0;
	}
	mBomb.mPos.x = mPos.x;
	mBomb.mPos.y = mPos.y;
	mBomb.mPos.z = mPos.z;
	mBomb.start(&gVecZero);
	setState(BOMB_LYING);
	return 1;
}

// 0x8018C628
int TObjChickenBomb::onThrow(ChickenBombMsg* msg)
{
	if (mState != BOMB_HELD) {
		return 0;
	}
	if (msg->mId != mHolderId) {
		return 0;
	}
	mBomb.mPos.x = mPos.x;
	mBomb.mPos.y = mPos.y;
	mBomb.mPos.z = mPos.z;
	mDir.x       = msg->mVel.x;
	mDir.y       = msg->mVel.y;
	mDir.z       = msg->mVel.z;
	mBomb.start(&mDir);
	setState(BOMB_THROWN);
	return 1;
}

// 0x8018C6C8
int TObjChickenBomb::isFlying(ChickenBombMsg*)
{
	if (mState != BOMB_THROWN) {
		return 0;
	}
	return mBomb.mRider.mMesh == 0;
}

// 0x8018C6EC
int TObjChickenBomb::getKind()
{
	return 3;
}

// 0x8018C6F4
Vec3f* TObjChickenBomb::getPos()
{
	return &mPos;
}

// 0x8018C6FC
int TObjChickenBomb::sendToHandle(ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x67:
		return onGrab((ChickenBombMsg*)msg);
	case 0x68:
		return onHang((ChickenBombMsg*)msg);
	case 0x69:
		return onDrop((ChickenBombMsg*)msg);
	case 0x71:
		return onThrow((ChickenBombMsg*)msg);
	case 0x73:
		return isFlying((ChickenBombMsg*)msg);
	}
	return 0;
}

// 0x8018C76C
EventEntry::EventEntry(void* arg, int (*start)(void*, int), int id)
{
	mNext  = 0;
	mArg   = arg;
	mStart = start;
	mId    = id;
}

// 0x8018C784
void EventEntry::clear()
{
	mNext  = 0;
	mArg   = 0;
	mStart = 0;
	mId    = 0x20;
}

// 0x8018C7A0
EventQueue::EventQueue()
{
	mCount  = 0;
	mHead   = 0;
	mNextId = 0;
}

// 0x8018C7B4
EventQueue::~EventQueue()
{
	EventEntry* entry = mHead;
	while (entry != 0) {
		EventEntry* next = entry->mNext;
		if (entry != 0) {
			delete entry;
		}
		entry = next;
	}
}

// 0x8018C81C
EventEntry* EventQueue::tail()
{
	EventEntry* entry;

	if (mHead == 0) {
		return 0;
	}
	for (entry = mHead; entry->mNext != 0; entry = entry->mNext) { }
	return entry;
}

// 0x8018C84C
int EventQueue::push(void* arg, int (*start)(void*, int))
{
	EventEntry* entry = (EventEntry*)::operator new(sizeof(EventEntry));
	entry->__ct(arg, start, mNextId);
	int id  = mNextId;
	mNextId = id + 1;
	EventEntry* last;

	mCount++;
	last = tail();
	if (last == 0) {
		mHead = entry;
	} else {
		last->mNext = entry;
	}
	return id;
}

// 0x8018C8D0
int EventQueue::pop(EventEntry* out)
{
	EventEntry* next;

	if (mHead == 0) {
		return 0;
	}
	*out       = *mHead;
	out->mNext = 0;
	next       = mHead->mNext;
	if (mHead != 0) {
		delete mHead;
		mHead = 0;
	}
	mHead = next;
	mCount--;
	return 1;
}

// 0x8018C964
extern "C" TObject* createTObjLastTorch(TObject* parent, SetDesc* desc)
{
	return new TObjLastTorch(parent, desc);
}
