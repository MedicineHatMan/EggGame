#include "object/ObjAction.h"
#include "enemy/TEneAm06.h"
#include "egg/EggCreature.h"
#include "enemy/boss/EneBoss.h"
#include "Ninja/ninja.h"
#include "object/TObjPlayer.h"
#include "player/Player.h"
struct TObjAction;
struct ColliShape;
struct ColliContact;
struct TObjPlayer;

// 0x80048E20
// Empty __construct_array calls it for each spot
Am06Spot::Am06Spot()
{
}

// 0x80048DD8
Am06Seen::Am06Seen()
{
}

// 0x80048AD0
void Am06Seen::reset()
{
	mCount = 0;
	for (int i = 0; i < 4; i++) {
		mSpot[i].mId    = -1;
		mSpot[i].mPos.x = 0.0f;
		mSpot[i].mPos.y = 0.0f;
		mSpot[i].mPos.z = 0.0f;
	}
}

// 0x80048B0C
void Am06Seen::add(s16 id, TObjAction* obj)
{
	if (mCount < 4) {
		mSpot[mCount].mId  = id;
		Vec3f* pos         = obj->getPos();
		mSpot[mCount].mPos = *pos;
		mCount++;
	}
}

// 0x80048B94
int Am06Seen::find(s16 id, Vec3f* out)
{
	int found = 0;
	for (int i = 0; i < mCount; i++) {
		if ((u16)id == (u16)mSpot[i].mId) {
			found = 1;
			*out  = mSpot[i].mPos;
			break;
		}
	}
	return found;
}

// 0x80048A54
// The id each player's vfn_A0 returns TChrPlayer's is ObjRef::getId of the reference list at +0x10C
static int collectPlayerIds(u16* out)
{
	int count = 0;
	for (int i = 0; i < 4; i++) {
		TObjPlayer* player = findPlayer((u16)i);
		if (player == 0) {
			continue;
		}
		u16 id = player->vfn_A0();
		if (id != 0xFFFF) {
			out[count] = id;
			count++;
		}
	}
	return count;
}

// 0x80048BF4
// Installs the base vtable then this one
Am06BrainBase::Am06BrainBase()
{
	mOwner = 0;
}

// 0x80048C28
Am06BrainBase::~Am06BrainBase()
{
}

// 0x800489BC
// Slot _14 Phase 2 is the check and a hit that returned from checkInterrupt skips it
void Am06Brain::update(TEneAm06* owner)
{
	mOwner = owner;
	mNext  = -1;
	if (checkInterrupt() == 0) {
		onPhase(mState, 2);
	}
	onPhase(mState, 1);
	if (mNext != -1) {
		mOwner->setAction(mNext);
	}
}

// 0x80048C84
// operator new can return null
Am06SnakeBrain* Am06SnakeBrain::create(TEneAm06* owner)
{
	return new Am06SnakeBrain(owner);
}

// 0x80048CCC
Am06SnakeBrain::Am06SnakeBrain(TEneAm06* owner)
{
	mNear.reset();
	mLook.reset();
	mCooldown = 0;
	mList[0].reset();
	mList[1].reset();
	mSide   = 0;
	mTarget = 0xFFFF;
	mStun   = 0;
	mAgain  = 0;
	_10C    = 0;
	mOwner  = owner;
	mNext   = -1;
	mState  = STATE_APPEAR;
	onPhase(mState, 0);
	if (mNext != -1) {
		mOwner->setAction(mNext);
	}
}

// 0x80048E24
// The sights go then the base
Am06SnakeBrain::~Am06SnakeBrain()
{
}

// 0x80048E98
// Slot _08 A kind 3 hit changes nothing but it does skip this frame's phase 2
int Am06SnakeBrain::checkInterrupt()
{
	int changed = 1;
	if (mOwner->isDead() != 0 && mState != STATE_DIE) {
		changeState(STATE_DIE);
	} else if (mOwner->wasHitKind3() == 0) {
		if (mOwner->wasHitKind1() != 0) {
			if (mOwner->isAimed() != 0) {
				changeState(STATE_DOWN);
			} else {
				mOwner->noteCrush();
			}
		} else {
			changed = 0;
		}
	}
	return changed;
}

// 0x80048FA8
// Anyone within 200
int Am06SnakeBrain::nearSight()
{
	return mNear.findInRange(40000.0f, mOwner->getPos());
}

