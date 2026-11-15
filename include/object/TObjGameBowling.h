#ifndef _TOBJGAMEBOWLING_H
#define _TOBJGAMEBOWLING_H
struct SetDesc;
struct TObject;
struct CharaColli;
class TObjBowlEgg;

#include "stage/SetActor.h"
struct DrawTask;
#include "object/ObjModel.h"
#include "engine/MotionPlayer.h"
#include "egg/ObjEgg.h"

/**
 * One pin. It is a TObject of its own, with a collider, three points down its middle and their speeds.
 * The bowling egg knocks it over in state 0, it falls in state 1, and state 2 does nothing. The names are ours.
 * Vtable 0x8025887C. 0x800A07D0-0x800A14C0.
 *
 * @size 0xB0
 */
class TBowlPin : public TObject {
public:
	TBowlPin();          // 0x800A07D0
	virtual ~TBowlPin(); // 0x800A0858

	virtual void draw();   // _0C 0x800A0CE8 one step of the state
	virtual void render(); // _1C 0x800A141C a debug shadow

	void reset();                     // 0x800A08C8
	void makeShape();                 // 0x800A0940
	void updateShape();               // 0x800A09A0
	void setPos(const Vec3f* pos);    // 0x800A0A10
	void setPos(f32 x, f32 y, f32 z); // 0x800A0A3C
	int collide(TBowlPin* other);     // 0x800A0AB0 pushes two pins apart 1 if they touched
	void checkHit();                  // 0x800A0D54 state 0
	void fall();                      // 0x800A0F84 state 1
	void rest();                      // 0x800A10CC state 2 empty
	void move();                      // 0x800A10D0 gravity and the floor for both ends
	void keepLength();                // 0x800A1358
	void updateMatrix();              // 0x800A13D8

	CharaColli* mShape; // _20 the collider
	HitVec mPos[3];     // _24 bottom middle and top
	HitVec mVel[3];     // _48 the same three points' speeds
	f32 mQuat[4];       // _6C
	f32 mMtx[12];       // _7C
	s16 mHits;          // _AC how many times it has been hit
	s16 mState;         // _AE
};

/**
 * A bowling lane, class 0x18. The player stands by it with a bowling egg, swings the arm to aim, and rolls the egg
 * at the ten pins. Fruit drops for the pins that went down. Vtable 0x8025889C. 0x8009F734-0x800A07D0.
 *
 * @size 0x864
 */
class TObjGameBowling : public TSetActor {
public:
	TObjGameBowling(TObject* parent, SetDesc* desc); // 0x8009F7E8
	virtual ~TObjGameBowling();                      // 0x8009F8F4

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                            // _0C 0x8009FB00
	virtual void vfn_10();                          // _10 0x800A0698 hides the draw task when off screen
	virtual void render();                          // _1C 0x800A0708
	virtual void getNodeMatrix(int node, f32* mtx); // _24 0x800A0574
	virtual void initFromDesc(SetDesc* desc);       // _30 0x8009F788
	virtual void debugPrint(void*);                 // _34 0x8009F7A8
	virtual int vfn_50();                           // _50 0x800A056C
	virtual void setup();                           // _74 0x8009F9A8

	void reset();                     // 0x8009FA78
	void phaseWait();                 // 0x8009FC24 0 waits for an egg and the player
	void phaseRise();                 // 0x8009FDBC 1
	void phaseAim();                  // 0x8009FDEC 2
	void phaseThrow();                // 0x8009FEA0 3
	void phaseRoll();                 // 0x8009FFA0 4
	void phaseCount();                // 0x800A00B8 5 blinks the pins
	void phaseLower();                // 0x800A014C 6
	void dropFeed();                  // 0x800A0194
	void placeShape();                // 0x800A0310
	void setupPins();                 // 0x800A0374
	void releaseEgg(TObjAction* egg); // 0x800A0568 empty The bowling egg calls it on the lane it came from
	int checkEgg();                   // 0x800A0634 0 when the egg is gone
	void setShowPins(int show);       // 0x800A0700

	DrawTask* mDraw;      // _64
	ObjModel mModel;      // _68 the lane
	ObjModel mArmModel;   // _88
	ObjModel mPinModel;   // _A8 drawn once for each pin
	MotionPlayer mMotion; // _C8 the arm's
	f32 mBaseMtx[12];     // _104
	f32 mArmMtx[12];      // _134
	s16 mAim;             // _164 the arm's angle from the lane
	s16 mAimSpeed;        // _166
	f32 mArmY;            // _168
	u16 mEggId;           // _16C
	TObjBowlEgg* mEgg;    // _170
	u16 mPlayerId;        // _174
	TObjAction* mPlayer;  // _178
	TBowlPin mPins[10];   // _17C
	int mShowPins;        // _85C
	s16 mPhase;           // _860
	s16 mTimer;           // _862
};

extern "C" {
TObject* createTObjGameBowling(TObject*, SetDesc*);
}

extern "C" {
extern const char* gTObjBowlEggName;
extern const char* gTObjGameBowlingName;
extern f32 sGameBowlingEmpty;
extern const MotionEntry sGameBowlingMotions[];
}

#endif
