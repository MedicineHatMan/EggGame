#ifndef _TOBJYELLOWTORNADOSCENE_H
#define _TOBJYELLOWTORNADOSCENE_H
struct TObject;
struct Fx;

#include "egg/ObjEgg.h"
#include "stage/Stage.h"

extern "C" void objHeapFree(void*, void*);

/**
 * A tornado on a path. Spawned, not placed. The path argument is clamped to 0..4 and picks path id 2 through 6.
 * The kind argument picks the effect: 0 is effect 2, anything else is effect 0x54. While no event is busy and the
 * landscape blend is 0, it rides the path, swings out on Y, and grows from 0.01 toward 1. Any other time the effect stops.
 *
 * @size 0x88
 */
class TObjYellowTornadoScene : public TObject {
public:
	TObjYellowTornadoScene(TObject* parent, int path, int kind);
	virtual ~TObjYellowTornadoScene();
	virtual void draw();
	virtual void vfn_10();

	void spawnFx();
	void updatePlace();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	EggCollide mRail; // _20
	Fx* mFx;        // _3C bank 8
	int mKind;        // _40 0 or 1 which effect id
	f32 mMatrix[12];  // _44 where the effect is drawn
	f32 mSwing;       // _74 50 times abs(sin) of the angle
	int mAngle;       // _78 advances 0x400 a frame
	Vec3f mScale;     // _7C starts at 0.01 and climbs to 1
};

extern "C" {
TObjYellowTornadoScene* createTObjYellowTornadoScene(TObject*, int, int);
}

extern "C" {
extern const char* gTObjYellowTornadoSceneName;
}

#endif
