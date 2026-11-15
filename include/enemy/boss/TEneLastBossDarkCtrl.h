#ifndef _TENELASTBOSSDARKCTRL_H
#define _TENELASTBOSSDARKCTRL_H

#include "object/ObjAction.h"
#include "sound/SeHolder.h"
#include "enemy/boss/BossParts.h"
#include "egg/ObjEgg.h"
#include "engine/DrawTask.h"

struct CharaColli;

// The darkness the last boss stands in a 256 by 256 I8 texture laid over a 20 by 20 sheet of points

/**
 * The pixels of one patch of the dark texture, in a shuffled order, with a state for fading them. Linked
 * into a list whose head is TEneLastBossDarkCtrl::mMaps.
 *
 * @size 0x40
 */
struct DarkPixelMap {
	DarkPixelMap(f32 radius, DarkPixelMap* prev, const Vec3f* pos, int id, int hold);
	~DarkPixelMap();

	int* getPixels();
	void setCount(int count);
	void shuffle();
	int getLastCount();
	void fill(u8* pixels, int value);
	void clear(u8* pixels);
	void startFade(u8* texture);
	int update(u8* pixels);

	DarkPixelMap* mPrev; // _00
	DarkPixelMap* mNext; // _04
	Vec3f mPos;          // _08
	f32 mRadius;         // _14
	int mId;             // _18 from TEneLastBossDarkCtrl::mNextId
	int mHold;           // _1C
	int mCount;          // _20
	int* mPixels;        // _24
	int mCursor;         // _28
	int mPerFrame;       // _2C
	int mLast;           // _30
	int mSeen;           // _34
	int mState;          // _38
	int mTimer;          // _3C
};

/**
 * mFlags: 1 = gone (no maps left), 2 = the texture changed, 4 = no contact, 8 = fading out, 0x10 = collision off.
 *
 * @size 0x2E5C
 */
class TEneLastBossDarkCtrl : public TObjAction {
public:
	TEneLastBossDarkCtrl(TObject* parent);
	virtual ~TEneLastBossDarkCtrl();
	virtual void draw();                                        // _0C 0x8015C8D4
	virtual void vfn_10();                                      // _10 0x8015CCE4
	virtual void render();                                      // _1C 0x8015CD60
	virtual Vec3f* getPos();                                    // _20 0x8015D2B0
	virtual int getKind();                                      // _2C 0x8015D2A8
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58 0x8015C618

void setPos(const Vec3f* pos);                 // 0x8015B9C8
	void deleteMaps();                             // 0x8015BA34
	void clearAll();                               // 0x8015BAA4
	void show();                                   // 0x8015BAF0
	void hide();                                   // 0x8015BB08
	void fadeOut();                                // 0x8015BB20
	void setNoContact();                           // 0x8015BB5C
	void clearNoContact();                         // 0x8015BB98
	void toTexel(f32 radius, const Vec3f* pos, f32* x, f32* z, f32* r); // 0x8015BBE8
	int getBounds(f32 x, f32 z, f32 r, f32* x0, f32* x1, f32* z0, f32* z1); // 0x8015BC44
	int addMap(f32 radius, const Vec3f* pos, int hold); // 0x8015BD94
	int paint(f32 radius, const Vec3f* pos, u8 value);  // 0x8015C050
	void clearMap(int id);                         // 0x8015C2A0
	void fadeMaps();                               // 0x8015C318
	int countMaps();                               // 0x8015C3A4
	int getMap(int index, Vec3f* pos, f32* radius); // 0x8015C414
	int hasMap(int id);                            // 0x8015C4A8
	int isInDark(const TObjAction* obj);                // 0x8015C4DC
	int touchContacts();                           // 0x8015C6D0

	Vec3f mPos;                // _30
	DrawTask* mDraw;           // _3C
	DarkPixelMap* mMaps;       // _40 an empty head the maps follow it
	CharaColli* mShape;        // _44
	int mNextId;               // _48
	HitVec mPoint[400];        // _4C the sheet at rest
	HitVec mPoint2[400];       // _130C the sheet with its ripples
	u8 mLive[400];             // _25CC which points are rippling
	f32 mVel[400];             // _275C each point's speed up and down
	int mRippleTimer;          // _2D9C
	int mTexAngleA;            // _2DA0
	int mTexAngleB;            // _2DA4
	f32 mScaleX;               // _2DA8
	f32 mScaleZ;               // _2DAC
	int mPulseX;               // _2DB0
	int mPulseZ;               // _2DB4
	u32 mDarkFlags;            // _2DB8
	int mAlpha;                // _2DBC
	f32 mPhase;                // _2DC0
	f32 mPhaseRate;            // _2DC4
	Vec3f mHitPos;             // _2DC8
	int mHitTimer;             // _2DD4
	SeHolder mSe;              // _2DD8
	int mFxKind;               // _2E0C
	int mFxHit;                // _2E10
	int mFxGone;               // _2E14
	int mSeId;                 // _2E18
	DarkFlashSlot mFlashes[8]; // _2E1C
};

extern "C" {
extern u8* gDarkTexture;               // 0x802C57C8 TEneLastBoss allocates it
extern u32 gDarkTextureSize;           // 0x802C57CC
extern TEneLastBossDarkCtrl* gDarkCtrl; // 0x802C57D0

void darkCtrlSetFx(int kind, int hit, int gone); // 0x8015B95C
void darkCtrlSetSe(int id);                      // 0x8015B980
void darkCtrlSetPos(const Vec3f* pos);           // 0x8015B994
void darkCtrlClearAll();                         // 0x8015BA78
void darkCtrlFadeOut();                          // 0x8015BB30
void darkCtrlSetNoContact();                     // 0x8015BB6C
void darkCtrlClearNoContact();                   // 0x8015BBA8
void darkCtrlSetPhaseRate(f32 rate);             // 0x8015BBD4
int darkCtrlAddMap(f32 radius, const Vec3f* pos, int hold); // 0x8015BD50
int darkCtrlPaint(f32 radius, const Vec3f* pos, u8 value);  // 0x8015C00C
void darkCtrlClearMap(int id);                   // 0x8015C26C
void darkCtrlFadeMaps();                         // 0x8015C2EC
int darkCtrlCountMaps();                         // 0x8015C370
int darkCtrlGetMap(int index, Vec3f* pos, f32* radius); // 0x8015C3C8
int darkCtrlHasMap(int id);                      // 0x8015C46C
void darkPulse(f32* scale, int* timer);          // 0x8015C804

// TEneLastBossFeather's at the end of this unit
void lastBossFeatherInit(); // 0x8015D2B8
void lastBossFeatherTerm(); // 0x8015D2F4
u8* lastBossFeatherTable(); // 0x8015D308
}

extern "C" {
extern const char* gTEneLastBossDarkCtrlName;
extern const int gLastBossFeatherNodes[];
}

#endif
