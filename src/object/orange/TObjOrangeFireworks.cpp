#include "core/types.h"
#include "object/orange/TObjOrangeFireworks.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "egg/EggCreature.h"
#include "engine/TParticle.h"
#include "sound/SoundCtrl.h"
#include "engine/Landscape.h"

const char* gTObjOrangeFireworksName = "TObjOrangeFireworks";

// 0x8012F054
// loads obj_ms_fwball.arc for TObjOrangeFireworksBall

extern "C" {
// 60 frames A one-element table the DOL loads it before the count is clamped as it would a variable
extern const f32 gFireworkFrames[1]   = { 60.0f };
extern const int gFireworkParticles[] = {
	7, 8, 9, 10, 11, 12, 13, 14, 15,
};
}

typedef char TObjOrangeFireworks_size_check[(sizeof(TObjOrangeFireworks) == 0x38) ? 1 : -1];

#define randUnit() (3.0517578e-05f * (f32)rand())

// 0x8012EC58
extern "C" TObjOrangeFireworks* createTObjOrangeFireworks(f32 spread, TObject* parent, Vec3f* pos, u32 count)
{
	return new TObjOrangeFireworks(spread, parent, pos, count);
}

// 0x8012ECC4
TObjOrangeFireworks::TObjOrangeFireworks(f32 spread, TObject* parent, const Vec3f* pos, u32 count)
    : TObject(parent)
    , mPos(*pos)
{
	mSpread    = spread;
	mTimer     = 0;
	f32 frames = gFireworkFrames[0];
	mWait      = (int)(frames / (f32)CLAMP_RANGE((int)count, 1, 0x3C));
	_00        = (u32)gTObjOrangeFireworksName;
	mSize      = 0x38;
	particleOnceById(this, &mPos, 1, gFireworkParticles[0]);
	setLandscapeBlend(0.95f, 0);
	setLandscapeBlend(1.0f, 0x3C);
	sePlay(0x24C);
}

// 0x8012EDDC
TObjOrangeFireworks::~TObjOrangeFireworks()
{
}

// 0x8012EE38
void TObjOrangeFireworks::draw()
{
	if (mTimer > mWait) {
		Vec3f at;
		at.x      = mSpread * (2.0f * (randUnit() - 0.5f)) + mPos.x;
		at.y      = mSpread * (2.0f * (randUnit() - 0.5f)) + mPos.y;
		at.z      = mSpread * (2.0f * (randUnit() - 0.5f)) + mPos.z;
		int which = (int)(9.0f * randUnit());
		if (particleOnceById(this, &at, 1, gFireworkParticles[which]) != 0) {
			setLandscapeBlend(0.050000012f * randUnit() + 0.9f, 0);
			setLandscapeBlend(1.0f, 0x3C);
			sePlay(0x24C);
		}
		mTimer = 0;
	}
	mTimer++;
}

// 0x8012F004
// is the static initializer for these
static inline u32 fireworkColor(u32 rgba)
{
	return rgba;
}

u32 sFireworkRed                 = fireworkColor(0x800000FF);
u32 sFireworkGreen               = fireworkColor(0x000080FF);
u32 sFireworkBlue                = fireworkColor(0x808000FF);
FireworkColor sFireworkColors[9] = {
	0x800000FF, 0x800000FF, 0x800000FF, 0x000080FF, 0x000080FF, 0x000080FF, 0x808000FF, 0x808000FF, 0x808000FF,
};
