#ifndef _TAEOBJCONSOLE1P_H
#define _TAEOBJCONSOLE1P_H
struct TArc;

#include "ui/TAeLayer.h"
struct DrawTask;

/**
 * A player's HUD: the life gauge, the egg hatch meter, the star counter, the score
 * and the banner. The constructor only clears the slots; one of the three setups builds
 * the layers. setupSolo is the one-player console, setupNumbered and setupPlain the battle
 * ones. draw() switches on mMode. TAeConsoleSet passes it the console messages.
 * Field names are ours.
 *
 * @size 0xC4
 */
class TAeObjConsole1P : public TAeObject {
public:
	TAeObjConsole1P(TObject* parent, void* data, void* assets);
	virtual ~TAeObjConsole1P();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void render(); // _1C

	void setupSolo();     // 0x800CF850
	void setupNumbered(); // 0x800CFBDC
	void setupPlain();    // 0x800CFFAC
	void openDone();      // 0x800D0460
	void eggBaseDone();   // 0x800D04E0
	void showHatch();     // 0x800D0564
	void hideHatch();     // 0x800D07EC
	void stepOpen();      // 0x800D0870
	void stepMain();      // 0x800D0918
	void placeBySlot();   // 0x800D0DB0
	void pulse(f32 step); // 0x800D0E50
	void place(f32 x, f32 y); // 0x800D0EB8

	void setValue(int value);              // 0x800D0F60 message 0xAD
	void setHatch(f32 time, s8 kind);      // 0x800D0FD4 message 0xAF
	void setStars(u16 count, u16 kind);    // 0x800D1010 message 0xB1
	void hitValue(int value);              // 0x800D118C message 0xB2
	void raiseLevel();                     // 0x800D1208 message 0xB3
	void setScore(int a, int b, int tenths); // 0x800D123C message 0xB4
	void showBanner(int which);            // 0x800D130C message 0xB5 -1 for none
	void hideBanner();                     // 0x800D1484 message 0xB6

	TAeLayer* mFrame;       // _34 the console frame everything is bound into
	TAeLayer* mOpen;        // _38 the opening animation of the solo console
	TAeLayer* mCells[5];    // _3C the life gauge
	TAeLayer* mEgg;         // _50 the egg meter frame
	TAeLayer* mEggBase;     // _54
	TAeLayer* mEggIcon;     // _58
	TAeLayer* mEggKind;     // _5C which egg by mKind
	TAeLayer* mEggTimer;    // _60 plays up to mHatchTime
	TAeLayer* mStarFrame;   // _64
	TAeLayer* mStars[5];    // _68
	TAeLayer* mScore;       // _7C
	TAeLayer* mBanner;      // _80
	TAeLayer* mBannerBase;  // _84
	TAeLayer* mBannerIcon;  // _88
	DrawTask* mTask;            // _8C
	u32 mFlags;             // _90 1 a cell blinks 2 hatching this frame 4 0x40 level raised
	int mWasHatching;       // _94
	int mBlinkCell;         // _98
	int mValue;             // _9C the life left 0-100
	f32 mHatchTime;         // _A0 0-1
	s8 mKind;               // _A4 the egg kind 0-3
	u8 _A5;                 // _A5
	u16 mStarMax;           // _A6
	u16 mStarCount;         // _A8
	u8 mLevel;              // _AA 0-3
	u8 _AB;                 // _AB
	int _AC;                // _AC
	u32 _B0;                // _B0
	f32 mPulse;             // _B4
	f32 mPulseStep;         // _B8
	int mMode;              // _BC 0 opening 1 solo 2 battle
	int mSlot;              // _C0 where a battle console sits on screen
};

class TObject;

extern "C" {
TAeObjConsole1P* createConsole1P(TObject*);
TAeObjConsole1P* createConsole1PB(TObject*, int);
TAeObjConsole1P* createConsole1PC(TObject*, int);
void freeConsole1P();
void loadConsole1P();
}

#include "Ninja/njtypes.h"
extern "C" {
extern TArc* sConsole1PArc;
extern void* sConsole1PAe;
extern NJS_TEXLIST* sConsole1PTexList;
}

#endif
