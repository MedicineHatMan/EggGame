#ifndef _TAELAYER_H
#define _TAELAYER_H

#include "ui/TAeObject.h"


/**
 * One sprite inside a clip. _02 is the kind: 0 empty, 1 eight shorts plus a texture,
 * 2 a cell id, 4 a linked clip, 5 a draw function and a clip. The bytes at _04 change
 * with the kind.
 *
 * @size 0x18
 */
struct AeSlot {
	u16 mIndex;   // _00
	u16 _02;      // _02
	u8 _04[0x10]; // _04
	void* _14;    // _14
};

/**
 * The eight shorts of a sprite-table entry, 0x10 a sprite.
 *
 * @size 0x10
 */
struct AeUv {
	u16 mVals[8]; // _00
};

/**
 * A colour as four floats, 1.0 for full. Copied as a whole when a group copies its frame.
 *
 * @size 0x10
 */
struct AeColor {
	f32 a; // _00
	f32 r; // _04
	f32 g; // _08
	f32 b; // _0C
};

/**
 * What the AE draw code walks with: the file's two tables, the time, the depth, the mark bits
 * and the colour, plus the clip's slots. fillFrame builds one from a clip, and every group
 * drawn copies its parent's. Names are ours.
 *
 * @size 0x2C
 */
struct AeFrame {
	u32 mHead;      // _00 AeFile._00 the sprite table 0x10 per sprite
	void* mList;    // _04 AeRecord*
	f32 mTime;      // _08
	f32 mRate;      // _0C clip _58 aeDrawSprite uses it as the depth
	u16 mMark;      // _10 bit 2 draws additively
	s16 _12;        // _12 left untouched
	AeColor mColor; // _14 clip _6C-_78
	int mCount;     // _24
	AeSlot* mItems; // _28
};

extern "C" {
void aeDrawSprite(AeFrame* frame, const u32* banks);
void aeGetRange(void* list, u32 id, f32* start, f32* end);
void aeDrawGroup(AeFrame* frame, int index, const u32* banks);
void aeCollectSlots(void* list, s16 index, s16* ids, int max, int* count);
}

/**
 * Playback of one animation inside an AE file. The vtable sits at offset 0. TAeLayer
 * inherits this as its second base, so the vtable lives at TAeLayer+0x20. Names are ours.
 *
 * Bit 10 of mPlayFlags is set when playback finishes. The low 7 bits are the repeat
 * count from the constructor, forced up to 1. Bits 7-9 are copied from the flags argument.
 *
 * @size 0x88
 */
class TAeClip {
public:
	TAeClip(TAeObject* owner, u32 id, u32 flags);
	virtual ~TAeClip();

	inline void operator delete(void* mem) { ::operator delete(mem); }

	virtual void step();  // _0C
	virtual void paint(); // _10
	virtual void onEnd(); // _14 the base is empty

	void setup(TAeObject* owner, u32 id, u32 flags); // 0x800A7218
	void reset(u32 id, u32 flags);                   // 0x800A7368
	void fillFrame(AeFrame* frame);                  // 0x800A7618 name is ours
	int isDone();                                    // 0x800A7684 name is ours
	int containsTime(f32 time);                      // 0x800A7690 name is ours
	void apply();                                    // 0x800A76F4 name is ours
	void setTrans(f32 x, f32 y);                     // 0x800A776C name is ours
	void setScale(f32 x, f32 y);                     // 0x800A7778 name is ours
	void setSpin(f32 angle);                         // 0x800A7784 name is ours
	void setColor(f32* color);                       // 0x800A778C name is ours
	void setRate(f32 rate);                          // 0x800A77B0 name is ours
	void setMode(u32 mode);                          // 0x800A77B8 name is ours
	void show();                                     // 0x800A77E4 name is ours
	void hide();                                     // 0x800A77F4 name is ours
	void hold();                                     // 0x800A7804 name is ours
	void release();                                  // 0x800A7814 name is ours
	void cue(int mode);                              // 0x800A7824 name is ours
	void wake();                                     // 0x800A7870 name is ours
	void pause();                                    // 0x800A7884 name is ours
	void flipPause();                                // 0x800A7894 name is ours
	void home();                                     // 0x800A78C8 name is ours
	AeSlot* findSlot(int id);                        // 0x800A7930 name is ours
	void setSprite(u32 id, AeUv* src, void* tex);    // 0x800A7984 name is ours
	void setFrame(u32 id, int index);                // 0x800A7A1C name is ours
	void setCell(u32 id, int index);                 // 0x800A7A64 name is ours
	void attach(u32 id, TAeClip* other, int flags);  // 0x800A7AB0 name is ours
	void setHook(u32 id, u32 fn, u32 arg);           // 0x800A7AF8 name is ours
	void clearSlot(u32 id);                          // 0x800A7B40 name is ours
	void clearSlots();                               // 0x800A7B70 name is ours
	void* getAssets();                               // 0x800A7BA8
	u32 getId();                                     // 0x800A7BCC name is ours
	void bind(u32 id, TAeClip* other, u32 = 0);      // 0x800A7BD4 name is ours callers pass 0 third never read

	TAeObject* mOwner; // _04
	u32 mPlayFlags;    // _08 low 7 bits are the repeat count
	u32 mId;           // _0C
	f32 mTime;     // _10 advanced by step
	f32 mPrevTime; // _14
	f32 mStart;    // _18
	f32 mEnd;      // _1C
	f32 mMtx[12];      // _20
	f32 _50;           // _50
	f32 _54;           // _54
	f32 _58;           // _58
	f32 _5C;           // _5C
	f32 _60;           // _60
	f32 _64;           // _64
	f32 _68;           // _68
	f32 _6C;           // _6C
	f32 _70;           // _70
	f32 _74;           // _74
	f32 _78;           // _78
	int mCount;        // _7C
	AeSlot* mItems;    // _80
	const u32* mBanks; // _84 the palette bank for each of its sprites' palettes
};

/**
 * One playing AE animation, parented to the screen object that created it.
 * Second base is the clip, so the clip vtable is at _20.
 *
 * @size 0xB0
 */
class TAeLayer : public TObject, public TAeClip {
public:
	TAeLayer(TObject* parent, TAeObject* owner, int id, u32 flags);
	virtual ~TAeLayer();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void onEnd(); // _14 of the clip vtable

	void update();  // 0x800A7DF4 name is ours
	void present(); // 0x800A7E58 name is ours
	int play();     // 0x800A7E94 name is ours Moves the clip toward _A8

	f32 _A8; // _A8 the time play() moves toward
	s16 _AC; // _AC 0 waiting 1 toward a later time 2 toward an earlier time
	u16 _AE; // _AE
};

extern "C" {
extern const char* gTAeLayerName;
extern f32 gAeFrameTime;
}

void drawLinked(AeFile* file, int index, void* arg);

#endif
