#include "enemy/ObjEnemy.h"
#include "io/Resource.h"
#include "enemy/EnemyFx.h"
#include "enemy/EnemyHit.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "sound/SeHolder.h"
#include "enemy/EnemyAnim.h"
#include "enemy/EnemyAppear.h"
#include "enemy/TEneAm01Parts.h"
#include "enemy/EnemyCommon.h"
#include "egg/ObjEgg.h"
#include "object/ObjAction.h"
#include "stage/TEventCtrl.h"
#include "object/ObjShadow.h"
#include "camera/TCameraCtrl.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "camera/TCamera.h"
#include "object/TObjLauncher.h"
#include "engine/CharaColli.h"
#include "engine/DrawState.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCamPath.h"
#include "enemy/EnemyGround.h"
#include "stage/TGameState.h"
#include "engine/GroundQuery.h"
#include "ui/TAeObjBossConsole.h"
#include "enemy/EnemyGroup.h"
#include "stage/SetSlot.h"
#include "engine/Colli.h"
#include "stage/TStageScore.h"
#include "core/TPeriVibrate.h"
#include "ui/BossConsole.h"
#include "engine/LightSet.h"
#include "object/TObjEggFeed.h"

extern const f32 gUnitVectorMin;

// ObjEnemy's class name Defined ahead of the code the DOL's .rodata has it before "ar_ene_param.arc"
const char* gTObjEnemyName = "TObjEnemy";

// 0x8000C500
void touchListInit(EnemyTouchList* list)
{
	list->mCount = 0;
	for (int i = 0; i < 4; i++) {
		list->mIds[i] = 0xFFFF;
	}
}

// 0x8000C52C
void touchListAdd(EnemyTouchList* list, u32 id)
{
	if (list->mCount < 4) {
		list->mIds[list->mCount] = id;
		list->mCount++;
	}
}

// 0x8000C554
int touchListHas(EnemyTouchList* list, u32 id)
{
	for (int i = 0; i < list->mCount; i++) {
		if (list->mIds[i] == (u16)id) {
			return 1;
		}
	}
	return 0;
}

// 0x8000C594
void cooldownListInit(EnemyCooldownList* list)
{
	list->mCount = 0;
	for (int i = 0; i < 4; i++) {
		list->mTimers[i] = 0;
		list->mIds[i]    = 0xFFFF;
	}
}

// 0x8000C5C8
// The cooldown is 30 frames
void cooldownListAdd(EnemyCooldownList* list, u32 id)
{
	if (list->mCount < 4) {
		list->mTimers[list->mCount] = 30;
		list->mIds[list->mCount]    = id;
		list->mCount++;
	}
}

// 0x8000C604
int cooldownListHas(EnemyCooldownList* list, u32 id)
{
	for (int i = 0; i < list->mCount; i++) {
		if (list->mIds[i] == (u16)id) {
			return 1;
		}
	}
	return 0;
}

// 0x8000C644
// Counts down the timers A finished entry is replaced by the last one
void cooldownListUpdate(EnemyCooldownList* list)
{
	for (int i = 0; i < list->mCount; i++) {
		list->mTimers[i]--;
		if (list->mTimers[i] <= 0) {
			list->mCount--;
			list->mTimers[i] = list->mTimers[list->mCount];
			list->mIds[i]    = list->mIds[list->mCount];
			list->mTimers[list->mCount] = 0;
			list->mIds[list->mCount]    = 0xFFFF;
			i--;
		}
	}
}

// 0x8000C6E4
void enemyHitInit(EnemyHit* hit)
{
	hit->mObjId    = 0xFFFF;
	hit->mHolderId = 0xFFFF;
	hit->mShape    = -1;
	hit->mKind     = 0;
	resetHitInfo(&hit->mInfo);
}

// 0x8000C72C
int enemyHitTestFlag(EnemyHit* hit, int bit)
{
	return hit->mInfo.mFlags.testBit(bit);
}

// 0x8000C750
// The push from a hit flat away from the point at 150/7 units a frame
void enemyHitPushAway(Vec3f* unused, const Vec3f* pos, const Vec3f* from, Vec3f* out)
{
	*out = *pos;
	njSubVector(out, from);
	out->y = 0.0f;
	if (njUnitVector(out) < gUnitVectorMin) {
		*out = gVecZero;
	}
	gjScaleVector(21.428572f, out, out);
}

