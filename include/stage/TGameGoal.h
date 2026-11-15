#ifndef _STAGE_TGAMEGOAL_H_
#define _STAGE_TGAMEGOAL_H_

#include "core/types.h"

class TObject;
struct Vec3f;

extern "C" {
void* createGameGoal(TObject*);
void goalScalePos(Vec3f*, const Vec3f*, f32);
}

extern "C" {
extern const char* gTGameGoalName;
}

#endif
