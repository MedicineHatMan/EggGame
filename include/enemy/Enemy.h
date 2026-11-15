#ifndef _ENEMY_ENEMY_H_
#define _ENEMY_ENEMY_H_

#include "core/types.h"
struct NJS_MOTION;
struct NJS_TEXLIST;
struct NJS_OBJECT;

class SetDesc;
class TObject;
struct SetClass;
struct SetClassList;
struct SetInitList;

extern "C" {
TObject* createTEneAm01(TObject*, SetDesc*);
const SetClass* findEnemyClass(int, int*);
const SetClass* findEnemyClassIn(const SetClassList*, int, int*);
void freeEnemyInits(const SetInitList**);
void freeEnemyThemes();
void initZakoMotions();
void loadEnemyTheme(const char*);
void loadZako(int);
void loadZakoAnt();
void loadZakoBat();
void loadZakoCat();
void loadZakoFugu();
void loadZakoSnake();
const SetClass* lookupEnemyClass(int);
void shutdownZakoMotions();
int zakoSlot(int);
}

#include "io/ResBank.h"
#include "stage/Stage.h"
extern "C" {
extern Vec3f gVecUp;
extern const SetInitList* sEnemyThemeInits;
extern const SetClassList* sEnemyThemeClasses;
extern const SetInitList* sEnemyCommonInits;
extern const SetClassList* sEnemyCommonClasses;
extern ResBank* sEnemyArcSlots;
extern NJS_OBJECT** gEnemyModelSlots;
extern NJS_TEXLIST** gEnemyTexSlots;
extern NJS_MOTION** gEnemyMotion;
extern ResBank sEnemyBank;
}

#endif
