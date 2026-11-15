#ifndef _TOBJORANGEMIRRORTRAP_H
#define _TOBJORANGEMIRRORTRAP_H
struct SetDesc;

#include "stage/SetActor.h"
#include "engine/DrawTask.h"

extern "C" void objHeapFree(void*, void*);

class TObjOrangeMirrorTrap;

struct MirrorTrapList;

/**
 * Link node kept on gMirrorTraps.
 *
 * @size 0x10
 */
struct MirrorTrapNode {
	MirrorTrapNode* mNext;        // _00
	MirrorTrapNode* mPrev;        // _04
	TObjOrangeMirrorTrap* mOwner; // _08
	MirrorTrapList* mList;        // _0C
};

/**
 * Every live trap, the same shape as KeyList. A static of TObjOrangeMirrorTrap.cpp, so its ctor and dtor are inline.
 *
 * @size 0xC
 */
struct MirrorTrapList {
	// Defined at the end of TObjOrangeMirrorTrap.cpp
	MirrorTrapList();
	~MirrorTrapList();

	void add(MirrorTrapNode* node);
	void remove(MirrorTrapNode* node);
	void clear();

	MirrorTrapNode* mTail; // _00
	MirrorTrapNode* mHead; // _04
	int mCount;            // _08
};

extern MirrorTrapList gMirrorTraps; // 0x80282844

/**
 * A mirror the player has to match. Set int 0 is the kind, clamped to 0..2, set int 1 is an enemy
 * group and set int 3 is the answer compared with the byte rolled for that kind. A wrong touch breaks
 * every trap of the same kind. A right touch opens the map mirror and breaks the others.
 *
 * @size 0x170
 */
class TObjOrangeMirrorTrap : public TSetActor {
public:
	TObjOrangeMirrorTrap(TObject* parent, SetDesc* desc);
	virtual ~TObjOrangeMirrorTrap();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void*);           // _34
	virtual void vfn_74();                    // _74

	int suitMatches();
	void shatter();
	void failOthers();
	void failOne();
	void clearOthers();
	void clearOne();
	void stateIdle();
	void stateGroup();
	void stateWaitGroup();
	void stateRecover();
	void stateBroken();
	void shrink();
	void grow();
	void drawMark();

	MirrorTrapNode mNode; // _64
	void* mModelA;        // _74 geobj model 10
	void* mModelB;        // _78 geobj model 9
	void* mTexA;          // _7C geobj texture 11
	void* mTexB;          // _80 geobj texture 10
	void* mTexC;          // _84 geobj texture 3
	DrawTask* mDrawTask;  // _88
	void* mMesh[3];       // _8C
	f32 mMatrix[12];      // _98
	f32 mBody[12];        // _C8
	f32 mFace[12];        // _F8 y scale follows mFaceScale
	f32 mMark[12];        // _128
	f32 mExtent;          // _158
	int mAnswer;          // _15C set int 3
	int mKind;            // _160 set int 0 clamped to 0..2
	int mGroup;           // _164 set int 1
	f32 mFaceScale;       // _168
	int mAct;             // _16C 0 idle 1 group 2 waiting 3 recover 4 broken
};

class TObject;
struct Vec3f;

extern "C" {
void createTMirrorBreaker(TObject*, const Vec3f*, f32, const Vec3f*, int);
}

extern "C" {
TObject* createTObjOrangeMirrorTrap(TObject*, SetDesc*);
}

extern "C" {
extern const char* sMirrorTrapIdNames[];
extern const char sMirrorTrapLabels[];
extern f32 sMirrorTrapTwo;
extern f32 sMirrorTrapThree;
extern const f32 sMirrorTrapSixty;
extern const f32 sMirrorTrapZero;
extern const f32 sMirrorTrapOne;
extern u32 sOrangeMirrorTrapClearColor;
extern const f32 sMirrorTrapMinusSixty;
extern const f32 sMirrorTrapOneTwenty;
extern const char* gTMirrorBreakerName;
}

void rollMirrorTrapAnswers();

#endif