// 0x8000C7E4
// The player behind a hit the hitter else whoever holds it else nobody
u16 enemyHitPlayer(EnemyHit* hit)
{
	if (isPlayerId(hit->mObjId) != 0) {
		return hit->mObjId;
	}
	if (isPlayerId(hit->mHolderId) != 0) {
		return hit->mHolderId;
	}
	return 0xFFFF;
}

// 0x8000C844
void enemyContactInit(EnemyContact* contact)
{
	contact->mPos.x     = 0.0f;
	contact->mPos.y     = 0.0f;
	contact->mPos.z     = 0.0f;
	contact->mCenter.x  = 0.0f;
	contact->mCenter.y  = 0.0f;
	contact->mCenter.z  = 0.0f;
	contact->mSurface.x = 0.0f;
	contact->mSurface.y = 0.0f;
	contact->mSurface.z = 0.0f;
}

// 0x8000C870
void enemyParamLoad()
{
	loadArc("ar_ene_param.arc", &sEnemyHitArc);
	EnemyCommonParams* params = (EnemyCommonParams*)enemyParamGet(0);
	gEnemyParams = params;
	gEnemyFilterX = params->mFilterX;
	int angle = params->mFilterAngle;
	gEnemyFilterZ = njCos(angle);
	gEnemyFilterY = njCos(angle);
	enemyFxLoadStub();
}

// 0x8000C8D8
void enemyParamRelease()
{
	enemyFxFreeStub();
	releaseResource(sEnemyHitArc);
	sEnemyHitArc = NULL;
	gEnemyParams = NULL;
}

// 0x8000C90C
void* enemyParamGet(int index)
{
	return ((void**)sEnemyHitArc->mData)[index];
}

struct GjActor;
struct Vec3f;
struct ObjShadow;
struct DrawState;
struct EnemyLight;
struct GroundFilter;
struct ColliContact;
struct ColliShape;
struct CharaColli;
struct SetLink;
struct EnemyCooldownList;
struct DrawStateSlot;
struct TAeObjBossConsole;

// What colliGetShape finds
struct EnemyShape {
	Vec3f mCenter; // _00
	u8 _0C[0xC];
	f32 mRadius;   // _18
	u8 _1C[8];
	u32 mFlags;    // _24 0x400 = hurts
};

// What sendGameMsg and bossConsoleMsg deliver and sendToHandle
struct EnemyMsg {
	u16 mType; // _00
	u16 _02;
	union {
		u16 mId;
		f32 mRatio;
	}; // _04
};

inline void TObjEnemy::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}

// 0x8000C920
// A bare action takes no id range
int TObjAction::getKind()
{
	return 5;
}

// 0x8000C928
// No description a fresh id
TObjEnemy::TObjEnemy(TObject* parent) : TObjAction(parent)
{
	setObjId(this, 0xFFFF);
	setup();
}

// 0x8000CA00
// The description's id wins unless that id is already taken
TObjEnemy::TObjEnemy(TObject* parent, SetDesc* desc) : TObjAction(parent)
{
	u16 id = (u16)(int)desc->mObjId;
	if (findObjById(id) != 0) {
		id = 0xFFFF;
	}
	setObjId(this, id);
	setup();
	readEnemyDesc(desc);
	mLink.get()->attach(desc);
	setLinkFree((SetLink*)&mLink);
	updateArea();
}

// 0x8000CACC
// The 10 is at the body's start and passes through its slot _0C
void TObjEnemy::setup()
{
	_00 = (u32)gTObjEnemyName;
	mSize = 0x158;
	mBody.mGravity.x = 0.0f;
	mBody.mGravity.y = -0.16333334f;
	mBody.mGravity.z = 0.0f;
	mBody.mRadius = 10.0f;
	SetTransform* body = &mBody;
	body->vfn_0C();
	motionCtrlSetPlayer(&mAnim, &mMotion);
	mCollider = 0;
	mHitFlags = 0;
	mEnemyFlags = 0;
	mHealth = 0;
	touchListInit(&mTouching);
	cooldownListInit(&mCooldown);
	hatchClear(&mAppearTimer);
	mAppearObj = 0;
	enemyHitInit(&mHit);
	enemyContactInit(&mContact);
	mFrameKey = 0;
	mArea = 0;
}

// 0x8000CB90
TObjEnemy::~TObjEnemy()
{
	if (mAppearObj != 0) {
		mAppearObj->mFlags |= 1;
		mAppearObj = 0;
	}
	notifySetLinkDeath((SetLink*)&mLink, (void*)this);
	releaseObjId(this);
}

