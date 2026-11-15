#ifndef _TMORPHEGGANIM_H
#define _TMORPHEGGANIM_H
struct Vec3f;

#include "stage/Stage.h"
#include "object/ObjModel.h"
class TParticleGenerator;

extern "C" void objHeapFree(void*, void*);

// MorphKey's base only pure virtuals so its vtable at 0x80255FB0 holds zeros
class MorphKeyBase {
public:
	MorphKeyBase() { _00 = 0; }

	u8 _00; // _00

	virtual int load(u8* data, int mode) = 0; // _08
	virtual int update()                 = 0; // _0C
};

// A float the script drives
// @size 0x1C
class MorphKey : public MorphKeyBase {
public:
	MorphKey();

	virtual int load(u8* data, int mode); // _08
	virtual int update();                 // _0C

	u8 mMode;    // _08 0 set 1 linear 2 hermite
	u8 _09;      // _09
	u16 mTime;   // _0A frames left
	u16 mSpan;   // _0C the hermite's starting count
	u8 _0E[2];   // _0E
	f32 mValue;  // _10
	f32 mTarget; // _14
	f32 mFrom;   // _18
};

extern "C" const u8 gMorphEggScript[12]; // 0x80242098

// Plays the byte script at gMorphEggScript
// 0x7F ends it 0x7B/0x7C loop 0x7D/0x7E mark a place and bit 7 holds a wait
// @size 0x58
class MorphScript {
public:
	MorphScript();

	void setScript(u8* script);
	void setSlot(int index, MorphKey* key);
	void update();
	u8* runCmd(u8* data, int mode);

	u8 mDone;            // _00 bit 0 = the script hit 0x7F
	u8 mLoops;           // _01
	u16 mWait;           // _02
	u8* mBase;     // _04
	u8* mPc;       // _08
	u8* mLoopAt;   // _0C
	u8* mMark;     // _10
	u32 mActive;         // _14 one bit per slot still running
	MorphKey* mSlot[16]; // _18
};

/**
 * Morphs an egg with a short byte script and tints a particle from the key's value.
 *
 * @size 0xCC
 */
class TMorphEggAnim : public TObject {
public:
	TMorphEggAnim(TObject* parent, const Vec3f* pos, f32 scale);
	virtual ~TMorphEggAnim();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw(); // _0C

	f32 mScale;         // _20
	Vec3f mPos;         // _24
	MorphScript mScript; // _30
	MorphKey mKey;      // _88
	ObjDrawState mState; // _A4 its tint mTev2 is the key value in every channel
	TParticleGenerator* mPart; // _C8 particleGenEmptyDefault
};

extern "C" {
extern const char* gTMorphEggAnimName;
}

#endif
