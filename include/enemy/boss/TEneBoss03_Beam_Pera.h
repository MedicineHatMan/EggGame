#ifndef _ENEMY_BOSS_TENEBOSS03_BEAM_PERA_H_
#define _ENEMY_BOSS_TENEBOSS03_BEAM_PERA_H_
struct BeamArena;
#include "Ninja/ninja.h"
#include "egg/ObjEgg.h"
#include "engine/CharaColli.h"
#include "object/ObjAction.h"
#include "stage/Stage.h"
#include "camera/TCamEditor.h"
struct BeamSprite;
struct TEneBoss03_Beam_Pera;

#include "core/types.h"

class TObject;
struct Vec3f;

extern "C" {
TObject* createTEneBoss03_Beam_Pera(TObject*, const Vec3f*);
}

/** One sprite. @size 0x38 */
struct BeamSprite {
	BeamSprite() { }
	Vec3f* pos() { return (Vec3f*)&mX; }

	u32 _00;        // _00
	Vec3f mView;    // _04 as in Sprite
	f32 mX;         // _10 the position separate floats so a sprite copy moves them with lfs/stfs
	f32 mY;         // _14
	f32 mZ;         // _18
	f32 mScaleX;    // _1C
	f32 mScaleY;    // _20
	s32 _24;        // _24
	f32 _28;        // _28
	SpriteKey mKey; // _2C see setSpriteMode
	s16 _30;        // _30
	s16 mFrame;     // _32
	s16 _34;        // _34
	s16 _36;        // _36
};

/**
 * @size 0xA8
 */
class TEneBoss03_Beam_Pera : public TObjAction {
public:
	TEneBoss03_Beam_Pera(TObject* parent, const Vec3f* pos);
	virtual ~TEneBoss03_Beam_Pera();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual int getKind();
	virtual int onHit(const TObjAction* other, int arg, int* result);

	f32 getScale();
	void placeSprites();
	void moveOn(Vec3f* pos);
	void setupCollider(void* colli, int mask, int group);
	int testRing(const Vec3f* at, int mark, f32 radius);
	void checkHits();
	void updateCollider();
	void clearHitList();
	void addHit(u16 id);
	int inHitList(u16 id);

	HitVec* mPoints;      // _30
	u32* mPointFlags;     // _34 1 outside the band 2 unused 4 hit
	BeamSprite* mSprites; // _38 3 per point
	CharaColli* mColli;   // _3C
	int mYaw;             // _40
	Vec3f mPos;           // _44
	f32 mGrow;            // _50
	Vec3f mDir;           // _54 y is the rise a frame x and z a unit flat direction
	f32 mLife;            // _60
	u16 mHitIds[32];      // _64
	int mHitCount;        // _A4
};

/** The arena the beam flies in */
struct BeamArena {
	f32 mCentreX;   // _00
	f32 mCentreZ;   // _04
	f32 mFloorY;    // _08
	f32 mCeilingY;  // _0C
	f32 mRadius;    // _10
	void* mHitFx;   // _14 0 = none
	void* mFxSet;   // _18
	void* mFxFlat;  // _1C
	void* mFxRound; // _20
};

extern "C" {
extern const char* gTEneBoss03_Beam_PeraName;
}

#endif
