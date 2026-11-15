#ifndef _STAGE_TGAMESTATE_H_
#define _STAGE_TGAMESTATE_H_
struct MiniGameRules;
struct TCustomGameContext;
struct ObjMsg;

#include "core/types.h"

class TGameContext;

extern "C" {
int cmdMissionFail(void*);
int cmdMissionSuccess(void*);
int countKeyBits();
TGameContext* createGameContext(int);
int getStageBgm();
}

extern "C" {
int sendGameMsg(ObjMsg*);
}

extern "C" {
void setGameContext(TCustomGameContext*);
int cmdSetBgm(u16*);
}

extern "C" {
int cmdMiniGame(MiniGameRules*);
}

extern "C" {
extern const char* sGameStateName;
extern const char* sGameContextName;
}

#endif
