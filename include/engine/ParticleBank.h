#ifndef _PARTICLEBANK_H
#define _PARTICLEBANK_H

#include "core/types.h"
#include "io/ResBank.h"

struct NJS_TEXLIST;
struct Vec3f;

/**
 * Which texture a particle draws with, and how. setSpriteMode and getSpriteAnim read and write mMode.
 * @size 0x4
 */
struct ParticleTex {
	u16 mMode;  // _00 Bits 0-1 blend 2-3 animation 1 = play once 2 = start on a random frame 4 a flag
	u8 mBank;   // _02 the slot the texture is in
	u8 mIndex;  // _03 the texture in that slot's list found from ParticleDesc::mTexId
};

/**
 * The parameter record. getParticleDesc finds one by slot and number.
 *
 * @size 0xA0
 */
struct ParticleDesc {
	u16 mKind;        // _00 0 emit 1 spiral 2 vacuum
	u16 mFlags;       // _02 see below
	ParticleTex mTex; // _04
	u32 mTexId;       // _08 the global index of the texture as in the file
	f32 mRadius;      // _0C of the sphere a vacuum particle starts on
	f32 mRadiusY;     // _10
	f32 mRadiusZ;     // _14
	f32 mScaleX;      // _18
	f32 mScaleXRange; // _1C
	f32 mScaleY;      // _20
	f32 mScaleYRange; // _24
	f32 mScaleDamp;   // _28
	f32 mSpeed;       // _2C
	f32 mSpeedRange;  // _30
	f32 mUp;          // _34
	f32 mUpRange;     // _38
	f32 mSpeedDamp;   // _3C
	f32 mGravity;     // _40
	u8 mColour[4];    // _44 at the start
	u8 mColourEnd[4]; // _48
	u16 mSpin;        // _4C
	u16 mRot;         // _4E
	s16 mRateRate;    // _50 how fast the rate swings 0 for steady
	s16 mRate;        // _52 in hundredths of a particle a frame
	s16 mLife;        // _54
	s16 _56;          // _56
	f32 mFadeIn;      // _58
	f32 mFadeOut;     // _5C
	f32 mOrbit;       // _60
	f32 mOrbitRange;  // _64
	f32 mWobbleSpeed; // _68 in degrees
	f32 mWobble;      // _6C
	s32 mTurn;        // _70
	u32 mTurnRange;   // _74
	u8 _78[8];        // _78
	f32 mAnimRate;    // _80
	u8 _84[0x1C];     // _84
};

/**
 * One slot's textures: a ResBank with the .arc and .gvm, and the global index of each texture in its list.
 * @size 0x14
 */
struct ParticleBank {
	ResBank mBank;  // _00
	int mTexCount;  // _0C
	u32* mTexIds;   // _10
};

extern "C" {
void loadParticles(const char* name);                     // 0x8003FC80
void freeParticles();                                     // 0x8003FDE0
ParticleDesc* getParticleDesc(int slot, int id);          // 0x8003FFCC
ParticleDesc* getParticleDescs(int slot);                 // 0x80040020
int getParticleSlotCount();                               // 0x80040044
int getParticleDescCount(int slot);                       // 0x8004004C
void particleRandomOffset(const ParticleDesc* desc, Vec3f* out); // 0x80040094
void setSpriteMode(u16* mode, int blend, int anim, int flag);    // 0x80040180
int getSpriteAnim(const u16* mode);                       // 0x80040194
}

#endif
