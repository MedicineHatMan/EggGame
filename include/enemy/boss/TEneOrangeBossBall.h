#ifndef _TENEORANGEBOSSBALL_H
#define _TENEORANGEBOSSBALL_H
struct CharaColli;

#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "engine/DrawTask.h"
#include "engine/TParticle.h"

enum OrangeBallState {
	BALL_HELD   = 0, // in the boss's hand
	BALL_FLYING = 1,
	BALL_LANDED = 2, // a short pause on the floor
	BALL_SWELL  = 3, // grows to 33 then deletes itself
};

/**
 * @size 0xA4
 */
class TEneOrangeBossBall : public TObjAction {
public:
	TEneOrangeBossBall(TObject* parent, const Vec3f* pos); // 0x8012227C
	virtual ~TEneOrangeBossBall();                         // 0x801223F4

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                                              // _0C 0x80122798
	virtual void vfn_10();                                            // _10 0x80122914
	virtual void render();                                            // _1C 0x80122974
	virtual Vec3f* getPos();                                          // _20 0x80122C88
	virtual int getKind();                                            // _2C 0x80122C90
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58 0x80122484

	void placeColli();                 // 0x8012252C
	void placeShadow();                // 0x80122574
	void keepParticles();              // 0x801225D8
	void stopParticles();              // 0x8012265C
	void setPos(const Vec3f* pos);     // 0x80122698
	void throwAt(const Vec3f* target); // 0x801226B4 a lob that lands on target in 60 frames
	void drawBall();                   // 0x801229A0

	Vec3f mPos;                        // _30
	Vec3f mVel;                        // _3C
	f32 mGravity;                      // _48 added to mVel.y each frame
	f32 mRadius;                       // _4C
	CharaColli* mShape;                // _50
	DrawTask* mDraw;                   // _54
	int mState;                        // _58 an OrangeBallState
	int mTimer;                        // _5C
	int mColor;                        // _60 0 red 1 blue 2 yellow
	TParticleGenerator* mParticles[2]; // _64
	ObjShadow mShadow;                 // _6C
};

extern "C" {
void orangeBallLoad();    // 0x80122234 takes the boss's model and textures for the balls
void orangeBallRelease(); // 0x8012226C
}

extern "C" {
extern const char* sOrangeBallName;
}

#endif
