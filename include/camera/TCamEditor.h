#ifndef _CAMERA_TCAMEDITOR_H_
#define _CAMERA_TCAMEDITOR_H_
#include "camera/TCamera.h"
#include "stage/Stage.h"
union SpriteKey;
struct TCamEditor;
struct Sprite;
struct TCamera;

#include "core/types.h"

struct NJS_TEXLIST;

extern "C" {
int spriteAddTexList(NJS_TEXLIST*);
void spriteClear();
void spriteDrawAll();
void spriteRemoveTexList(int);
void spriteSetTexList(int, NJS_TEXLIST*);
void spritesInit();
void spritesTerm();
}

extern "C" {
TCamera* createTCamEditor(int, int, void*);
void spriteInit(Sprite*);
void spriteAdd(Sprite*);
int spriteFindTexture(u32, int*, int*);
}

class TCamEditor : public TCamera {
public:
	TCamEditor(TObject* parent, int player, int look, void* user);
	virtual ~TCamEditor();
	virtual void vfn_20(); // _20 0x80015A3C steers by the pad
	virtual void vfn_24(); // _24 0x80015C68 empty

	void init();      // 0x80015980
	void apply();     // 0x800159B0 puts the eye mDist from the point at mYaw and mPitch
	void nextSpeed(); // 0x80015C6C

	BOOL mMoveAt;  // _50 whether the stick moves the point when the camera isn't turning
	int mPitch;    // _54
	int mYaw;      // _58
	f32 mDist;     // _5C
	int mSpeed;    // _60 0 to 3 see editSpeed
};

struct SpriteKeyFields {
	u16 mFlags; // _00 3 the blend 0xC the texture coordinates 0x10 a point rather than a quad
	u8 mSlot;   // _02 which texture list
	u8 mTex;    // _03 which texture in it
};

/** What a sprite is drawn with */
union SpriteKey {
	SpriteKeyFields mField; // _00
	u32 mWord;              // _00
};

/** One queued debug sprite. @size 0x38 */
struct Sprite {
	Sprite() { }

	Sprite* mNext;  // _00
	Vec3f mView;    // _04 mPos in view space worked out when drawn
	Vec3f mPos;     // _10
	f32 mWidth;     // _1C half
	f32 mHeight;    // _20 half
	u32 mColor;     // _24
	f32 mPush;      // _28 moved this far toward the eye
	SpriteKey mKey; // _2C
	s16 mRoll;      // _30
	s16 mCoord;     // _32 which texture coordinates four per quad
	s16 mRotX;      // _34
	s16 mRotY;      // _36
};

extern "C" {
extern const f32 gSpriteCoords[8];
extern int gSpriteCount;
extern Sprite* gSpriteTail;
extern int gSpriteRoll;
}

void spriteDrawQuads(Sprite** list, int count);

extern "C" {
extern const f32 gSpriteCoordGrid[0x80];   // 0x802407D0 a grid of them
}

#endif
