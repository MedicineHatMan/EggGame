#ifndef _ENEMY_ENEMYMOTIONKEY_H_
#define _ENEMY_ENEMYMOTIONKEY_H_
struct EnemyMotion;
struct MotionKey;

#include "core/types.h"

struct MotionEntry;

extern "C" {
f32 jumpHeightAt(f32, f32, f32);
f32 jumpRiseTime(f32, f32);
f32 jumpSpeed(f32, f32);
void jumpTimesAt(f32, f32, f32, f32*, f32*);
f32 motionBlendLength(f32);
f32 motionEntryLength(const MotionEntry*);
f32 motionEntryTimeAt(const MotionEntry*, f32);
}

extern "C" {
void motionKeyPlay(const MotionEntry*, MotionKey*, EnemyMotion*, f32, f32);
}

/** One key of the script */
struct MotionKey {
	enum {
		KEY_ONCE  = 0, // plays the entry once holding the end
		KEY_LOOP  = 1, // loops the entry
		KEY_BLEND = 2, // blends from the key before into the entry
		KEY_END   = 3,
	};

	int mKind;  // _00
	f32 mTime;  // _04 when the key starts
	int mEntry; // _08 a row of the motion table
	int _0C;    // _0C
};

#endif
