#ifndef _TENEMYLIFEBAR_H
#define _TENEMYLIFEBAR_H

#include "stage/Stage.h"
struct DrawTask;

/**
 * @size 0x18
 */
struct LifePart {
	int mKind;   // _00
	u32 mFlags;  // _04
	f32 mLevel;  // _08 0 to 1
	f32 mDepth;  // _0C how far in front of the camera for the draw's depth
	f32 mX;      // _10 where on screen from the centre
	f32 mY;      // _14

	void init();                                 // 0x8004BFE4
	void hide();                                 // 0x8004C008
	void show();                                 // 0x8004C018
	void update();                               // 0x8004C028 grows or shrinks
	void track(Vec3f* pos, f32 size, f32 max);   // 0x8004C0C8
	int project(Vec3f* pos, f32 size, f32 max);  // 0x8004C128 0 off screen
	void render(u8* frame, f32 max, f32 value);  // 0x8004C318
};

/**
 * The bar above an enemy. The name is the one its constructor stores.
 *
 * Health is kept doubled, so a full bar of 50 is 100 here. mBlink runs 1 to 7 while mValue is at or under mMark.
 *
 * @size 0xAC
 */
class TEnemyLifeBar : public TObject {
public:
	TEnemyLifeBar(TObject* parent);
	virtual ~TEnemyLifeBar();

	inline void operator delete(void* mem);

	virtual void draw();   // _0C advances the parts and the blink
	virtual void vfn_10(); // _10
	virtual void render(); // _1C draws the part for this frame

	void setMax(s16 health);  // 0x8004C840 also sets the mark to a tenth
	void setMark(s16 health); // 0x8004C8A0
	void setValue(s16 health); // 0x8004C8F4
	void setSize(f32 rise, f32 height); // 0x8004C948
	void setPos(const Vec3f* pos);  // 0x8004C954 then lifts it by mRise
	void show();              // 0x8004C980
	void hide();              // 0x8004C9CC

	DrawTask* mTask;      // _20
	void* mTex;       // _24 first entry of the archive's texlists or 0
	Vec3f mPos;       // _28
	f32 mRise;        // _34
	f32 mHeight;      // _38
	f32 mMax;         // _3C
	f32 mValue;       // _40
	f32 mMark;        // _44
	s16 mBlink;       // _48
	u8 _4A[2];        // _4A
	LifePart mPart[4]; // _4C
};

extern "C" {
extern const u8 sLifeBarFrames[4];
extern const u8 sLifeBarBlinkFrames[12];
}

#endif