// 0x8000CC58
void TObjEnemy::draw()
{
	updateBody();
}

// 0x8000CC84
int TObjEnemy::returnZero()
{
	return 0;
}

// 0x8000CC8C
// Slot _38 The position is copied as the home too
void TObjEnemy::readEnemyDesc(SetDesc* desc)
{
	desc->getPos(&mBody.mPos);
	mHome = mBody.mPos;
	desc->getRot(&mBody.mRot);

	s16 kind;
	s16 unused;
	enemyDescGetKind(desc, &kind, &unused);
	if (isEnemyGrouped(kind) != 0) {
		mEnemyFlags |= ENEMY_FROM_DESC;
	} else {
		mEnemyFlags &= ~ENEMY_FROM_DESC;
	}
}

// 0x8000CD3C
extern "C" void drawFirstFrame(NJS_MOTION** table, int count, void* model)
{
	gjPushUnitMatrix();
	poseFirstFrame(table, count, model);
	njPopMatrixEx();
}

// 0x8000CD84
extern "C" void poseFirstFrame(NJS_MOTION** table, int count, void* model)
{
	f32 frame;
	NJS_MOTION* motion = firstMotionFrame(table, count, &frame);
	gjMotionActor(frame, (GjActor*)model, motion);
}

// Clamps index to [lo hi]
static inline void clampIndex(int& index, s32 lo, int hi)
{
	if (index < lo) {
		index = lo;
	} else {
		index = index > hi ? hi : index;
	}
}

// 0x8000CDC4
extern "C" NJS_MOTION* firstMotionFrame(NJS_MOTION** table, int count, f32* frame)
{
	int index = 0;
	clampIndex(index, 0, count - 1);
	NJS_MOTION* motion = table[index];
	f32 last = motion->nbFrame - 1;
	*frame = 0.0f > last ? last : 0.0f;
	return motion;
}

// 0x8000CE2C
u16 TObjEnemy::getHitPlayer()
{
	return enemyHitPlayer(&mHit);
}

// 0x8000CE50
// Slot _74
void TObjEnemy::setPos(const Vec3f* pos)
{
	f32 x = pos->x;
	f32 y = pos->y;
	mBody.mPos.x = x;
	f32 z = pos->z;
	mBody.mPos.y = y;
	mBody.mPos.z = z;
}

// 0x8000CE6C
// Slot _44
int TObjEnemy::canAct()
{
	u8 ok = 0;
	if (isBlocked() == 0 && (mEnemyFlags & ENEMY_CANT_ACT) == 0) {
		ok = 1;
	}
	return ok;
}

// 0x8000CEC4
// Slot _90
int TObjEnemy::isBlocked()
{
	return mEnemyFlags & ENEMY_BLOCKED;
}

// 0x8000CED0
void TObjEnemy::enterAction(int action)
{
	onActionPhase(action, 0);
}

// 0x8000CF00
void TObjEnemy::exitAction(int action)
{
	onActionPhase(action, 2);
}

// 0x8000CF30
void TObjEnemy::vfn_64(int action)
{
	onActionPhase(action, 3);
}

// 0x8000CF60
void TObjEnemy::vfn_68(int action)
{
	onActionPhase(action, 1);
}

// 0x8000CF90
void TObjEnemy::beginAppear(int* step)
{
	mEnemyFlags |= ENEMY_APPEARING;
	mHitFlags &= ~HIT_DONE;
	if (mCollider != 0) {
		colliDisable(mCollider);
	}
	*step = 0;
}

// 0x8000CFE8
void TObjEnemy::runAppear(int* step)
{
	if (*step == 0) {
		mAppearObj = new TEnemyAppear(this, mBody.mPos, 1);
		hatchClear(&mAppearTimer);
		(*step)++;
	} else if (*step == 1) {
		if (appearLanded() != 0) {
			(*step)++;
		}
	} else {
		tickAppear();
	}
}

// 0x8000D0A4
// As above with the object told to travel along the body's velocity
void TObjEnemy::runAppearAimed(int* step)
{
	if (*step == 0) {
		mAppearObj = new TEnemyAppear(this, mBody.mPos, 0);
		if (mAppearObj != 0) {
			mAppearObj->launch(&mBody.mVel);
		}
		hatchClear(&mAppearTimer);
		(*step)++;
	} else if (*step == 1) {
		if (appearLanded() != 0) {
			(*step)++;
		}
	} else {
		tickAppear();
	}
}

