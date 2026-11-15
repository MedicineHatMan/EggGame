#ifndef _ENEMYGROUPWAVE_H
#define _ENEMYGROUPWAVE_H

#include "core/types.h"

extern "C" {
void enemyGroupStepsEnd();                      // 0x80057BB8
int enemyGroupStartStep(s16 group, s16 step);   // 0x80057BE0 0 already out 1 let out 2 nothing to let out
int enemyGroupTakeStep(s16 group, s16 step);    // 0x80057C7C the same taking the records
int enemyGroupStepActive(s16 group, s16 step);  // 0x80057D18
void enemyGroupWavesEnd();                      // 0x80057F50
int enemyGroupStartWave(s16 group, int keyBit, int restart); // 0x80057F78
int enemyGroupStartFirst(s16 group, s16* step); // 0x80058000
int enemyGroupWaveActive(s16 group);            // 0x8005809C
}

#endif
