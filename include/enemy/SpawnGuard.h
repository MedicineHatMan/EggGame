#ifndef _SPAWNGUARD_H
#define _SPAWNGUARD_H

#include "core/types.h"
#include "stage/Stage.h"

struct SpawnGuardList;

/**
 * One circle on the ground, in the global list while it lives. Only x and z are tested.
 *
 * @size 0x20
 */
struct SpawnGuard {
	SpawnGuard();  // 0x800707A0 a radius of 21
	~SpawnGuard(); // 0x80070804

	void setPos(const Vec3f* pos);         // 0x800708AC
	void setRadius(f32 radius);      // 0x800708C8 stored squared and 20 wider
	void set(Vec3f* pos, f32 radius); // 0x800708E4

	SpawnGuard* mNext;     // _00
	SpawnGuard* mPrev;     // _04
	SpawnGuard* mOwner;    // _08 always this
	SpawnGuardList* mList; // _0C
	Vec3f mPos;            // _10
	f32 mRadiusSq;         // _1C
};

/** A list of SpawnGuards, added at the tail. @size 0xC */
struct SpawnGuardList {
	SpawnGuardList();  // 0x80070928
	~SpawnGuardList(); // 0x8007093C

	void add(SpawnGuard* guard);    // 0x80070984
	void remove(SpawnGuard* guard); // 0x800709C4
	void clear();                   // 0x80070A24

	SpawnGuard* mTail; // _00
	SpawnGuard* mHead; // _04
	int mCount;        // _08
};

extern "C" {
void spawnGuardInit();          // 0x80070730 from initSetManager
void spawnGuardFree();          // 0x80070768 from freeSetManager
int spawnGuardTest(Vec3f* pos); // 0x80070854 1 if pos is inside any guard
}

#endif
