#ifndef _TOBJPURPLEVFSNOW_H
#define _TOBJPURPLEVFSNOW_H
#include "Ninja/ninja.h"
struct DrawTask;
#include "core/types.h"
struct SnowCell;
struct SetDesc;
struct TObject;

#include "stage/SetActor.h"

extern "C" void objHeapFree(void*, void*);

/**
 * A field of snow the snowman head rolls through. One cell is 0x18: xyz, a texture coordinate, and a colour byte.
 * The head lowers cells it covers, and those cells creep back toward the heights resetSnowField stored.
 *
 * @size 0x80
 */
class MeshPlate;

class TObjPurpleVFSnow : public TSetActor {
public:
	TObjPurpleVFSnow(TObject* parent, SetDesc* desc);
	virtual ~TObjPurpleVFSnow();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual void vfn_74();

	void makeShape();
	void placeShape();
	void freeShape();
	void settle();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	DrawTask* mDrawTask; // _64
	Vec3f mCenter;       // _68 mPos plus the field offset
	f32 mRadius;         // _74 distance from mPos to mCenter
	MeshPlate* mPlate;   // _78 the floor's collision
	int mSoundWait;      // _7C frames until the roll sound can play again
};

extern "C" {
void initSnowField();
void resetSnowField();
void termSnowField();
}

extern "C" {
TObject* createTObjPurpleVFSnow(TObject*, SetDesc*);
}

// One vertex of the field
struct SnowCell {
	f32 x;     // _00
	f32 y;     // _04
	f32 z;     // _08
	f32 u;     // _0C
	f32 v;     // _10
	u8 color;  // _14
	u8 _15[3]; // _15

	inline SnowCell() { } // 0x800F8D14 for the array new
};

extern "C" {
extern const char* gTObjPurpleVFSnowName;
extern SnowCell* sSnowCells;
extern f32* sSnowBase;
}

#endif
