#ifndef _TOBJPURPLEICEGEARL_H
#define _TOBJPURPLEICEGEARL_H
#include "Ninja/ninja.h"
#include "core/types.h"
#include "stage/Stage.h"
struct GearMesh;
struct GearNode;
struct SetDesc;
struct TObject;

#include "stage/SetActor.h"
#include "object/ObjModel.h"
#include "sound/SoundHandle.h"
#include "engine/DrawTask.h"
#include "camera/EventCamRef.h"

class TCamPath;

extern "C" void objHeapFree(void*, void*);

/**
 * A large ice gear. Set int 0 is the spin step, set int 2 is the switch, and set float 0 is added to a scale of 1.
 * While the switch is clear it spins and plays 0x235. An event camera slows it to a stop, and message 0x8E follows.
 *
 * @size 0x154
 */
class TObjPurpleIceGearL : public TSetActor {
public:
	TObjPurpleIceGearL(TObject* parent, SetDesc* desc);
	virtual ~TObjPurpleIceGearL();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual int sendToHandle(ObjMsg* msg);
	virtual void vfn_74();

	void spin();
	void ease();
	void settle();
	void stopped();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	void* mGeo;          // _64 MODEL_ICE_GEAR_L
	void* mTex;          // _68 TLS_MODEL_ICE_GEAR_L
	DrawTask* mDrawTask; // _6C
	ObjModel mModel;     // _70
	f32 mMatrix[12];     // _90
	f32 mSpin[12];       // _C0 mMatrix rolled by mAngle
	f32 mScale;          // _F0 1 plus set float 0
	int mAngle;          // _F4
	int mRotSpeed;       // _F8 set int 0 ease() shrinks it
	TObject* mHide0;     // _FC guess cleared from here never stored
	TObject* mHide1;     // _100 guess
	u16 mSwitch;         // _104 set int 2
	u16 _106;            // _106
	EventCamRef mCam;    // _108
	TCamPath* mEvent;    // _110 from eventCamArcPlay
	SoundHandle mSound;  // _114
	Vec3f mCentre;       // _140
	f32 mRadius;         // _14C
	int mAct;            // _150 0 spinning 1 easing 2 settling 3 stopped
};

extern "C" {
void iceGearSetExit();
void iceGearSetInit();
void initPurpleIceGearLSet();
void termPurpleIceGearLSet();
}

extern "C" {
TObject* createTObjPurpleIceGearL(TObject*, SetDesc*);
}

struct GearMesh {
	u8 _00[0x14];
	Vec3f mCentre; // _14
	f32 mRadius;   // _20
};

struct GearNode {
	u8 _00[4];
	GearMesh* mMesh; // _04
};

#endif