// 0x8000D174
int TObjEnemy::appearLanded()
{
	if (mAppearObj->isLanded()) {
		mAppearObj->getPos(&mBody.mPos);
		hatchStart(&mAppearTimer);
		mEnemyFlags &= ~ENEMY_APPEARING;
		return 1;
	}
	return 0;
}

// 0x8000D1D8
int TObjEnemy::tickAppear()
{
	hatchStep(&mAppearTimer);
	if (hatchDone(&mAppearTimer)) {
		hatchClear(&mAppearTimer);
		mHitFlags |= HIT_DONE;
		return 1;
	}
	return 0;
}

// 0x8000D238
void TObjEnemy::endAppear(int* step)
{
	TObject* obj = mAppearObj;
	if (obj != 0) {
		obj->mFlags |= 1;
		mAppearObj = 0;
	}
	if (mCollider != 0) {
		colliEnable(mCollider);
	}
	mEnemyFlags &= ~ENEMY_APPEARING;
	hatchClear(&mAppearTimer);
}

// 0x8000D2A4
void* TObjEnemy::allocPose(void* model)
{
	return actorCreate(model);
}

// 0x8000D2C8
void TObjEnemy::freePose(void** pose)
{
	actorFree(pose);
}

// 0x8000D2EC
void TObjEnemy::applyPose(void* pose, void* anim)
{
	actorApplyMotion(pose, anim);
}

// 0x8000D314
void TObjEnemy::applyPoseFaded(void* pose, f32 rate, void* anim, f32* timer)
{
	gjPushUnitMatrix();
	applyPose(pose, anim);
	njPopMatrixEx();
	if (hatchActive(timer) != 0) {
		hatchPose(timer, (GjActor*)pose, rate);
	}
}

// 0x8000D38C
void TObjEnemy::setFrameKey(const EnemyFrameKey* table)
{
	mFrameKey = enemyFrameKey(table, motionCtrlFrame(&mAnim));
}

// 0x8000D3CC
void TObjEnemy::clearFrameKey()
{
	mFrameKey = 0;
}

// 0x8000D3D8
void TObjEnemy::updateArea()
{
	mArea = lightSetFindArea(&mBody.mPos);
}

// 0x8000D40C
void TObjEnemy::useArea()
{
	lightSetSelect(mArea);
}

// 0x8000D430
void TObjEnemy::clearHitFlags()
{
	mHitFlags = 0;
}

// 0x8000D43C
int TObjEnemy::isDone()
{
	return mHitFlags & HIT_DONE;
}

// 0x8000D448
int TObjEnemy::wasHitHard()
{
	return mHitFlags & (HIT_KIND3 | HIT_KIND2 | HIT_KIND4);
}

// 0x8000D454
int TObjEnemy::wasHitKind2()
{
	return mHitFlags & HIT_KIND2;
}

// 0x8000D460
int TObjEnemy::wasHitKind3()
{
	return mHitFlags & HIT_KIND3;
}

// 0x8000D46C
int TObjEnemy::isDead()
{
	return mHitFlags & HIT_DEAD;
}

// 0x8000D478
int TObjEnemy::wasHitKind1()
{
	return mHitFlags & HIT_KIND1;
}

// 0x8000D484
int TObjEnemy::isActionDone(int action)
{
	u8 done = 0;
	if (action == mAction && isDone() != 0) {
		done = 1;
	}
	return done;
}

// 0x8000D4CC
void TObjEnemy::setDone()
{
	mHitFlags |= HIT_DONE;
}

// 0x8000D4DC
int TObjEnemy::waitMotion()
{
	return waitMotion((MotionPlayer*)motionCtrlGetPlayer(&mAnim));
}

// 0x8000D518
int TObjEnemy::waitMotion(MotionPlayer* motion)
{
	if (motion->isDone()) {
		mHitFlags |= HIT_DONE;
		return 1;
	}
	return 0;
}

// 0x8000D568
// Slot _A4
int TObjEnemy::getDamage(EnemyHit* hit)
{
	return hit->mInfo.mDamage;
}

// 0x8000D570
// Slot _A8
int TObjEnemy::getHitKind(const void*, HitInfo* info)
{
	return info->mDamage <= 0 ? 0 : 3;
}

