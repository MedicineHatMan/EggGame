#ifndef _STAGE_TGAMESTATECLEAR_H_
#define _STAGE_TGAMESTATECLEAR_H_

#include "core/types.h"

class TGameContext;
struct ObjMsg;

extern "C" {
void* createGameStateClear(void*, TGameContext*);
void* createGameStateDead(void*, TGameContext*);
void* createGameStateFailed(void*, TGameContext*);
void* createGameStateGameOver(void*, TGameContext*);
void* createGameStateMiss(void*, TGameContext*);
void* createGameStateSucceed(void*, TGameContext*);
void* createGameStateWaitClear(void*, TGameContext*);
void* createGameStateWarp(void*, TGameContext*);
int gameStateToDead(TGameContext*, ObjMsg*);
}

extern "C" {
extern const char* gTGameStateClearName;
extern const char* gTGameStateWarpName;
extern const char* gTGameStateDeadName;
extern const char* gTGameStateFailedName;
extern const char* gTGameStateGameOverName;
extern const char* gTGameStateMissName;
extern const char* gTGameStateSucceedName;
extern const char* gTGameStateWaitClearName;
}

#endif
