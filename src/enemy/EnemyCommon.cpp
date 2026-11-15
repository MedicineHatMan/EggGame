#include "enemy/EnemyCommon.h"
#include "enemy/TEneAm01Parts.h"
#include "Ninja/ninja.h"
#include "enemy/ObjEnemy.h"
#include "io/ResBank.h"
#include "egg/EggCreature.h"
#include "ui/AeArc.h"

// 0x80090C78
void enemyCommonLoad()
{
	resBankLoad(&gEnemyCommonBank, "ar_ene_common.arc");
	resBankAddTextures(&gEnemyCommonBank, 0, "ene_common.gvm");
	resBankAddTextures(&gEnemyCommonBank, 2, "ene_eye.gvm");
	resBankAddTextures(&gEnemyCommonBank, 1, "ene_emotion.gvm");
	gEnemyCommonModels   = resBankGetModels(&gEnemyCommonBank);
	gEnemyCommonMotion   = resBankGetMotion(&gEnemyCommonBank, 0);
	gEnemyCommonTextures = resBankGetTextures(&gEnemyCommonBank);
	fxBankLoadDefault(&gEnemyCommonFx, "pa_enemy.pad");
	enemyEffectLoad();
}

// 0x80090D34
void enemyCommonRelease()
{
	enemyEffectRelease();
	fxBankRelease(&gEnemyCommonFx);
	resBankRemoveTextures(&gEnemyCommonBank, 1);
	resBankRemoveTextures(&gEnemyCommonBank, 2);
	resBankRemoveTextures(&gEnemyCommonBank, 0);
	resBankRelease(&gEnemyCommonBank);
	gEnemyCommonModels   = NULL;
	gEnemyCommonMotion   = NULL;
	gEnemyCommonTextures = NULL;
}

// 0x80090DA8
NJS_TEXLIST* enemyEyeTextures()
{
	return gEnemyCommonTextures[2];
}

// 0x80090DB4
NJS_TEXLIST* enemyCommonTextures()
{
	return gEnemyCommonTextures[0];
}

// 0x80090DC0
void enemyCommonSetTexture()
{
	njSetTexture(enemyCommonTextures());
}

// 0x80090DE4
void enemyCommonSetEye(int frame)
{
	NJS_TEXLIST* list = enemyCommonTextures();
	njSetTexture(list);
	enemyEyeSet(list, 0, frame);
}

// 0x80090E28
void enemyCommonEndEye()
{
	enemyEyeRestore();
}

// 0x80090E48
NJS_TEXLIST* enemyEmotionTextures()
{
	return gEnemyCommonTextures[1];
}

// 0x80090E54
NJS_OBJECT* enemyBombModel()
{
	return gEnemyCommonModels[1];
}

// 0x80090E60
NJS_OBJECT* enemyShadowModel()
{
	return gEnemyCommonModels[0];
}

// 0x80090E6C
NJS_OBJECT* enemyCrowModel()
{
	return gEnemyCommonModels[2];
}

// 0x80090E78
void* enemyCommonMotion()
{
	return gEnemyCommonMotion;
}

// 0x80090E80
// The table ends with a row below 0
int enemyFrameKey(const EnemyFrameKey* table, f32 frame)
{
	for (; table->mTime >= 0.0f; table++) {
		if (frame < table->mTime) {
			break;
		}
	}
	return table->mValue;
}

// 0x80090EAC
GjTexMem* enemyEyeSwap(NJS_TEXLIST* list, int slot, int frame)
{
	NJS_TEXLIST* eyes = enemyEyeTextures();
	NJS_TEXNAME* name = &list->textures[slot];
	GjTexMem* old     = name->texaddr;
	name->texaddr     = eyes->textures[frame].texaddr;
	return old;
}

// 0x80090F04
void enemyEyeSet(NJS_TEXLIST* list, int slot, int frame)
{
	if (gEnemyEyeList == NULL) {
		gEnemyEyeList  = list;
		gEnemyEyeSlot  = slot;
		gEnemyEyeSaved = enemyEyeSwap(list, slot, frame);
	}
}

// 0x80090F3C
void enemyEyeRestore()
{
	gEnemyEyeList->textures[gEnemyEyeSlot].texaddr = gEnemyEyeSaved;
	gEnemyEyeList                                  = NULL;
}
