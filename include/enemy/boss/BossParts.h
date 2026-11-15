#ifndef _BOSSPARTS_H
#define _BOSSPARTS_H

#include "core/types.h"
#include "engine/MotionPlayer.h"
#include "stage/Stage.h"

struct EnemyMotion { // size 0x3C
	EnemyMotion();
	~EnemyMotion(); // 0x80072C3C

	MotionPlayer mPlayer; // _00
};

/** What a boss shows as it dies in TEneDeathCrow.cpp 0x800E89C8-0x800E8D9C its colour fades out over one span of
 * time and over another it sinks from one height to another keeping two particles at the sinking line and sending
 * crows up off its parts */
struct BossSuit { // size 0x38
	BossSuit();   // 0x800E89C8
	~BossSuit();  // 0x800E8A20

	u32 mAlpha;      // _00 255 until the fade starts then down to 0
	f32 mFadeStart;  // _04
	f32 mFadeRate;   // _08 1 over the fade's length
	f32 mHeight;     // _0C where the sink has got to
	f32 mSinkStart;  // _10
	f32 mSinkRate;   // _14 1 over the sink's length
	f32 mSinkFrom;   // _18
	f32 mSinkTo;     // _1C
	f32 _20;         // _20 set by bossSuitSetSink and not read here
	TObject* mFx[2]; // _24 the particles at the sinking line
	int mFxKind;     // _2C starts at 1
	int mFxId[2];    // _30
};

extern "C" {
void bossSuitSetFade(f32 start, f32 end, BossSuit* suit);                              // 0x800E8A68
int bossSuitFade(f32 time, BossSuit* suit);                                            // 0x800E8A80
void bossSuitSetSink(f32 start, f32 end, f32 from, f32 to, f32 extra, BossSuit* suit); // 0x800E8B04
int bossSuitSink(f32 time, BossSuit* suit, const Vec3f* pos, void* model);             // 0x800E8B2C
void bossSuitClearFx(BossSuit* suit);                                                  // 0x800E8BFC
void bossSuitSetFx(BossSuit* suit, int kind, int idA, int idB);                        // 0x800E8C38
void bossSuitKeepFx(BossSuit* suit, const Vec3f* pos);                                 // 0x800E8C48
int bossSuitUpdate(f32 time, BossSuit* suit, const Vec3f* pos, void* model);           // 0x800E8CBC
int bossSuitUpdateNoFx(f32 time, BossSuit* suit);                                      // 0x800E8D30
}

/** A value the id it came from and how long it has been there */
struct DarkFlashSlot { // size 0x8
	void reset();
	void age();
	int check(f32 value, u16 id, int* same);

	f32 mValue; // _00
	u16 mId;    // _04
	u16 mAge;   // _06
};

#endif
