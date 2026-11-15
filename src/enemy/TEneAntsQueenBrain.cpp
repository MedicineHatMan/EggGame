#include "enemy/TEneAntsQueen.h"
#include "enemy/EnemyFx.h"
#include "object/ObjAction.h"
#include "stl/math.h"
#include "enemy/boss/EneBoss.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "animal/RacerMover.h"

// 0x80094110
void AntsQueenBrain::update(TEneAntsQueen* owner)
{
	mOwner = owner;
	mNext = -1;
	if (checkInterrupt() == 0) {
		onPhase(mState, 2);
	}
	onPhase(mState, 1);
	if (mNext != -1) {
		mOwner->setAction(mNext);
	}
}

// 0x800941A8
AntsQueenBrainBase::AntsQueenBrainBase()
{
	mOwner = 0;
}

// 0x800941DC
AntsQueenBrainBase::~AntsQueenBrainBase() { }

// 0x80094238
AntsQueenHuntBrain* AntsQueenHuntBrain::create(TEneAntsQueen* owner)
{
	return new AntsQueenHuntBrain(owner);
}

// 0x80094280
AntsQueenHuntBrain::AntsQueenHuntBrain(TEneAntsQueen* owner)
{
	mSight.reset();
	mMemory.reset();
	mSummonTimer = 0;
	mStep = 0;
	mTimer = 0;
	mOwner = owner;
	mNext = -1;
	mState = STATE_APPEAR;
	onPhase(mState, 0);
	if (mNext != -1) {
		mOwner->setAction(mNext);
	}
}

// 0x8009433C
AntsQueenHuntBrain::~AntsQueenHuntBrain() { }

// 0x800943A4
// A hard hit stuns her or flips her when she isn't throwing and every ant goes after who hit her
int AntsQueenHuntBrain::checkInterrupt()
{
	if (mOwner->isDead() != 0 && mState != STATE_DIE) {
		changeState(STATE_DIE);
		return 1;
	}
	if (mOwner->wasHitHard() != 0) {
		if (mOwner->isNotThrowing() != 0) {
			if (mOwner->wasHitKind2() != 0) {
				changeState(STATE_STUN);
			} else {
				changeState(STATE_FLIP);
			}
		} else {
			mOwner->stunEffects();
		}
		mMemory.setChase(mOwner->getHitPlayer());
		mOwner->alertAnts(mOwner->getHitPlayer());
	}
	return 0;
}

// 0x80094518
// A random point 25 to 35 from home
void AntsQueenHuntBrain::pickTarget()
{
	Vec3f home;
	Vec3f target;
	home = mOwner->mHome;
	randomPointInRing(25.0f, 35.0f, &home, &target);
	mOwner->setTarget(&target);
}

// 0x8009457C
// Away from the player between the ways from it to home and to her to where that meets the ring 60 from home
void AntsQueenHuntBrain::pickRetreat()
{
	Vec3f player = mSight.mTarget;
	Vec3f pos = *mOwner->getPos();
	Vec3f home;
	mOwner->getHome(&home);
	Vec3f dir = home;
	njSubVector(&dir, &player);
	dir.y = 0.0f;
	if (njUnitVector(&dir) < gUnitVectorMin) {
		dir = gVecZero;
	}
	Vec3f away = pos;
	njSubVector(&away, &player);
	away.y = 0.0f;
	if (njUnitVector(&away) < gUnitVectorMin) {
		away = gVecZero;
	}
	njAddVector(&dir, &away);
	if (njUnitVector(&dir) < gUnitVectorMin) {
		dir = gVecZero;
	}
	f32 dx = pos.x - home.x;
	f32 dz = pos.z - home.z;
	f32 distSq = dx * dx + dz * dz;
	if (distSq > 3364.0f) {
		f32 scale = (f32)__frsqrte(distSq) * 58.0f;
		distSq = 3364.0f;
		dx *= scale;
		dz *= scale;
	}
	f32 t = quadRoot(1.0f, 2.0f * dir.x * dx + 2.0f * dir.z * dz, distSq - 3600.0f);
	pos.y = player.y;
	pos.x = dir.x * t + (home.x + dx);
	pos.z = dir.z * t + (home.z + dz);
	mOwner->setTarget(&pos);
}

// 0x800947B8
// Follows the player she remembers or looks out to 170 nearer than 70 all round
int AntsQueenHuntBrain::look()
{
	int seen = 0;
	mMemory.tick();
	if (mMemory.isReady() != 0) {
		if (mMemory.isChasing() != 0) {
			Vec3f* pos;
			seen = mSight.track(mMemory.getPlayer(), mOwner->getPos(), mOwner->getRotY());
		} else {
			seen = mSight.search(28900.0f, 4900.0f, mOwner->getPos(), mOwner->getRotY(), 0x8000, 0xFF49);
		}
	}
	return seen;
}

// 0x800948D0
int AntsQueenHuntBrain::canSummon()
{
	return mOwner->canSummon();
}

// 0x800948F4
// Every 180 frames she may call more
int AntsQueenHuntBrain::tickSummon()
{
	mSummonTimer--;
	if (mSummonTimer <= 0) {
		mSummonTimer = 180;
		return canSummon();
	}
	return 0;
}

// 0x8009493C
void AntsQueenHuntBrain::summonOrWander()
{
	if (canSummon() != 0) {
		changeState(STATE_SUMMON);
	} else {
		changeState(STATE_WANDER);
	}
}

