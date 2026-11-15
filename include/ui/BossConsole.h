#ifndef _UI_BOSSCONSOLE_H_
#define _UI_BOSSCONSOLE_H_
struct TArc;
struct TAeObjBossConsole;

#include "core/types.h"

extern "C" {
void bossConsoleFree();
void bossConsoleLoad(const char*);
}

extern "C" {
TAeObjBossConsole* bossConsoleGet();
}

#include "Ninja/njtypes.h"
extern "C" {
extern TAeObjBossConsole* sBossConsole;
extern int sBossConsoleOpen;
extern int sBossConsoleClose;
extern TArc* sBossConsoleArc;
extern void* sBossConsoleAe;
extern NJS_TEXLIST* sBossConsoleTexList;
extern const char* gTAeObjBossConsoleName;
extern const u32 sBossConsoleBanks[2];
}

#endif