// 0x80049000
// 65 degrees either side out to 80 or 125 degrees either side within 40
int Am06SnakeBrain::look()
{
	if (mCooldown <= 0) {
		return mLook.search(6400.0f, 1600.0f, mOwner->getPos(), mOwner->getRotY(), 0x5C71, 0xB1C7);
	}
	mCooldown--;
	return 0;
}

// 0x800490A0
// A contact the head does not already know as a player that has moved toward the snake since last frame and changed
// height by at least half a unit
u16 Am06SnakeBrain::pick(Am06Seen* fresh, Am06Seen* previous)
{
	u16 result    = 0xFFFF;
	fresh->mCount = 0;
	u16 ids[4];
	int count  = collectPlayerIds(ids);
	void* head = mOwner->headShape();
	for (ColliContact* touch = ((CharaColli*)head)->mContacts; touch != 0; touch = touch->mNext) {
		u32 raw   = colliContactOwner(touch);
		u16 id    = (u16)raw;
		int known = 0;
		for (int i = 0; i < count; i++) {
			if (id == ids[i]) {
				known = 1;
				break;
			}
		}
		if (known != 0) {
			continue;
		}
		ColliShape* shape = colliContactShape(touch);
		if ((shape->mFlags & 0x10) != 0) {
			continue;
		}
		TObjAction* obj = findObjById(id);
		if (obj != 0) {
			fresh->add((s16)raw, obj);
		}
	}

	Vec3f origin = *mOwner->getPos();
	for (int i = 0; i < fresh->mCount; i++) {
		Vec3f old;
		if (previous->find(fresh->mSpot[i].mId, &old) == 0) {
			continue;
		}
		Vec3f spot;
		spot = fresh->mSpot[i].mPos;
		njSubVector(&old, &spot);
		if ((f32)__fabs(old.y) < 0.5f) {
			continue;
		}
		njSubVector(&spot, &origin);
		if (njInnerProduct(&spot, &old) > 0.0f) {
			result = (u16)fresh->mSpot[i].mId;
		}
	}
	return result;
}

// 0x80049258
// Slot _0C
void Am06SnakeBrain::onPhase(int state, int phase)
{
	switch (state) {
	case STATE_APPEAR:
		phaseAppear(phase);
		break;
	case STATE_GLOW:
		phaseGlow(phase);
		break;
	case STATE_COIL:
		phaseCoil(phase);
		break;
	case STATE_SWAY:
		phaseSway(phase);
		break;
	case STATE_REAR:
		phaseRear(phase);
		break;
	case STATE_STRIKE:
		phaseStrike(phase);
		break;
	case STATE_BITE:
		phaseBite(phase);
		break;
	case STATE_STAGGER:
		phaseStagger(phase);
		break;
	case STATE_SPIT:
		phaseSpit(phase);
		break;
	case STATE_FLINCH:
		phaseFlinch(phase);
		break;
	case STATE_FLIP:
		phaseFlip(phase);
		break;
	case STATE_DIE:
		phaseDie(phase);
		break;
	case STATE_DOWN:
		phaseDown(phase);
		break;
	}
}

// 0x8004932C
void Am06SnakeBrain::phaseAppear(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAm06::ACT_APPEAR;
		break;
	case 2:
		if (mOwner->isAppearDone() != 0) {
			changeState(STATE_GLOW);
		}
		break;
	case 3:
		break;
	}
}

// 0x800493CC
// Waits until someone is within 200 then coils
void Am06SnakeBrain::phaseGlow(int phase)
{
	switch (phase) {
	case 0:
		mNext  = TEneAm06::ACT_GLOW;
		mAgain = 0;
		break;
	case 2:
		if (nearSight() != 0) {
			changeState(STATE_COIL);
		}
		break;
	case 3:
		break;
	}
}

// 0x80049470
void Am06SnakeBrain::phaseCoil(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAm06::ACT_COIL;
		mOwner->stopEffect();
		break;
	case 2:
		if (mOwner->isCoiled() != 0) {
			changeState(STATE_SWAY);
		}
		break;
	case 3:
		break;
	}
}

