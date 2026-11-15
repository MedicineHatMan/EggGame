#include "enemy/SpawnGuard.h"

static SpawnGuardList* sSpawnGuards; // 0x802C4FE0

// 0x80070730
extern "C" void spawnGuardInit()
{
	sSpawnGuards = new SpawnGuardList;
}

// 0x80070768
extern "C" void spawnGuardFree()
{
	if (sSpawnGuards != 0) {
		delete sSpawnGuards;
		sSpawnGuards = 0;
	}
}

// 0x800707A0
SpawnGuard::SpawnGuard()
{
	mNext = 0;
	mPrev = 0;
	mList = 0;
	mPos.x = 0.0f;
	mPos.y = 0.0f;
	mPos.z = 0.0f;
	setRadius(21.0f);
	mOwner = this;
	sSpawnGuards->add(this);
}

// 0x80070804
SpawnGuard::~SpawnGuard()
{
	sSpawnGuards->remove(this);
}

// 0x80070854
// Flat distance only
extern "C" int spawnGuardTest(Vec3f* pos)
{
	f32 x = pos->x;
	f32 z = pos->z;
	SpawnGuard* guard;
	for (SpawnGuard* node = sSpawnGuards->mHead; node != 0; node = node->mNext) {
		guard = node->mOwner;
		f32 dx;
		f32 dz;
		dz = guard->mPos.z - z;
		dx = guard->mPos.x - x;
		if (dx * dx + dz * dz < guard->mRadiusSq) {
			return 1;
		}
	}
	return 0;
}

// 0x800708AC
void SpawnGuard::setPos(const Vec3f* pos)
{
	mPos = *pos;
}

// 0x800708C8
void SpawnGuard::setRadius(f32 radius)
{
	mRadiusSq = 20.0f + radius;
	mRadiusSq = mRadiusSq * mRadiusSq;
}

// 0x800708E4
void SpawnGuard::set(Vec3f* pos, f32 radius)
{
	setPos(pos);
	setRadius(radius);
}

// 0x80070928
SpawnGuardList::SpawnGuardList()
{
	mHead = 0;
	mTail = 0;
	mCount = 0;
}

// 0x8007093C
SpawnGuardList::~SpawnGuardList()
{
	clear();
}

// 0x80070984
void SpawnGuardList::add(SpawnGuard* guard)
{
	if (mHead == 0) {
		mTail = guard;
		mHead = guard;
	} else {
		guard->mPrev = mTail;
		mTail->mNext = guard;
		mTail = guard;
	}
	guard->mList = this;
	mCount++;
}

// 0x800709C4
void SpawnGuardList::remove(SpawnGuard* guard)
{
	if (guard->mPrev != 0) {
		guard->mPrev->mNext = guard->mNext;
	} else {
		mHead = guard->mNext;
	}
	if (guard->mNext != 0) {
		guard->mNext->mPrev = guard->mPrev;
	} else {
		mTail = guard->mPrev;
	}
	guard->mNext = 0;
	guard->mPrev = 0;
	guard->mList = 0;
	mCount--;
}

// 0x80070A24
// Unlinks every guard without freeing it
void SpawnGuardList::clear()
{
	SpawnGuard* node = mHead;
	while (node != 0) {
		SpawnGuard* next = node->mNext;
		node->mList = 0;
		node->mNext = 0;
		node->mPrev = 0;
		node = next;
	}
}
