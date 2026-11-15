#ifndef _OBJMODEL_H
#define _OBJMODEL_H
struct NJS_OBJECT;
struct GjActor;

#include "core/types.h"
struct NJS_TEXLIST;
#include "stage/Stage.h"

class MotionPlayer;

// How a model is drawn
// See "Drawing and collision" in docs/notes.md
enum DrawStateFlag {
	DRAWSTATE_MATERIAL = 0x1,    // use mMaterial
	DRAWSTATE_TEV1     = 0x10,   // use mTev1
	DRAWSTATE_TEV2     = 0x20,   // use mTev2
	DRAWSTATE_BLEND    = 0x100,  // use mBlend
	DRAWSTATE_AMBIENT  = 0x200,  // use mAmbient in place of the normal ambient source
	DRAWSTATE_EXTRA    = 0x2000, // use mExtra
};

/**
 * @size 0x24
 */
struct DrawState {
	u32 mFlags;    // _00 DrawStateFlag 0x400 0x800 and 0x1000 select special passes
	u32 mMaterial; // _04 RGBA
	u32 _08;       // _08
	u32 mTev1;     // _0C RGBA
	u32 mTev2;     // _10 RGBA
	u32 mAmbient;  // _14 RGBA sent to GXSetChanAmbColor
	int mBlend;    // _18 0 = alpha blend 1 = additive
	void* mExtra;  // _1C
	void* _20;     // _20 a colour used by pass 0x1000
};

/**
 * A DrawState kept in an object. Building one clears only the flags and the pass-0x1000 colour, ahead of the
 * owner's constructor body. A DrawState made on the stack is not cleared. The name is ours.
 *
 * @size 0x24
 */
struct ObjDrawState : public DrawState {
	inline ObjDrawState()
	{
		mFlags = 0;
		_20    = 0;
	}

	// Copies a whole block as from getDefaultDrawState
	inline void operator=(const DrawState& other) { *(DrawState*)this = other; }
};

/**
 * A position, rotation and scale with the matrix built from them.
 *
 * @size 0x5C
 */
struct ObjPlace {
	ObjPlace();
	void update();

	u32 mFlags;      // _00 1 = use mPos 2 = use mRot 4 = use mScale
	Vec3f mPos;      // _04
	f32 mRot[4];     // _10
	Vec3f mScale;    // _20
	f32 mMatrix[12]; // _2C
};

/**
 * One model with what it needs to draw: an actor when the model asks for one, a motion, and per-node places.
 * The class name is ours.
 *
 * @size 0x20
 */
class ObjModel {
public:
	ObjModel();
	~ObjModel();

	void setModel(void* model);
	void setModelNodes(void* model, int nodes);
	void release();
	void calcMotion();
	void draw();

	NJS_OBJECT* mModel;    // _00
	GjActor* mActor;       // _04 allocated when the model has flag 0x2000
	DrawState* mState;     // _08
	MotionPlayer* mMotion; // _0C
	ObjPlace* mPlace;      // _10
	ObjPlace* mNodes;      // _14 one per node from setModelNodes
	NJS_TEXLIST* mTexList; // _18
	u32 mFlags;            // _1C 1 = has mNodes
};

extern "C" {
int objModelLinkNodes(NJS_OBJECT*, ObjPlace*, int);
void objModelNodeCallback();
int objModelNodes(NJS_OBJECT*, ObjPlace*, int);
}

extern "C" {
extern ObjPlace* sModelCallbackNodes;
extern int sModelCallbackIndex;
}

#endif
