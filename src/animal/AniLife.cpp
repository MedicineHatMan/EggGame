#include "animal/AniMount.h"
#include "engine/Color.h"
#include "Ninja/ninja.h"
#include "player/Player.h"
#include "engine/GroundQuery.h"
#include "animal/TAniCamelOstrich.h"

u32 gBlinkRed = ColorValue(0xFF0000FF).mValue;

// 0x801CD190
AniLife::AniLife()
    : mHp(100)
    , mMax(100)
{
}

// 0x801CD1D0
AniLife::~AniLife()
{
}

// 0x801CD20C
void AniLife::damage(int damage)
{
	int hp;

	if (flagTest(&mFlags, 0) == 0) {
		hp = (mHp < damage) ? 0 : mHp - damage;
		if (hp != mHp) {
			flagWordSetBit(&mFlags, 0);
			mHp = hp;
		}
	}
}

// 0x801CD280
AniBlink::AniBlink()
    : mResult(0)
    , mBlink(0)
    , mFlash(0)
{
	mLife = NULL;
}

// 0x801CD2D8
void AniBlink::update()
{
	if (mBlink == 0) {
		if (flagTest(&mLife->mFlags, 0) != 0) {
			mBlink = 90;
			mFlash = 20;
			flagWordSetBit(&mFlags, 0);
		}
	} else if (--mBlink == 0) {
		flagClear(&mLife->mFlags, 0);
	}

	switch ((u8)mResult) {
	case 0:
		if (flagTest(&mBody->mFlags, 2) != 0) {
			flagWordSetBit(&mFlags, 1);
			mResult = 2;
		} else if (mLife->mHp == 0) {
			flagWordSetBit(&mFlags, 1);
			mResult = 1;
		}
		break;
	}

	if (mFlash != 0) {
		mFlash--;
	}
}

// 0x801CD3DC
int AniBlink::tint(DrawState* state) const
{
	if (mBlink != 0) {
		lerpColor(0.3f * njCos(mFlash << 13) + 0.7f, &state->mFlags, &gBlinkRed, &gColorBlack);
		return 1;
	}
	return 0;
}

// 0x801CD444
int AniBlink::isHidden()
{
	bool hidden = false;
	s16 blink   = mBlink;
	if (blink != 0 && (blink & 1) == 0 && mFlash == 0) {
		hidden = true;
	}
	return hidden;
}

// 0x801CD474
int AniBlink::getResult()
{
	switch (mResult) {
	case 0:
		return 0;
	case 1:
		return 1;
	case 2:
	default:
		return 2;
	}
}
