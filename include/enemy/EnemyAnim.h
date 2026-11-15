#ifndef _ENEMYANIM_H
#define _ENEMYANIM_H

#include "core/types.h"
struct NJS_MOTION;
#include "engine/MotionPlayer.h"

struct EnemyMotion;

/**
 * Holds which motion the enemy plays. A play is queued in mNext and started on the next motionCtrlUpdate.
 *
 * @size 0xC
 */
struct EnemyAnim {
	EnemyAnim(); // 0x80072A18

	MotionPlayer* mPlayer; // _00
	int mNext;             // _04 the queued entry -1 = none
	f32 mBlend;            // _08 the queued blend rate
};

/**
 * One step of a motion timeline, built by motionStepFromEntry and its kin and played by motionStepsPlay. The list
 * ends with a STEP_END.
 *
 * @size 0x14
 */
struct MotionStep {
	enum {
		STEP_ONCE  = 0, // plays the entry once holding the end
		STEP_LOOP  = 1, // loops the entry
		STEP_BLEND = 2, // blends from this step's frame into the next step's
		STEP_END   = 3,
	};

	s16 mType;  // _00
	s16 mEntry; // _02 a row of the motion table
	f32 mTime;  // _04 when the step starts
	f32 mRate;  // _08 frames per tick or the blend rate
	f32 mStart; // _0C the first frame
	f32 mEnd;   // _10 the last frame
};

extern "C" {
void motionCtrlSetPlayer(void* anim, void* player);          // 0x80072A34
void* motionCtrlGetPlayer(void* anim);                       // 0x80072A3C
void motionCtrlInit(void* anim, void* table, void* motions); // 0x80072A44
void motionCtrlUpdate(void* anim);                           // 0x80072A68
void motionCtrlPlay(void* anim, int entry);                  // 0x80072AE4
void motionCtrlPlayBlend(f32 rate, void* anim, int entry);   // 0x80072B08
int motionCtrlIsDone(const void* anim);                      // 0x80072B18
int motionCtrlCurrent(void* anim);                           // 0x80072B3C the entry playing
void motionCtrlSetSpeed(void* anim, f32 speed);              // 0x80072B48
int motionCtrlIsBlending(const void* anim);                  // 0x80072B54
NJS_MOTION* motionCtrlGetMotion(const void* anim);           // 0x80072B78
f32 motionCtrlFrame(void* anim);                             // 0x80072BA0
void motionCtrlSetFrame(f32 frame, void* anim);              // 0x80072BAC
void* motionCtrlGetBlend(const void* anim);                  // 0x80072BB8
f32 motionCtrlBlendTime(void* anim);                         // 0x80072BDC
int motionCtrlPassed(f32 frame, void* anim);                 // 0x80072BE8

void enemyMotionSet(EnemyMotion* motion, int entry, f32 frame);                                      // 0x80072C78
void enemyMotionBlend(EnemyMotion* motion, int fromEntry, f32 from, int toEntry, f32 to, f32 blend); // 0x80072C98
void enemyMotionBlendEnds(EnemyMotion* motion, int fromEntry, int toEntry, f32 blend);               // 0x80072D2C

void motionStepFromEntry(f32 time, MotionStep* step, int entry, const MotionEntry* table); // 0x80072DA8
void motionStepOnce(f32 time, f32 start, f32 end, f32 rate, MotionStep* step, int entry);  // 0x80072E00
void motionStepLoop(f32 time, f32 start, f32 end, f32 rate, MotionStep* step, int entry);  // 0x80072E20
void motionStepBlend(f32 time, f32 rate, MotionStep* step, EnemyMotion* from);             // 0x80072E40
void motionStepBlendOut(f32 time, f32 rate, MotionStep* step, const MotionStep* from);     // 0x80072E6C
void motionStepEnd(MotionStep* step);                                                      // 0x80072F2C
f32 motionStepOnceFrame(f32 time, f32 start, f32 end, f32 rate);                           // 0x80072F54
f32 motionStepLoopFrame(f32 time, f32 start, f32 end, f32 rate);                           // 0x80072F7C
f32 motionStepBlendRate(f32 time, f32 rate);                                               // 0x80072FE8
int motionStepsPlay(f32 time, const MotionStep* steps, EnemyMotion* motion);               // 0x80073014
}

#endif
