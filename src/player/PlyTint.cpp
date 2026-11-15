#include "engine/Color.h"
#include "Ninja/ninja.h"

#include "player/PlyTint.h"

u32 sTintPulse = ColorValue(0xFFFFFFFF).mValue; // 0x802C4F50
u32 sTintFadeA = ColorValue(0x00FF00FF).mValue; // 0x802C4F54
u32 sTintFadeB = ColorValue(0xFF0080FF).mValue; // 0x802C4F58

// 0x80066BC0
extern "C" void plyTintStop(PlyTint* tint)
{
	tint->mTimer = 0;
}

// 0x80066BCC
extern "C" void plyTintPulse(PlyTint* tint)
{
	tint->mMode  = 0;
	tint->mTimer = 20;
}

// 0x80066BE0
extern "C" void plyTintFadeA(PlyTint* tint)
{
	tint->mMode  = 1;
	tint->mTimer = 10;
	tint->mFade  = 1.0f;
}

// 0x80066BFC
extern "C" void plyTintFadeB(PlyTint* tint)
{
	tint->mMode  = 2;
	tint->mTimer = 10;
	tint->mFade  = 1.0f;
}

// 0x80066C18
extern "C" int plyTintIsDone(PlyTint* tint)
{
	return tint->mTimer == 0;
}

// 0x80066C28
extern "C" void plyTintUpdate(PlyTint* tint)
{
	if (tint->mTimer == 0) {
		return;
	}
	switch (tint->mMode) {
	case 0:
		lerpColor(0.3f * njCos(tint->mTimer << 13) + 0.7f, &tint->mColor, &sTintPulse, &gColorBlack);
		break;
	case 1:
		tint->mFade += -tint->mFade / tint->mTimer;
		lerpColor(tint->mFade, &tint->mColor, &sTintFadeA, &gColorBlack);
		break;
	case 2:
		tint->mFade += -tint->mFade / tint->mTimer;
		lerpColor(tint->mFade, &tint->mColor, &sTintFadeB, &gColorBlack);
		break;
	}
	tint->mTimer--;
}
