#ifndef _TOBJPURPLEICETRAP_H
#define _TOBJPURPLEICETRAP_H
struct TArc;
struct SetDesc;
struct TObject;
struct CharaColli;

#include "stage/SetActor.h"
struct NJS_OBJECT;
struct NJS_TEXLIST;
#include "engine/ModelDraw.h"
#include "engine/DrawTask.h"
class TParticleGenerator;

extern "C" void objHeapFree(void*, void*);

/**
 * An ice spike trap. Set int 0 is an eternal flag (kept to 0 or 1), set int 1 is added to a power of 25, set int 2
 * is added to an on-time of 120 and set int 3 to an off-time of 180. The body is one sphere and seven more march
 * out along the rotation. A non-eternal trap plays its motion shut, pops open, then cools and repeats.
 *
 * @size 0xF4
 */
class TObjPurpleIceTrap : public TSetActor {
public:
	TObjPurpleIceTrap(TObject* parent, SetDesc* desc);
	virtual ~TObjPurpleIceTrap();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual int onHit(const TObjAction* other, int arg, int* result);
	virtual void vfn_74();

	void placeShapes();
	void updateMatrix();
	void open();
	void hold();
	void cool();
	void eternal();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	DrawTask* mDrawTask; // _64
	ModelDraw mModel;    // _68 geobj model 4 and texture 1
	MotionTrack mTrack;  // _84
	CharaColli* mSpikes;       // _A0 seven spheres
	f32 mMatrix[12];     // _A4 placed at mPos and rotated
	int mAct;            // _D4 0 opening 1 on 2 cooling 3 eternal
	int mEternal;        // _D8 set int 0 clamped to 0 or 1
	int mPower;          // _DC set int 1 plus 25
	int _E0;             // _E0
	int mOnTime;         // _E4 set int 2 plus 120
	int mOffTime;        // _E8 set int 3 plus 180
	int mTimer;          // _EC
	TParticleGenerator* mSpray; // _F0 particle 0xE9
};

extern "C" {
void loadBalloonArc();
void makeOrangeBalloon();
void releaseBalloonArc();
}

extern "C" {
TObject* createTObjPurpleIceTrap(TObject*, SetDesc*);
}

extern "C" {
extern NJS_OBJECT** gBalloonModels;
extern NJS_TEXLIST** gBalloonTexLists;
extern void* gBalloonPosExport;
extern int sIceTrapVisible;
extern TArc* sBalloonArc;
extern void* sBalloonObj;
}

struct TrapTree;
struct TrapBox;
typedef void (*TrapHitFn)(u32 index, void* arg);
extern "C" {
void queryTrapNodes(TrapTree* tree, TrapBox* box, TrapHitFn fn, void* arg);
}

#endif
