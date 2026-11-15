#ifndef _OBJECT_TOBJPLAYER_H_
#define _OBJECT_TOBJPLAYER_H_
struct BinFile;
struct TArc;
struct TObjPlayer;
struct RestartList;
struct RestartPoint;
struct TObject;

#include "core/types.h"

struct LightBase;
struct Vec3f;

extern "C" {
void restartPointCtor();
void playerLoadShared();
void playerFreeShared();
void playerLoadArc(int, const char*);
void playerEnsureBattleArc(int);
void playerSetDrawView(int);
int playerGetGoal(Vec3f*);
void playerAddLight(int, LightBase*);
void playerRemoveLight(int, LightBase*);
void restartPointSet(int, const Vec3f*, int);
void restartPointUse(int, int);
}

extern "C" {
void* playerGetParamsTail();
void playerLoadSoundBank(int, int);
void clearFlagWord(u32*);
void restartListClear(RestartList*);
void restartListCopy(RestartList*, RestartPoint*, const RestartPoint*, int);
void restartListGrow(RestartList*, int);
void restartListReserve(RestartList*, int);
void restartListAdd(RestartList*, const RestartPoint*);
void playerEnsureArc(int);
void playerFreeArc(int);
void* playerCreate(TObject*, int, u8, int);
int evLoadPlayer(u8*);
}

extern "C" {
// OrangeCannon.cpp sets DRAW_VIEW_RETURNS_U16 It passes the result straight into ObjRef::setId and only a u16 return
// leaves out the caller's mask
#ifdef DRAW_VIEW_RETURNS_U16
u16 playerGetDrawView();
#else
int playerGetDrawView();
#endif
TObjPlayer* findPlayer(int);
void playerSetStart(int, const Vec3f*, int);
void* playerCreateBattle(TObject*, int, u8);
}

extern "C" {
struct MobileParams;
MobileParams* playerGetParams(int);
int evTakeoffSuit();
}

#include "object/ObjCommon.h"
extern "C" {
extern u8 gBattleGroups[4];
extern FlagWord gPlayerFlags;
extern TArc* gPlayerArc;
extern u16 sDrawViewPlayerId;
extern int gPlayerGoalVisible;
extern BinFile* sPlayerParamFile;
extern void* sPlayerParamsTail;
extern void** sPlayerTexLists2;
extern const char* sPlayerArchives[5];
extern const char* sPlayerArchives2[5];
extern const char* sPlayerSoundBanks[5];
extern TArc* gPlayerArcs[5];
}

#endif
