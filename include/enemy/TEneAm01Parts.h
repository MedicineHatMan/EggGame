#ifndef _TENEAM01PARTS_H
#define _TENEAM01PARTS_H

#include "core/types.h"
#include "stage/Stage.h"

struct Vec3f;
struct SetRot;
class TObject;
class TParticleGenerator;

/**
 * Colour flashes on hit: three pulses of the light's added colour up to 0xA0 and back. Handed to drawLit, which adds
 * it to the lights. The name is ours.
 *
 * @size 0x8
 */
struct EnemyTint {
	EnemyTint();             // 0x8001CAB8
	void reset();            // 0x8001CAE8
	void start();            // 0x8001CB08
	void clear();            // 0x8001CB20
	int update();            // 0x8001CB34 1 once it's done or idle
	int isActive();          // 0x8001CBD4
	void getPhase(u32* out); // 0x8001CBE8
	void addToLight(u32* state); // 0x8001CBF8 state is the 9-word draw state

	u32 mLevel;  // _00
	s16 mPhase;  // _04 even going up odd going down
	s16 mActive; // _06
};

/**
 * A camera-facing sprite drawn by drawSprite (0x80015DA8). Its texture and frame come from a sheet. The same layout
 * as BeamSprite in TEneBoss03_Beam_Pera.cpp.
 *
 * @size 0x38
 */
struct EffectSprite {
	u32 _00[4];     // _00
	Vec3f mPos;     // _10
	f32 mScaleX;    // _1C
	f32 mScaleY;    // _20
	s32 mColor;     // _24
	f32 _28;        // _28
	u16 mMode;      // _2C see setSpriteMode
	u8 mSheet;      // _2E the sheet the texture is in
	u8 mCell;       // _2F
	u16 _30;        // _30
	s16 mFrame;     // _32
	u16 _34;        // _34
	u16 _36;        // _36
};

/**
 * The emotion mark that pops up over an enemy that has spotted the player: it grows for ten frames, then holds for
 * twelve, following a point. In EnemyEffect.cpp (0x800E8D9C-0x800E90AC). The names are ours.
 *
 * @size 0x50
 */
struct EnemyEffect {
	EnemyEffect();  // 0x800E8DE8
	~EnemyEffect(); // 0x800E8E70

	EffectSprite mSprite;  // _00
	const Vec3f* mTarget;  // _38 the point it follows
	f32 mHeight;           // _3C how far above the point
	f32 mSize;             // _40
	u32 _44;               // _44
	int mState;            // _48 0 off 1 growing 2 holding
	int mTimer;            // _4C
};

extern "C" {
void enemyEffectLoad();                                                        // 0x800E8D9C
void enemyEffectRelease();                                                     // 0x800E8DC4
void enemyEffectSet(f32 size, f32 height, EnemyEffect* effect, const Vec3f* target); // 0x800E8EAC
void enemyEffectSetSmall(f32 height, EnemyEffect* effect, const Vec3f* target); // 0x800E8EBC size 5
void enemyEffectSetLarge(f32 height, EnemyEffect* effect, const Vec3f* target); // 0x800E8EE4 size 8
void enemyEffectStart(EnemyEffect* effect);                                    // 0x800E8F0C
void enemyEffectUpdate(EnemyEffect* effect);                                   // 0x800E8F48
}

/**
 * How an enemy gets squashed when something lands on it: flattened along the direction the hit came from, then
 * popped back up into the air. The name is ours.
 *
 * @size 0x10
 */
struct EnemyCrush {
	void clear();                                         // 0x8001C47C
	void start(Vec3f* pos, const Vec3f* hit, const Vec3f* ground);     // 0x8001C498
	int step(Vec3f* pos, Vec3f* vel, SetRot* rot);         // 0x8001C520 1 once it's done
	void burst(f32 height, Vec3f* pos, SetRot* rot, Vec3f* out); // 0x8001C678
	void rotate(const SetRot* rot);                             // 0x8001C3E8
	void apply(SetRot* rot);                              // 0x8001C430

	f32 mTime;  // _00 the height scale
	int mAngle; // _04 which way the hit came from
	int mState; // _08
	int mTimer; // _0C
};

/**
 * The arc a dying enemy flies off in, with its smoke and sparks. The name is ours.
 *
 * @size 0xC
 */
struct EnemyBreak {
	EnemyBreak(Vec3f dir, Vec3f* pos, Vec3f* vel);        // 0x8001C6F8
	~EnemyBreak();                                        // 0x8001C744
	void init(Vec3f dir, Vec3f* pos, Vec3f* vel);         // 0x8001C7C4
	int update(Vec3f* pos, Vec3f* vel);                   // 0x8001C888 1 once it's done

	TParticleGenerator* mSmoke;  // _00
	TParticleGenerator* mSparks; // _04
	int mTimer;       // _08
};

#endif
