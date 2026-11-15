#ifndef _TOBJYELLOWTIMBERBRICK_H
#define _TOBJYELLOWTIMBERBRICK_H
struct SetDesc;
struct TObject;
struct MeshObject;

#include "stage/SetActor.h"
#include "object/ObjModel.h"
#include "sound/SoundHandle.h"
#include "engine/DrawTask.h"
#include "camera/EventCamRef.h"
#include "Ninja/ninja.h"

class TCamPath;

extern "C" void objHeapFree(void*, void*);

/**
 * The sand's height. There is one. Set int 0 and set int 1 are switches, clamped to 0..0x800, and set int 2 is kept
 * as an event id. Neither switch leaves the sand at its home height, the first raises it 100, and both raise it 200.
 * Message 0xC7 asks for the next step: 0 while it is home, or 1 while it is at 100. The sand floor and an EVENT sand
 * fall read that height from mPos.y plus mHeight, and the floor only exists while this object does.
 *
 * @size 0x130
 */
class TObjYellowTimberBrick : public TSetActor {
public:
	TObjYellowTimberBrick(TObject* parent, SetDesc* desc);
	virtual ~TObjYellowTimberBrick();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual int sendToHandle(ObjMsg* msg);
	virtual void vfn_74();

	void updatePlace();
	void updateColli();
	void startEvent();
	void stateWait();
	void stateEvent();
	void stateRise();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	NJS_OBJECT* mModelData; // _64 geobj model 3
	NJS_TEXLIST* mTexList;  // _68 geobj texture 3
	void* mColliData;       // _6C
	DrawTask* mDrawTask;    // _70
	MeshObject* mColli;     // _74
	ObjModel mModel;        // _78
	f32 mMatrix[12];        // _98
	u16 mLevel;             // _C8 0 home 1 raised once 2 raised twice
	f32 mHeight;            // _CC added to mPos.y 0 100 or 200
	u16 _D0;                // _D0
	u16 mSwitch1;           // _D2 set int 0
	u16 mSwitch2;           // _D4 set int 1
	u16 mEventId;           // _D6 set int 2
	EventCamRef mCam;       // _D8
	int mRequest;           // _E0 the level message 0xC7 asked for
	f32 mTarget;            // _E4 the height stateRise climbs to
	TCamPath* mEvent;       // _E8 from eventCamArcPlay
	u16 mTimer;             // _EC counts down from 180 before the rise
	SoundHandle mSound;     // _F0 0x25F while the sand is moving
	Vec3f mCenter;          // _11C
	f32 mRadius;            // _128
	int mAct;               // _12C 0 waiting 1 in the event 2 rising
};

extern "C" {
void timberBrickSetExit();
void timberBrickSetInit();
}

extern "C" {
TObject* createTObjYellowTimberBrick(TObject*, SetDesc*);
}

#endif
