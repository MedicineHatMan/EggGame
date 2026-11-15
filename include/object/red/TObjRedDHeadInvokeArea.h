#ifndef _TOBJREDDHEADINVOKEAREA_H
#define _TOBJREDDHEADINVOKEAREA_H
struct AreaList;
struct SetDesc;
struct TObject;

#include "stage/SetActor.h"
#include "engine/DrawTask.h"

extern "C" void objHeapFree(void*, void*);

/**
 * One node of the live-area list at gRedAreaList. The owner is the area.
 * @size 0x10
 */
struct RedAreaLink {
	RedAreaLink* mNext; // _00
	RedAreaLink* mPrev; // _04
	void* mOwner;       // _08
	void* mList;        // _0C
};

/**
 * A volume that opens the dino heads of its group. Set int 0 is the group, set int 1 is the shape
 * (0 cylinder, 1 sphere), set float 0 is the radius and set float 1 is the height. While something
 * is inside, the matching heads stay open and a falling object is spawned every 48 frames. A head
 * whose switch is already set shuts the area.
 *
 * @size 0x90
 */
class TObjRedDHeadInvokeArea : public TSetActor {
public:
	TObjRedDHeadInvokeArea(TObject* parent, SetDesc* desc);
	virtual ~TObjRedDHeadInvokeArea();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void*);           // _34
	virtual void setup();                     // _74

	// shut is what a finished head calls
	void makeColli();
	void makeShape();
	void placeShape();
	void freeShape();
	void shut();
	void drop();

	RedAreaLink mLink;   // _64
	DrawTask* mDrawTask; // _74
	s16 mGroupId;        // _78 set int 0
	s16 mColType;        // _7A set int 1 clamped to 0 or 1
	f32 mRadius;         // _7C set float 0
	f32 mHeight;         // _80 set float 1
	int mInside;         // _84 non-zero while a contact is held
	s16 mTimer;          // _88
	s16 _8A;             // _8A
	int mClosed;         // _8C set by shut
};

extern "C" {
TObject* createTObjRedDHeadInvokeArea(TObject*, SetDesc*);
}

// The live areas built by the file's __sinit
struct AreaList {
	AreaList();  // 0x800C7070
	~AreaList(); // 0x800C7084

	RedAreaLink* mHead; // _00
	RedAreaLink* mTail; // _04 what the head walks
	int mCount;         // _08
};

extern "C" {
extern char* sRedDHeadColTypeNames[];
extern AreaList gRedAreaList;
}

#endif
