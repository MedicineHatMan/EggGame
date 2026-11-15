#include "enemy/TEneAm01Parts.h"
#include "sound/SeHolder.h"
#include "enemy/EnemyCommon.h"
#include "camera/TCamEditor.h"
#include "engine/ParticleBank.h"

struct NJS_TEXLIST;

extern "C" {
}

int sEmotionSheet; // 0x802C5318

// 0x800E8D9C
extern "C" void enemyEffectLoad()
{
	sEmotionSheet = spriteAddTexList(enemyEmotionTextures());
}

// 0x800E8DC4
extern "C" void enemyEffectRelease()
{
	spriteRemoveTexList(sEmotionSheet);
}

// 0x800E8DE8
EnemyEffect::EnemyEffect()
{
	_44     = 0;
	mTarget = 0;
	mHeight = 0.0f;
	mSize   = 10.0f;
	mState  = 0;
	mTimer  = 0;
	spriteInit((Sprite*)&mSprite);
	mSprite.mColor = -1;
	setSpriteMode(&mSprite.mMode, 0, 0, 0);
	mSprite.mSheet = sEmotionSheet;
	mSprite.mCell  = 0;
	mSprite._28    = 10.0f;
}

// 0x800E8E70
EnemyEffect::~EnemyEffect() { }

// 0x800E8EAC
extern "C" void enemyEffectSet(f32 size, f32 height, EnemyEffect* effect, const Vec3f* target)
{
	effect->mTarget = target;
	effect->mSize   = size;
	effect->mHeight = height;
}

// 0x800E8EBC
extern "C" void enemyEffectSetSmall(f32 height, EnemyEffect* effect, const Vec3f* target)
{
	enemyEffectSet(5.0f, height, effect, target);
}

// 0x800E8EE4
extern "C" void enemyEffectSetLarge(f32 height, EnemyEffect* effect, const Vec3f* target)
{
	enemyEffectSet(8.0f, height, effect, target);
}

// 0x800E8F0C
extern "C" void enemyEffectStart(EnemyEffect* effect)
{
	effect->mState = 1;
	effect->mTimer = 0;
	seStartAt(0xDD, effect->mTarget);
}

// 0x800E8F48
extern "C" void enemyEffectUpdate(EnemyEffect* effect)
{
	switch (effect->mState) {
	case 0:
		break;
	case 1: {
		effect->mTimer++;
		f32 scale = 0.1f * effect->mSize * effect->mTimer;
		effect->mSprite.mScaleX = scale;
		effect->mSprite.mScaleY = scale;
		if (effect->mTarget != 0) {
			effect->mSprite.mPos = *effect->mTarget;
			effect->mSprite.mPos.y += effect->mHeight;
			effect->mSprite.mPos.y += effect->mSize;
		}
		spriteAdd((Sprite*)&effect->mSprite);
		if (effect->mTimer >= 10) {
			effect->mState = 2;
			effect->mTimer = 12;
		}
		break;
	}
	case 2:
		effect->mTimer--;
		if (effect->mTimer <= 0) {
			effect->mState = 0;
		}
		if (effect->mTarget != 0) {
			effect->mSprite.mPos = *effect->mTarget;
			effect->mSprite.mPos.y += effect->mHeight;
			effect->mSprite.mPos.y += effect->mSize;
		}
		spriteAdd((Sprite*)&effect->mSprite);
		break;
	}
}
