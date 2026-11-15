#ifndef _TEGGSHOCKWAVE_H
#define _TEGGSHOCKWAVE_H
struct CharaColli;

#include "object/ObjAction.h"

extern "C" void objHeapFree(void*, void*);

/**
 * Cylinder left where an egg lands. Kind 3. onHit skips the source id and the holder.
 *
 * @size 0x54
 */
class TEggShockWave : public TObjAction {
public:
	TEggShockWave(f32 radius, TObject* parent, u16 source, void* tag);
	virtual ~TEggShockWave();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                              // _0C
	virtual Vec3f* getPos();                          // _20
	virtual int getKind();                            // _2C
	virtual int vfn_48(TObjAction* other);            // _48
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58
	virtual void setPos(const Vec3f* pos);            // _6C

	s16 mTimer;       // _30 draw count 0 plays the effect 0x14 sets the delete flag
	u16 mSource;      // _32 object id passed to the constructor
	s16 mPower;       // _34 written by the caller after construction
	u16 _36;          // _36
	BitFlags mHitFlags; // _38 copied into the hit
	CharaColli* mShape;     // _3C
	f32 mRadius;      // _40
	Vec3f mPos;       // _44
	void* mTag;       // _50 copied to HitInfo + 0x14
};

extern "C" {
extern const char* gTEggShockWaveName;
}

#endif