// 0x800949F4
// The free slot nearest the way she faces
void AntsQueenHuntBrain::pickSlot()
{
	AntLink* link;
	int rot = mOwner->getRotY();
	AntLink* ants = mOwner->getAnts();
	int count = mOwner->getMaxAnts();
	link = ants->mNext;
	int bestDiff = 0x7FFFF;
	int best = -1;
	int bestAngle = 0;
	for (int i = 0; i < count; i++) {
		int free = 0;
		if (link != 0) {
			if (link->mSlot == i) {
				link = link->mNext;
			} else if (link->mSlot > i) {
				free = 1;
			} else {
				link = link->mNext;
				i--;
			}
		} else {
			free = 1;
		}
		if (free) {
			int angle = (i << 16) / count;
			int diff = angleDist(rot, angle);
			if (diff < bestDiff) {
				bestDiff = diff;
				best = i;
				bestAngle = angle;
			}
		}
	}
	if (best >= 0) {
		mOwner->setSlot(bestAngle, best);
	}
}

// 0x80094AFC
// Slot _0C
void AntsQueenHuntBrain::onPhase(int state, int phase)
{
	switch (state) {
	case STATE_APPEAR:
		phaseAppear(phase);
		break;
	case STATE_WANDER:
		phaseWander(phase);
		break;
	case STATE_SUMMON:
		phaseSummon(phase);
		break;
	case STATE_ATTACK:
		phaseAttack(phase);
		break;
	case STATE_STUN:
		phaseStun(phase);
		break;
	case STATE_FLIP:
		phaseFlip(phase);
		break;
	case STATE_DIE:
		phaseDie(phase);
		break;
	}
}

// 0x80094B88
void AntsQueenHuntBrain::phaseAppear(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAntsQueen::ACT_APPEAR;
		break;
	case 2:
		if (mOwner->isAppearDone() != 0) {
			summonOrWander();
		}
		break;
	case 3:
		break;
	}
}

// 0x80094BF0
// Walks between random points resting at each
void AntsQueenHuntBrain::phaseWander(int phase)
{
	switch (phase) {
	case 0:
		pickTarget();
		mStep = randomInt(300.0f, 120.0f);
		mNext = TEneAntsQueen::ACT_WALK;
		mTimer = 0;
		break;
	case 2:
		if (tickSummon() != 0) {
			changeState(STATE_SUMMON);
		} else if (look() != 0) {
			mOwner->startEffect();
			changeState(STATE_ATTACK);
		}
		break;
	case 1:
		mStep--;
		if (mStep < 0) {
			pickTarget();
			mStep = randomInt(300.0f, 120.0f);
			if (mTimer != 0) {
				mTimer = 0;
				mNext = TEneAntsQueen::ACT_WALK;
			}
		} else if (mOwner->hasArrived() != 0) {
			mTimer = 1;
			mNext = TEneAntsQueen::ACT_IDLE;
		}
		break;
	case 3:
		break;
	}
}

// 0x80094D74
// Calls ants until the ring is full
void AntsQueenHuntBrain::phaseSummon(int phase)
{
	switch (phase) {
	case 0:
		mStep = 0;
		break;
	case 2:
		if (mStep == 2 && mOwner->isSummonDone() != 0) {
			changeState(STATE_WANDER);
		}
		break;
	case 1:
		switch (mStep) {
		case 0:
			mNext = TEneAntsQueen::ACT_SUMMON;
			pickSlot();
			mStep = 1;
			break;
		case 1:
			if (mOwner->hasSummoned() != 0) {
				if (canSummon() != 0) {
					mStep = 0;
				} else {
					mStep = 2;
				}
			}
			break;
		}
		break;
	case 3:
		break;
	}
}

// 0x80094E90
// Throws then backs off a player nearer than 90 or waits for 180 frames and throws again
void AntsQueenHuntBrain::phaseAttack(int phase)
{
	switch (phase) {
	case 0:
		mStep = 2;
		mOwner->setAim(&mSight.mTarget, mSight.mAngle);
		mNext = TEneAntsQueen::ACT_THROW;
		break;
	case 2:
		if (look() == 0) {
			mMemory.startCooldown();
			summonOrWander();
		}
		break;
	case 1:
		switch (mStep) {
		case 2:
			if (mOwner->isThrowDone() != 0) {
				mTimer = 180;
				if (mSight.mDistSq < 8100.0f) {
					mStep = 3;
					pickRetreat();
					mNext = TEneAntsQueen::ACT_RETREAT;
				} else {
					mStep = 1;
					mNext = TEneAntsQueen::ACT_IDLE;
				}
			}
			break;
		case 3:
			pickRetreat();
			break;
		case 1:
			if (mSight.mDistSq < 8100.0f) {
				mStep = 3;
				mNext = TEneAntsQueen::ACT_RETREAT;
			}
			break;
		}
		if (mStep == 3 || mStep == 1) {
			mTimer--;
			if (mTimer <= 0) {
				mStep = 2;
				mOwner->setAim(&mSight.mTarget, mSight.mAngle);
				mNext = TEneAntsQueen::ACT_THROW;
			}
		}
		break;
	case 3:
		break;
	}
}

// 0x80095008
void AntsQueenHuntBrain::phaseStun(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAntsQueen::ACT_STUN;
		break;
	case 2:
		if (mOwner->isStunDone() != 0) {
			summonOrWander();
		}
		break;
	case 3:
		break;
	}
}

// 0x80095070
void AntsQueenHuntBrain::phaseFlip(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAntsQueen::ACT_FLIP;
		break;
	case 2:
		if (mOwner->isFlipDone() != 0) {
			summonOrWander();
		}
		break;
	case 3:
		break;
	}
}

// 0x800950D8
void AntsQueenHuntBrain::phaseDie(int phase)
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