// 0x8000D58C
// Sends message 0x2A to anything newly touching us whose shape can hurt
void TObjEnemy::updateTouches()
{
	EnemyTouchList now;
	touchListInit(&now);
	if (mCollider != 0) {
		u32 id;
		for (ColliContact* t = mCollider->mContacts; t != 0; t = t->mNext) {
			if (colliContactHasBit8((ColliContact*)t) != 0) {
				id = colliContactOwner(t);
				touchListAdd(&now, id);
				if (touchListHas(&mTouching, id) == 0 && (colliContactShape(t)->mFlags & 0x400) != 0) {
					TObjAction* other = findObjById(id);
					if (other != 0) {
						EnemyMsg msg;
						msg.mType = 0x2A;
						msg.mId = mObjId;
						other->sendToHandle((ObjMsg*)&msg);
					}
				}
			}
		}
	}
	mTouching = now;
}

// 0x8000D67C
// Slot _AC
int TObjEnemy::showsHitEffect()
{
	return 1;
}

// 0x8000D684
// Asks everything touching us to hit us and keeps the hit with the highest kind
int TObjEnemy::takeHits(EnemyHit* hit, EnemyContact* contact)
{
	ColliContact* bestTouch;
	EnemyHit best;
	bestTouch = 0;
	enemyHitInit(&best);

	if (isIntangible() == 0 && mCollider != 0) {
		for (ColliContact* t = mCollider->mContacts; t != 0; t = t->mNext) {
			if (colliContactHasBit8((ColliContact*)t) == 0) {
				colliContactHasBit40((ColliContact*)t);
			} else {
				u32 id = colliContactOwner(t);
				if (cooldownListHas(&mCooldown, id) == 0) {
					TObjAction* other = findObjById(id);
					if (other != 0) {
						HitInfo info;
						if (other->onHit(this, colliContactTag((ColliContact*)t), (int*)&info) == 0) {
							colliContactHasBit40((ColliContact*)t);
						} else if (info.mDamage > 0) {
							int kind = getHitKind(t, &info);
							if (best.mKind < kind) {
								best.mKind = kind;
								bestTouch = t;
								best.mObjId = id;
								best.mHolderId = other->mHolderId;
								best.mInfo = info;
							}
						}
					}
				}
			}
		}
	}

	cooldownListUpdate(&mCooldown);
	updateTouches();
	if (bestTouch != 0) {
		HitInfo unused;
		cooldownListAdd(&mCooldown, best.mObjId);
		colliContactTag((ColliContact*)bestTouch);
		*hit = best;
		hit->mShape = bestTouch->mShape;

		contact->mPos = bestTouch->mPos;
		EnemyShape* shape = (EnemyShape*)colliGetShape(mCollider, bestTouch->mShape);
		if (shape != 0) {
			contact->mCenter = shape->mCenter;
			Vec3f dir;
			dir = contact->mPos;
			njSubVector(&dir, &contact->mCenter);
			if (njUnitVector(&dir) < gUnitVectorMin) {
				dir = gVecZero;
			}
			gjMulAddVector(0.5f + shape->mRadius, &contact->mSurface, &contact->mCenter, &dir);
		} else {
			contact->mSurface = contact->mPos;
			contact->mPos.z -= 1.0f;
			contact->mCenter = contact->mPos;
			contact->mCenter.z -= 2.0f;
		}

		if ((colliContactShape(bestTouch)->mFlags & 0x400) != 0 && showsHitEffect() != 0) {
			seStartAt(0x95, &bestTouch->mPos);
		}
		return 1;
	}
	return 0;
}

// 0x8000DA00
int TObjAction::onHit(const TObjAction*, int, int*) { return 0; }

// 0x8000DA08
// Slot _88
int TObjEnemy::isIntangible()
{
	return mEnemyFlags & ENEMY_INTANGIBLE;
}

// 0x8000DA14
void TObjEnemy::applyDamage(EnemyHit* hit)
{
	mHealth -= getDamage(hit);
	if (mHealth <= 0) {
		mHealth = 0;
		mHitFlags |= HIT_DEAD;
		return;
	}

	switch (hit->mKind) {
	case 0:
		break;
	case 1:
		mHitFlags |= HIT_KIND1;
		break;
	case 2:
		mHitFlags |= HIT_KIND2;
		break;
	case 3:
		mHitFlags |= HIT_KIND3;
		break;
	case 4:
		mHitFlags |= HIT_KIND4;
		break;
	}
}

// 0x8000DAEC
// What this enemy does to something it hits
void TObjEnemy::fillAttack(void* where, HitInfo* out)
{
	out->mDamage = mHit.mInfo.mDamage;
	out->mFlags.clear();
	out->mFlags.setBit(2);
	out->_14 = mHit.mInfo._14;
	if (where != 0) {
		out->mFlags.setBit(0xC);
		out->mPos = *(Vec3f*)((u8*)where + 0x10);
	}
}

