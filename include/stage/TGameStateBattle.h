#ifndef _STAGE_TGAMESTATEBATTLE_H_
#define _STAGE_TGAMESTATEBATTLE_H_

#include "core/types.h"

class TGameContextBattle;
struct ObjMsg;

extern "C" {
void battleAddPlayer(TGameContextBattle*, u16, u8);
int battleDefaultMessage(TGameContextBattle*, ObjMsg*);
int battleFindPlayer(TGameContextBattle*, u16);
int battlePlayerOut(TGameContextBattle*, u16, u16, int);
void* changeBattleState(void*, TGameContextBattle*, void*);
void* createGameStateBattleEnd(void*, TGameContextBattle*);
void* createGameStateBattleStart(void*, TGameContextBattle*);
}

extern "C" {
extern const char* sGameContextBattleName;
extern const char* gTGameStateBattleName;
extern const char* gTGameStateBattleStartName;
extern const char* gTGameStateBattleEndName;
}

#endif
