#ifndef _MODELDRAW_H
#define _MODELDRAW_H
struct DrawState;

#include "core/types.h"
struct NJS_MOTION;
struct NJS_TEXLIST;

struct NJS_OBJECT;

/**
 * Plays one motion by frame. mRate is a multiplier, so an owner can slow or stop it without losing its place.
 *
 * @size 0x1C
 */
struct MotionTrack {
	MotionTrack();                                                        // 0x8005202C
	~MotionTrack();                                                       // 0x80052038
	void set(void* motion, int loop, f32 speed);                          // 0x80052074 plays all of a motion
	void setRange(void* motion, f32 start, f32 end, int loop, f32 speed); // 0x800520CC
	void update();                                                        // 0x800520FC
	int isDone();                                                         // 0x80052154 never for a looping track

	NJS_MOTION* getMotion() const { return mMotion; }
	f32 getFrame() const { return mFrame; }

	NJS_MOTION* mMotion; // _00
	f32 mStart;          // _04
	f32 mEnd;            // _08
	f32 mSpeed;          // _0C frames per step
	int mLoop;           // _10
	f32 mFrame;          // _14
	f32 mRate;           // _18
};

/** What ModelDraw::walkNodes calls for each node with the node's matrix current */
typedef void (*ModelNodeFunc)(int node, void* arg);

/**
 * What draws a model: the model, how to draw it, and a track and matrix when it has them.
 *
 * @size 0x1C
 */
struct ModelDraw {
	ModelDraw();                               // 0x80052180
	~ModelDraw();                              // 0x800521A4
	void setModel(void* model);                // 0x800521E0
	void setNodeFunc(void* func, void* arg);   // 0x800521E8
	void walkNodes();                          // 0x800521F4
	int walkNode(NJS_OBJECT* node, int index); // 0x8005227C returns the next node index
	void draw();                               // 0x80052300

	NJS_OBJECT* mModel;    // _00 an NJS_OBJECT
	DrawState* mDrawState; // _04
	NJS_TEXLIST* mTexList; // _08
	MotionTrack* mTrack;   // _0C
	f32* mMatrix;          // _10
	void* mNodeFunc;       // _14 a ModelNodeFunc called for each node by walkNodes
	void* mNodeArg;        // _18
};

#endif