// 0x8000DB6C
void TObjEnemy::turnTo(int angle, int rate)
{
	bodyTurnTo(&mBody, angle, rate);
}

// 0x8000DB90
void TObjEnemy::faceToward(Vec3f* target, Vec3f* from, void* arg)
{
	bodyFace(&mBody, target, from, (int)arg);
}

// 0x8000DBB4
void TObjEnemy::getFlatPush(Vec3f* out)
{
	colliFlatPush(mCollider, out);
}

// 0x8000DBD8
void TObjEnemy::moveAlong(f32 dist, Vec3f* pos, Vec3f* dir, Vec3f* out)
{
	flatSteer(dist, pos, dir, out);
}

// 0x8000DC04
void TObjEnemy::getPush(Vec3f* out)
{
	colliSumPush(mCollider, out);
}

// 0x8000DC28
extern "C" void initEnemyFilter(SlopeFilter* filter)
{
	filter->mStep       = gEnemyFilterX;
	filter->mUpNormal   = gEnemyFilterY;
	filter->mDownNormal = gEnemyFilterZ;
}

// 0x8000DC44
void TObjEnemy::setAirborne(u32 result)
{
	if ((result & 2) != 0) {
		mEnemyFlags |= ENEMY_AIRBORNE;
	} else {
		mEnemyFlags &= ~ENEMY_AIRBORNE;
	}
}

// 0x8000DC6C
int TObjEnemy::moveVertical(f32 dy, Vec3f* pos, void* out)
{
	u32 result = dropToGround(dy, pos, (ObjShadow*)out);
	setAirborne(result);
	return result & 1;
}

// 0x8000DCB8
int TObjEnemy::probeWall(f32 dist, Vec3f* from, Vec3f* to, void* a, void* b, void* c)
{
	return ::probeWall(dist, from, to, (f32*)a, (Vec3f*)b, (Vec3f*)c) & 4;
}

// 0x8000DCF0
int TObjEnemy::collide(f32 dist, Vec3f* from, Vec3f* to, void* out)
{
	GroundFilter filter;
	return collide(dist, from, to, &filter, out);
}

// 0x8000DD24
int TObjEnemy::collideFiltered(f32 dist, Vec3f* from, Vec3f* to, void* out)
{
	SlopeFilter filter;
	initEnemyFilter(&filter);
	return collide(dist, from, to, &filter, out);
}

// 0x8000DDA0
int TObjEnemy::collide(f32 dist, Vec3f* from, Vec3f* to, void* filter, void* out)
{
	u32 result = moveBody(dist, from, to, (GroundFilter*)filter, (ObjShadow*)out);
	setAirborne(result);
	return result & 4;
}

// 0x8000DDF4
int TObjEnemy::probeLine(Vec3f* from, Vec3f* to, void* a, void* b)
{
	return nearXZ(from, to, a, b);
}

// 0x8000DE24
void TObjEnemy::resetLink()
{
	setLinkFree((SetLink*)&mLink);
}

// 0x8000DE48
// The first call tells the mission and scores
void TObjEnemy::killed(int timed)
{
	if ((mEnemyFlags & ENEMY_KILLED) == 0) {
		EnemyMsg msg;
		msg.mType = 0x96;
		sendGameMsg((ObjMsg*)&msg);
		mEnemyFlags |= ENEMY_KILLED;
		scoreHit(mHit.mInfo._14);
	}

	if ((mEnemyFlags & ENEMY_FROM_DESC) != 0) {
		setLinkDone((SetLink*)&mLink);
	} else if (timed != 0) {
		setLinkTimed((SetLink*)&mLink, 0x708);
	} else {
		setLinkForever((SetLink*)&mLink);
	}
	notifySetLinkDeath((SetLink*)&mLink, (void*)this);
}

// 0x8000DEE8
int TObjEnemy::isInView(f32 far, f32 radius, void* pos, void* arg)
{
	return cameraIsVisible(far, radius, (const Vec3f*)pos, (Vec3f*)arg);
}

// 0x8000DF10
void TObjEnemy::creditKiller()
{
	int id;
	if (isPlayerId(mHit.mObjId) != 0) {
		id = mHit.mObjId;
	} else if (isPlayerId(mHit.mHolderId) != 0) {
		id = mHit.mHolderId;
	} else {
		id = 0;
	}
	periVibratePlay(id, 0);
}

