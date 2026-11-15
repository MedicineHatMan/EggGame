#ifndef _TPLYCOMB_H
#define _TPLYCOMB_H

#include "stage/Stage.h"
#include "engine/Light.h"
#include "egg/ObjEgg.h"
#include "engine/CombVec.h"

template <typename T>
struct CombList {
	T* mData;   // _00
	int mCount; // _04
	int mCap;   // _08

	void reserve(int count);
	CombList(int count);
	~CombList();
};

template <typename T>
CombList<T>::CombList(int count)
{
	reserve(count);
	mCount = count;
}

template <typename T>
CombList<T>::~CombList()
{
	if (mData != 0) {
		delete[] mData;
	}
}

template <typename T>
void CombList<T>::reserve(int count)
{
	mData = new T[count];
	mCap  = count;
}

extern "C" void objHeapFree(void*, void*);

// One link of a strand
struct CombLink {
	CombLink() { } // 0x8004658C emitted for the array

	Vec3f mPos;      // _00
	Vec3f mPrev;     // _0C
	Vec3f mForce;    // _18
	Vec3f mDir;      // _24
	f32 mLen;        // _30
	f32 mRadius;     // _34
	f32 _38;         // _38
	f32 mLimit;      // _3C
	f32 mRest;       // _40
	CombLink* mBack; // _44
	CombLink* mNext; // _48
	f32 mMtx[12];    // _4C
	f32 mLocal[12];  // _7C
	f32 mWorld[12];  // _AC
};

// Five links and the matrices they follow
struct CombGroup {
	CombLink* mLink; // _00
	int mCount;      // _04

	void fill(f32* mtx);             // 0x800453F0
	void drag(f32* mtx);             // 0x800459D8
	void push(f32* mtx);             // 0x80045AB0
	void solve(f32* root);           // 0x80045B40
	void limit();                    // 0x80045D98
	void separate(CombGroup* other); // 0x80045F84
};

/**
 * The strands that hang off the player. draw() swings them and skins the model.
 *
 * @size 0x11C
 */
class TPlyComb : public TObject {
public:
	TPlyComb(TObject* parent);
	virtual ~TPlyComb();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw(); // _0C

	// The names below are ours
	void build(void* model);                // 0x80045210
	void getJoint(int index, f32** out);    // 0x800455D0
	void drawModel(void* state);            // 0x80045674
	void setRoot(f32* mtx);                 // 0x800456F4
	void readPose(void* motion, f32 frame); // 0x80045968 the motion at that frame

	u8 mOn;                    // _20
	u8 _21;                    // _21
	s16 mBits;                 // _22 bit 0x40 means the root just moved
	u8 _24[4];                 // _24
	CombVec<u8> mStore;        // _28
	CombVec<HitVec> mHits;     // _34
	void* mTexture;            // _40
	PointLight mLight;         // _44
	f32* mRoot;                // _C0
	CombGroup mGroup[3];       // _C4
	CombList<CombLink> mLinks; // _DC
	u32 _E8;                   // _E8
	f32 mMtx[12];              // _EC
};

extern "C" {
extern const char* gTPlyCombName;
}

#endif
