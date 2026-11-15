#ifndef _TBATTLEMODELPLAYER_H
#define _TBATTLEMODELPLAYER_H
struct GjActor;

#include "engine/MotionPlayer.h"
struct DrawTask;
#include "stage/Stage.h"

extern "C" void objHeapFree(void*, void*);

/**
 * One fighter on the battle screen. mKind picks the model and a 3-entry motion table.
 * While mFalling is set, draw() lowers mPos.z and spins mAngle.v[1].
 *
 * @size 0xB8
 */
// The DOL copies the angle as a block paired loads so it is a struct holding an array
struct BattleAngle {
	int v[3];
};

class TBattleModelPlayer : public TObject {
public:
	TBattleModelPlayer(TObject* parent, const Vec3f* pos, int* angle, int kind);
	virtual ~TBattleModelPlayer();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_10(); // _10 empty
	virtual void render(); // _1C

	void setup(int kind); // 0x801AE850
	void updateMatrix();  // 0x801AEA08
	void stepMotion();    // 0x801AEA4C
	void play(s16 id);    // 0x801AEAEC
	void startFall();     // 0x801AEB28

	GjActor* mActor;      // _20
	f32 mMtx[12];         // _24
	Vec3f mPos;           // _54
	BattleAngle mAngle;   // _60
	f32 mDrop;            // _6C
	DrawTask* mDraw;      // _70
	MotionPlayer mMotion; // _74
	s16 mMotionId;        // _B0
	u16 _B2;              // _B2
	int mFalling;         // _B4
};

class TObject;
struct Vec3f;

extern "C" {
#ifndef TBATTLEMODELPLAYER_CPP
// Callers see the kind as an s8 TModeBattleSetup::init sign-extends it last
TBattleModelPlayer* createTBattleModelPlayer(TObject*, const Vec3f*, const int*, s8);
#endif
void freeBattleModelPlayer();
void loadBattleModelPlayer();
}

extern "C" {
extern const char* gTBattleModelPlayerName;
extern MotionEntry sBattleModelPlayerMotions[];
}

#endif