// 0x8000DF74
int TObjEnemy::lockOn()
{
	if ((mEnemyFlags & ENEMY_LOCKED_ON) == 0 && eventStartById(mObjId, 0) != 0) {
		mEnemyFlags |= ENEMY_LOCKED_ON;
		return 1;
	}
	return 0;
}

// 0x8000DFD4
void TObjEnemy::unlock()
{
	if ((*(volatile u32*)&mEnemyFlags & ENEMY_LOCKED_ON) != 0) {
		mEnemyFlags &= ~ENEMY_LOCKED_ON;
		eventEnd();
	}
}

// 0x8000E00C
extern "C" int isPlayerObj(const TObjAction* obj)
{
	return ((TObjAction*)obj)->getKind() == 0;
}

// 0x8000E040
// Inlined from a range test whose upper limit of 4 never decides anything
extern "C" int isPlayerId(u16 id)
{
	u8 ok = 0;
	if (id == 0 && id < 4) {
		ok = 1;
	}
	return ok;
}

// 0x8000E060
extern "C" int isEnemyObj(const TObjAction* obj)
{
	return ((TObjAction*)obj)->getKind() == 2;
}

// 0x8000E098
extern "C" int isEggObj(const TObjAction* obj)
{
	return ((TObjAction*)obj)->getKind() == 1;
}

// 0x8000E0D0
extern "C" int isPlayerOrKind4(const TObjAction* obj)
{
	int kind = ((TObjAction*)obj)->getKind();
	u8 yes = 0;
	if (kind == 0 || kind == 4) {
		yes = 1;
	}
	return yes;
}

// 0x8000E118
extern "C" int divideBy5(int value)
{
	return value / 5;
}

// 0x8000E124
// The ground below a point
int probeGround(Vec3f* pos, Vec3f* outPos, Vec3f* outNormal)
{
	EggHits hits;
	int found = (int)meshColliProbe((EggHit*)&hits, pos, 1);
	if (found != 0) {
		if (outPos != 0) {
			*outPos = hits.mHits[0].mPos;
		}
		if (outNormal != 0) {
			*outNormal = hits.mHits[0].mNormal;
		}
	}
	return found;
}

// 0x8000E1CC
EggHit::EggHit() { }

// 0x8000E214
// The ground along a line
extern "C" int probeGroundLine(Vec3f* from, Vec3f* to, Vec3f* outPos, Vec3f* outNormal)
{
	EggHits hits;
	int found = meshColliProbeLine((EggHit*)&hits, (const Vec3f*)from, (const Vec3f*)to, 1);
	if (found != 0) {
		if (outPos != 0) {
			*outPos = hits.mHits[0].mPos;
		}
		if (outNormal != 0) {
			*outNormal = hits.mHits[0].mNormal;
		}
	}
	return found;
}

// 0x8000E2C4
extern "C" void drawModelLit(void* model)
{
	shadowAlphaBegin();
	drawActorDefault(model);
	shadowAlphaEnd();
}

// 0x8000E2FC
// Lighting for something drawn by hand with extras from a stage object and an EnemyTint if given
extern "C" void beginLit(void* stageObj, void* tint)
{
	DrawState light;
	((u32*)&light)[0] = 0;
	((u32*)&light)[8] = 0;
	DrawStateSlot* lights = getDrawStateSlot();
	light = *getDefaultDrawState();
	if (stageObj != 0) {
		setDrawAmbient((EnemyLight*)stageObj, &light);
	}
	if (tint != 0) {
		((EnemyTint*)tint)->addToLight((u32*)&light);
	}
	beginDrawState(lights, &light);
	shadowAlphaBegin();
}

// 0x8000E3C0
extern "C" void endLit()
{
	endDrawState(getDrawStateSlot());
	shadowAlphaEnd();
}

// 0x8000E3E8
extern "C" void drawLit(void* stageObj, void* tint, void* model)
{
	beginLit(stageObj, tint);
	gjDrawActor((GjActor*)model);
	endLit();
}

// 0x8000E420
extern "C" void initBossBar()
{
	bossConsoleGet();
}

// 0x8000E440
extern "C" void setBossBar(int max, int value)
{
	EnemyMsg msg;
	msg.mType = 0xB9;
	msg.mRatio = (f32)value / (f32)max;
	bossConsoleMsg((BossConsoleMsg*)&msg);
}