// 0x80049518
// A player in the cone is a strike
void Am06SnakeBrain::phaseSway(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAm06::ACT_SWAY;
		mSide = 0;
		mList[1].reset();
		mTarget = 0xFFFF;
		break;
	case 2:
		if (look() != 0) {
			changeState(STATE_STRIKE);
		} else if (nearSight() != 0) {
			if (mSide == 0) {
				mTarget = pick(&mList[0], &mList[1]);
				mSide   = 1;
			} else {
				mTarget = pick(&mList[1], &mList[0]);
				mSide   = 0;
			}
			if (mTarget != 0xFFFF) {
				mOwner->setGrabId(mTarget);
				changeState(STATE_BITE);
			}
			mOwner->setAim(mNear.mAngle);
		} else {
			changeState(STATE_REAR);
		}
		break;
	case 3:
		break;
	}
}

// 0x800496D0
// Rears until someone comes within 200 then goes back to swaying
void Am06SnakeBrain::phaseRear(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAm06::ACT_REAR;
		break;
	case 2:
		if (nearSight() != 0) {
			mOwner->stopEffect();
			changeState(STATE_SWAY);
		} else {
			if (mOwner->isReared() != 0) {
				changeState(STATE_GLOW);
			}
		}
		break;
	case 3:
		break;
	}
}

// 0x800497C8
// Aims the head at whoever is close and holds the cone's angle for the strike
void Am06SnakeBrain::phaseStrike(int phase)
{
	switch (phase) {
	case 0:
		mOwner->setHold(mLook.mAngle);
		mNext = TEneAm06::ACT_STRIKE;
		break;
	case 2:
		if (mOwner->isStruck() != 0) {
			mCooldown = 60;
			changeState(STATE_SWAY);
		}
		break;
	case 3:
		break;
	case 1:
		nearSight();
		mOwner->setAim(mNear.mAngle);
		break;
	}
}

// 0x80049894
// An accepted grab staggers it into the spit
void Am06SnakeBrain::phaseBite(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAm06::ACT_BITE;
		break;
	case 2:
		if (mOwner->isBitten() != 0) {
			if (mOwner->getGrab() == 1) {
				changeState(STATE_STAGGER);
			} else {
				changeState(STATE_SWAY);
			}
		}
		break;
	case 3:
		break;
	}
}

// 0x80049988
// The exit copies how long the spit should hold on
void Am06SnakeBrain::phaseStagger(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAm06::ACT_STAGGER;
		break;
	case 2:
		if (mOwner->isStaggerDone() != 0) {
			changeState(STATE_SPIT);
		}
		break;
	case 3:
		mStun = mOwner->stunFrames();
		break;
	}
}

// 0x80049A40
// Holds the grab for mStun frames
void Am06SnakeBrain::phaseSpit(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAm06::ACT_SPIT;
		mOwner->grab();
		mAgain = 0;
		break;
	case 2:
		if (mStun < 0 && mAgain == 0) {
			changeState(STATE_FLINCH);
		}
		break;
	case 1:
		mStun--;
		if (mStun <= 0) {
			mOwner->release();
		}
		mAgain = 0;
		if (mOwner->isFlipDone() != 0) {
			mNext  = TEneAm06::ACT_SPIT;
			mAgain = 1;
		} else if (mOwner->wasHitKind3() != 0) {
			mNext = TEneAm06::ACT_FLIP;
		}
		break;
	case 3:
		mOwner->release();
		break;
	}
}

// 0x80049B98
void Am06SnakeBrain::phaseFlinch(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAm06::ACT_FLINCH;
		break;
	case 2:
		if (mOwner->isFlinchDone() != 0) {
			changeState(STATE_SWAY);
		}
		break;
	case 3:
		break;
	}
}

// 0x80049C38
void Am06SnakeBrain::phaseFlip(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAm06::ACT_FLIP;
		break;
	case 2:
		if (mOwner->isFlipDone() != 0) {
			changeState(STATE_SWAY);
		}
		break;
	case 3:
		break;
	}
}

// 0x80049CD8
void Am06SnakeBrain::phaseDie(int phase)
{
	switch (phase) {
	case 0:
		mNext = mOwner->deathAction();
		break;
	case 1:
		break;
	case 2:
		break;
	}
}

// 0x80049D1C
void Am06SnakeBrain::phaseDown(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAm06::ACT_DOWN;
		break;
	case 2:
		if (mOwner->isDownDone() != 0) {
			changeState(STATE_SWAY);
		}
		break;
	case 3:
		break;
	}
}
