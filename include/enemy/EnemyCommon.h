#ifndef _ENEMYCOMMON_H
#define _ENEMYCOMMON_H

#include "core/types.h"

struct NJS_OBJECT;
struct NJS_TEXLIST;
struct GjTexMem;

// What every enemy shares from ar_ene_common.arc three models a motion the common emotion and eye textures and the
// pa_enemy.pad effects

struct EnemyFrameKey;

extern "C" {
void enemyCommonLoad();                                         // 0x80090C78
void enemyCommonRelease();                                      // 0x80090D34
NJS_TEXLIST* enemyEyeTextures();                                // 0x80090DA8 ene_eye.gvm
NJS_TEXLIST* enemyCommonTextures();                             // 0x80090DB4 ene_common.gvm
void enemyCommonSetTexture();                                   // 0x80090DC0
void enemyCommonSetEye(int frame);                              // 0x80090DE4 the common textures with that eye
void enemyCommonEndEye();                                       // 0x80090E28
NJS_TEXLIST* enemyEmotionTextures();                            // 0x80090E48 ene_emotion.gvm
NJS_OBJECT* enemyBombModel();                                   // 0x80090E54 model 1 TEneAntsBomb's bomb
NJS_OBJECT* enemyShadowModel();                                 // 0x80090E60 model 0 ObjShadow's
NJS_OBJECT* enemyCrowModel();                                   // 0x80090E6C model 2 TEneDeathCrow's
void* enemyCommonMotion();                                      // 0x80090E78 TEneDeathCrow's
int enemyFrameKey(const EnemyFrameKey* table, f32 frame);       // 0x80090E80 the value of the first row past frame
GjTexMem* enemyEyeSwap(NJS_TEXLIST* list, int slot, int frame); // 0x80090EAC gives back what was there
void enemyEyeSet(NJS_TEXLIST* list, int slot, int frame);       // 0x80090F04 one at a time
void enemyEyeRestore();                                         // 0x80090F3C
}

#include "Ninja/ninja.h"
#include "Ninja/njtypes.h"
#include "io/ResBank.h"
extern "C" {
extern ResBank gEnemyCommonBank;
extern NJS_OBJECT** gEnemyCommonModels;
extern NJS_MOTION** gEnemyCommonMotion;
extern NJS_TEXLIST** gEnemyCommonTextures;
extern NJS_TEXLIST* gEnemyEyeList;
extern int gEnemyEyeSlot;
extern GjTexMem* gEnemyEyeSaved;
extern FxBankFile gEnemyCommonFx;
}

#endif
