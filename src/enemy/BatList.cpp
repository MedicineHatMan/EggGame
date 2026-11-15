#include "enemy/BatList.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "core/TMainTask.h"

static inline f32 random01()
{
	return 3.0517578e-5f * rand();
}

// 0x800D9B20
BatList::BatList()
{
	mPrev  = 0;
	mNext  = 0;
	mCount = 0;
}

// 0x800D9B34
BatList::~BatList()
{
	unlink();
}

// 0x800D9B7C
// Goes in right after head
void BatList::link(BatList* head)
{
	mNext = head->mNext;
	if (mNext != 0) {
		mNext->mPrev = this;
	}
	mPrev       = head;
	head->mNext = this;
	head->mCount++;
}

// 0x800D9BAC
void BatList::unlink()
{
	if (mPrev != 0) {
		mPrev->mNext = mNext;
	}
	if (mNext != 0) {
		mNext->mPrev = mPrev;
	}
	mPrev = 0;
	mNext = 0;
}

// 0x800D9BE4
// Leaves the list taking one off the head's count
void BatList::remove()
{
	BatList* head = this;
	while (head->mPrev != 0) {
		head = head->mPrev;
	}
	head->mCount--;
	unlink();
}

// 0x800D9C28
int BatList::hasAny()
{
	return (u8)(mNext != 0);
}

// 0x800D9C3C
int BatList::isEmpty()
{
	return (u8)__rlwnm(1, __cntlzw(mCount), 31, 31);
}

// 0x800D9C54
// Lets go count enemies on a ring of 5 around center evenly spaced from a random angle
void BatList::spawn(Vec3f* center, int count)
{
	int step;
	TEneAm01* enemy;
	int angle;
	int i;
	f32 z;
	f32 y;
	f32 x;
	angle = (int)(65536.0f * random01());
	step  = 0x10000 / count;
	x     = center->x;
	y     = center->y;
	z     = center->z;
	for (i = 0; i < count; i++) {
		Vec3f pos;
		f32 sin;
		f32 cos;
		pos.x = x;
		pos.y = y;
		pos.z = z;
		njSinCos(angle, &sin, &cos);
		pos.x = 5.0f * sin + pos.x;
		pos.z = 5.0f * cos + pos.z;
		enemy = createTEneAm01WithBrain(gTL_04, 4, new Am01BatBrain(this, &pos, angle));
		if (enemy != 0) {
			enemy->setPos(&pos);
			enemy->updateArea();
		}
		angle += step;
	}
}

// 0x800D9DC0
Am01BatBrain::Am01BatBrain(BatList* head, const Vec3f* center, int angle)
{
	mCenter = *center;
	mAngle  = angle;
	mLink.link(head);
}

// 0x800D9E38
Am01BatBrain::~Am01BatBrain()
{
}

// 0x800D9EA0
// Slot _0C
void Am01BatBrain::onPhase(int state, int phase)
{
	switch (state) {
	case STATE_DROP:
		phaseDrop(phase);
		break;
	case STATE_FLIP:
		phaseFlip(phase);
		break;
	case STATE_DIE:
		phaseDie(phase);
		break;
	case STATE_WANDER:
		phaseLeap(phase);
		break;
	}
}

// 0x800D9F0C
// Falls to the ground 30 out from mCenter staggers then leaps
void Am01BatBrain::phaseDrop(int phase)
{
	Vec3f pos;
	Vec3f ground;
	f32 sin;
	f32 cos;
	switch (phase) {
	case 0:
		njSinCos(mAngle, &sin, &cos);
		pos.x = 30.0f * sin;
		pos.y = 0.0f;
		pos.z = 30.0f * cos;
		njAddVector(&pos, &mCenter);
		if (probeGround(&pos, &ground, 0) != 0) {
			pos.y = ground.y;
		}
		mOwner->mHome        = pos;
		mOwner->mBody.mVel.x = sin;
		mOwner->mBody.mVel.y = 2.0f;
		mOwner->mBody.mVel.z = cos;
		mOwner->mBody.mRot.y = mAngle;
		mOwner->mSpeed       = 1.0f;
		mNext                = TEneAm01::ACT_STAGGER;
		break;
	case 2:
		if (mOwner->isStaggerDone() != 0) {
			changeState(STATE_WANDER);
		}
		break;
	case 1:
		break;
	case 3:
		break;
	}
}

// 0x800DA064
// Slot _24 Starts falling instead of appearing
void Am01BatBrain::attach(TEneAm01* owner)
{
	mOwner = owner;
	mNext  = -1;
	mState = STATE_DROP;
	onPhase(mState, 0);
	if (mNext != -1) {
		mOwner->setAction(mNext);
	}
}

// 0x800DA0CC
// Slot _20
void Am01BatBrain::onDeathStart()
{
	mLink.remove();
}
