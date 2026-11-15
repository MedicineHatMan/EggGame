#ifndef _SEHOLDER_H
#define _SEHOLDER_H

#include "core/types.h"
#include "sound/SoundHandle.h"

/**
 * @size 0x34
 */
struct SeHolder {
	SeHolder();               // 0x801239CC
	~SeHolder();              // 0x801239EC
	void setRange(f32 range); // 0x80123A28

	SoundHandle mSound; // _00
	int mPlaying;       // _2C 1 once started
	f32 mRange;         // _30 how far it carries 400 to start with
};

/** A sound set to play once a count reaches a frame */
struct SeCue { // size 0x4
	u16 mFrame; // _00
	u16 mId;    // _02 the sound
};

extern "C" {
void seCueClear(SeCue* cue);                  // 0x80123968
void seCueSet(SeCue* cue, int frame, int id); // 0x80123978
void seCueUpdate(SeCue* cue, int frame);      // 0x80123984

void seHolderStop(SeHolder* holder);                            // 0x80123A30
void seHolderPlay(SeHolder* holder, int id, const Vec3f* pos);  // 0x80123A70

int seStart(int id);                                           // 0x80123ABC
void seStartArg(int id, int arg);                               // 0x80123ADC
int seStartAt(int id, const Vec3f* pos);                       // 0x80123AFC
int seStartWith(int id, int arg);                               // 0x80123B1C
int seStartAtWith(int id, const Vec3f* pos, int arg);          // 0x80123B58
void seStartAtFar(int id, const void* pos);                     // 0x80123B94 carries 700
void seStartAtRange(int id, const void* pos, int volume, f32 range); // 0x80123BBC

void seEnemyDie0(const Vec3f* pos);  // 0x80123BDC sounds 0xE3 and 0xE5
void seEnemyDie1(const Vec3f* pos);  // 0x80123C1C sounds 0xE1 and 0xE5
void seEnemyDie2(const Vec3f* pos);  // 0x80123C5C sounds 0xE2 and 0xE5
void seEnemyCrush(const Vec3f* pos); // 0x80123C9C sound 0xDF
void seEnemyPuff(const Vec3f* pos);  // 0x80123CC4 sound 0x10D

void seLoopStart(SoundHandle* sound, int id, const Vec3f* pos); // 0x80123CEC
void seLoopMove(SoundHandle* sound, const Vec3f* pos);          // 0x80123D18
void seLoopStop(SoundHandle* sound);                            // 0x80123D34
}

#endif
