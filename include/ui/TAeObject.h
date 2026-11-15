#ifndef _TAEOBJECT_H
#define _TAEOBJECT_H

#include "stage/Stage.h"

extern "C" void objHeapFree(void*, void*);

class TAeLayer;

/**
 * Header of a relocated AE file. relocateAe adds the file base to the two words and to the
 * records hanging off mList. A record is 0x38 bytes and a zero kind ends a group.
 *
 * @size 0x8
 */
struct AeFile {
	u32 _00;     // _00
	void* mList; // _04 first record
};

/**
 * One record of an AE file. Records come in groups ended by a zero kind. Kind 1 draws the
 * sprite mChild of the file's sprite table, kind 2 draws the group starting at record mChild,
 * kind 3 draws whatever the clip put in the slot with this record's index. The six key
 * lists are relocated in place by relocateAe, so they stay words here. Each list is a run of
 * a time and 1-3 values, ended by a negative time. Names are ours.
 *
 * @size 0x38
 */
struct AeRecord {
	u16 mKind;  // _00 0 ends the group
	u16 mChild; // _02
	u16 mFlags; // _04 or'ed into the frame's mark
	u8 _06[2];  // _06
	f32 mSizeX; // _08 half of it is the spin centre
	f32 mSizeY; // _0C
	f32 mRate;  // _10
	f32 mDelay; // _14
	f32 mStart; // _18
	f32 mEnd;   // _1C
	u32 mOff20; // _20 anchor keys 2 values
	u32 mOff24; // _24 position keys 2 values
	u32 mOff28; // _28 scale keys 2 values
	u32 mOff2C; // _2C spin keys in degrees
	u32 mOff30; // _30 alpha keys
	u32 mOff34; // _34 colour keys 3 values
};

/**
 * Base of the TAeObj* screen objects. The two pointers are the relocated AE file and the
 * asset block that came out of the same archive entry.
 *
 * @size 0x34
 */
class TAeObject : public TObject {
public:
	TAeObject(TObject* parent, void* data, void* assets);
	virtual ~TAeObject();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual int vfn_20(s16*) { return 0; } // _20 The argument is a message code the overrides read

	TAeLayer* createLayer(int id, u32 flags); // 0x800A710C
	void* getAssets();                        // 0x800A7104 name is ours

	void* mData;   // _20 AeFile*
	void* mAssets; // _24
	u32 _28;       // _28
	void* _2C;     // _2C
	u32 _30;       // _30 a subclass sets this when its layer finishes
};

// 0x800A7010
// Not a method
void relocateAe(void* file);

extern "C" {
extern const char* gTAeObjectName;
}

#endif