// 0x8000E4A4
extern "C" void hideBossBar()
{
	EnemyMsg msg;
	msg.mType = 0xBA;
	bossConsoleMsg((BossConsoleMsg*)&msg);
}

// 0x8000E4D0
void TObjEnemy::slain()
{
	killed(0);
}

// 0x8000E4F4
extern "C" void sendBossDefeated(TObjEnemy* boss) // boss is not read
{
	EnemyMsg msg;
	msg.mType = 0x90;
	sendGameMsg((ObjMsg*)&msg);
}

// 0x8000E520
void TObjEnemy::setCantAct()
{
	mEnemyFlags |= ENEMY_CANT_ACT;
}

// 0x8000E530
void TObjEnemy::clearCantAct()
{
	mEnemyFlags &= ~ENEMY_CANT_ACT;
}

// 0x8000E540
extern "C" void pulseBossView(const Vec3f* pos, f32 value)
{
	camShakeNearSoft(value, pos);
}

// 0x8000E560
extern "C" void setBossView(f32 a, f32 b, f32 c)
{
	camSetCurveLift(a, 0);
	camSetChaseOffset(b, c, 0);
}

// 0x8000E5AC
extern "C" void resetBossView()
{
	camResetCurveLift(0);
	camResetChaseOffset(0);
}

// 0x8000E5D8
// Falling 150 below home is death
void TObjEnemy::checkFellOut()
{
	if (mBody.mPos.y - mHome.y < -150.0f) {
		mHealth = 0;
		enemyHitInit(&mHit);
		mHitFlags |= HIT_DEAD;
	}
}

// 0x8000E634
// One drop is a random one of 7 kinds
extern "C" void spawnDrop(void* pos, int count, int flag)
{
	if (count > 0) {
		if (count == 1) {
			int kind = (int)(7.0f * (3.0517578e-5f * rand()));
			if (flag == 0) {
				createTObjEggFeedAt(kind, (const Vec3f*)pos, 0);
			} else {
				createTObjEggFeedAt(kind, (const Vec3f*)pos, 1);
			}
		} else if (flag == 0) {
			spawnDropRun((Vec3f*)pos, count);
		} else {
			spawnDropRunEx((Vec3f*)pos, count, 1);
		}
	}
}

// 0x8000E6EC
// Both values come from a table the stage sets
extern "C" void spawnDropA(void* pos)
{
	spawnDrop(pos, gEnemyParams->mDropACount, gEnemyParams->mDropAFlag);
}

// 0x8000E718
extern "C" void spawnDropB(void* pos)
{
	spawnDrop(pos, gEnemyParams->mDropBCount, gEnemyParams->mDropBFlag);
}

// 0x8000E744
void TObjEnemy::noOp() { }

// 0x8000E748
int TObjEnemy::getKind()
{
	return 2;
}

// 0x8000E750
int TObjEnemy::getRotY()
{
	return mBody.mRot.y;
}

// 0x8000E758
Vec3f* TObjEnemy::getPos()
{
	return &mBody.mPos;
}

// 0x8000E760
void TObjEnemy::syncCollider() { }

// 0x8000E764
int TObjEnemy::isAirborne()
{
	return mEnemyFlags & ENEMY_AIRBORNE;
}

// 0x8000E770
int TObjEnemy::isAppearing()
{
	return mEnemyFlags & ENEMY_APPEARING;
}

// 0x8000E77C
int TObjEnemy::isFlipping()
{
	return mEnemyFlags & ENEMY_FLIPPING;
}

// 0x8000E788
int TObjEnemy::isThrown()
{
	return mEnemyFlags & ENEMY_THROWN;
}

// 0x8000E794
int TObjEnemy::vfn_98()
{
	return mEnemyFlags & 0x40;
}

// 0x8000E7A0
int TObjEnemy::vfn_9C()
{
	return !(mEnemyFlags & ENEMY_BLOCKED);
}

// 0x8000E7B4
int TObjEnemy::vfn_A0()
{
	return !(mEnemyFlags & ENEMY_BLOCKED);
}

// TObjAction's defaults that sit in this range

// 0x8000E7C8
void TObjAction::getNodeMatrix(int, f32*) { }

// 0x8000E7CC
f32 TObjAction::vfn_40() { return 0.0f; }

// 0x8000E7D4
int TObjAction::vfn_48(TObjAction*) { return 0; }

// 0x8000E7DC
int TObjAction::vfn_50() { return 0; }

// 0x8000E7E4
f32 TObjAction::getRadiusV() { return 0.0f; }
