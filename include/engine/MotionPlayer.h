#ifndef _MOTIONPLAYER_H
#define _MOTIONPLAYER_H

#include "core/types.h"
struct NJS_MOTION;

enum MotionType {
	MOTION_LOOP  = 0,
	MOTION_ONCE  = 1, // holds the last frame and sets the done flag
	MOTION_CHAIN = 2, // goes straight on to mNext
	MOTION_BLEND = 3, // blends into mNext
};

/**
 * One row of a motion table. An owner's states index these.
 * @size 0x1C
 */
struct MotionEntry {
	s8 mType;       // _00
	s16 mMotion;    // _02 an index into the motions -1 = none
	s16 mShape;     // _04 an index into the second motion list -1 = none
	s16 mNext;      // _06 a row of this table
	f32 mStart;     // _08
	f32 mEnd;       // _0C
	f32 mRate;      // _10 frames per update can be negative
	f32 mBlendRate; // _14 for MOTION_BLEND
	int mUser;      // _18 the owner's own flags
};

// What the owner hands to the blending draw call
struct MotionBlend {
	NJS_MOTION* mFrom; // _00
	NJS_MOTION* mTo;   // _04
	f32 mFromFrame; // _08
	f32 mToFrame;   // _0C
};

/**
 * @size 0x3C
 */
struct MotionPlayer {
	MotionPlayer();

	void init(const MotionEntry* table, void* motions);
	void init(const MotionEntry* table, void* motions, void* shapes);
	void play(s16 id);
	void playBlend(s16 id, f32 rate);
	void playSynced(s16 id, f32 rate);
	void set(s16 id);
	int isBlending();
	int isDone();
	MotionBlend* getBlend();
	void update();
	void setMotion(s16 id);
	int advance();
	void startBlend();
	void endBlend();
	int passedFrame(f32 frame);
	u32 getUser(s16 id);
	s16 getType(s16 id);
	s16 getNext(s16 id);
	NJS_MOTION* getMotion(s16 id);
	void* getShape(s16 id);
	f32 getStart(s16 id);
	f32 getEnd(s16 id);
	f32 getRate(s16 id);
	f32 getBlendRate(s16 id);
	void getCurrent(void** motion, void** shape);

	s8 mState;           // _00 1 = started 2 = playing 3 = blending 4 = blending at the same frame
	s16 mCurrent;        // _02
	s16 mPrev;           // _04
	u32 mFlags;          // _08 1 = blending 2/4/8 = a pending play 0x10 = done 0x100 = reached the end
	f32 mFrame;          // _0C
	f32 mPrevFrame;      // _10
	f32 mBlendTime;      // _14 0 to 1
	f32 mBlendRate;      // _18
	f32 mSpeed;          // _1C scales every motion's rate
	MotionBlend mBlend;  // _20
	const MotionEntry* mTable; // _30
	NJS_MOTION** mMotions; // _34
	void** mShapes;      // _38
};

extern "C" {
}

extern "C" {
}

#endif
