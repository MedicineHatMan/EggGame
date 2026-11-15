#include "egg/TMorphEggAnim.h"
#include "engine/TParticle.h"
#include "Ninja/ninja.h"

typedef char MorphKey_size_check[(sizeof(MorphKey) == 0x1C) ? 1 : -1];
typedef char MorphScript_size_check[(sizeof(MorphScript) == 0x58) ? 1 : -1];
typedef char TMorphEggAnim_size_check[(sizeof(TMorphEggAnim) == 0xCC) ? 1 : -1];

extern "C" {

extern const u8 gMorphEggScript[] = {
	0x01, 0x06, 0x00, 0x0A, 0x3F, 0x80, 0x00, 0x00, 0xBC, 0x7F, 0x00, 0x00,
};
}

const char* gTMorphEggAnimName = "TMorphEggAnim";

// 0x800565C4
TMorphEggAnim::TMorphEggAnim(TObject* parent, const Vec3f* pos, f32 scale)
    : TObject(parent)
    , mScale(scale)
    , mPos(*pos)
{
	_00           = (u32)gTMorphEggAnimName;
	mSize         = 0xCC;
	mState.mFlags = 0x20;
	mState.mTev2  = 0;
	mScript.setSlot(0, &mKey);
	mScript.setScript((u8*)gMorphEggScript);
	mPart = particleGenEmptyDefault(pos, 6, 0x1D);
	mPart->setScale(scale);
}

// 0x800566A4
TMorphEggAnim::~TMorphEggAnim()
{
	mPart->mFlags |= 1;
}

// 0x80056710
void TMorphEggAnim::draw()
{
	mScript.update();
	u8 c         = (u8)(255.0f * mKey.mValue);
	mState.mTev2 = (c << 24) | (c << 16) | (c << 8) | c;
}
