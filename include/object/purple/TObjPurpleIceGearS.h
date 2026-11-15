#ifndef _TOBJPURPLEICEGEARS_H
#define _TOBJPURPLEICEGEARS_H
struct SetDesc;
struct TObject;

#include "stage/SetActor.h"
#include "object/ObjModel.h"
#include "sound/SoundHandle.h"
#include "engine/DrawTask.h"

extern "C" void objHeapFree(void*, void*);

/**
 * A small ice gear. Set int 0 is the spin step, set int 2 the event it calls when it breaks, set int 3 the switch
 * that removes it, and set float 0 is added to a scale of 1. A hit stops the spin, and after a short wait the
 * switch is set and the gear deletes itself.
 *
 * @size 0x144
 */
class TObjPurpleIceGearS : public TSetActor {
public:
	TObjPurpleIceGearS(TObject* parent, SetDesc* desc);
	virtual ~TObjPurpleIceGearS();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual void vfn_74();

	void makeShape();
	void placeShape();
	void freeShape();
	void spin();
	void shatter();
	void finish();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	void* mGeo;          // _64 MODEL_ICE_GEAR_S
	void* mTex;          // _68 TLS_MODEL_ICE_GEAR_S
	DrawTask* mDrawTask; // _6C
	ObjModel mModel;     // _70
	f32 mMatrix[12];     // _90 placed and scaled
	f32 mSpin[12];       // _C0 mMatrix rolled by mAngle
	f32 mScale;          // _F0 1 plus set float 0
	int mAngle;          // _F4
	int mRotSpeed;       // _F8 set int 0 added to mAngle each frame
	u16 mSwitch;         // _FC set int 3
	u16 mEvent;          // _FE set int 2
	SoundHandle mSound;  // _100
	u16 mTimer;          // _12C frames left after the break
	u16 _12E;            // _12E
	Vec3f mCentre;       // _130
	f32 mRadius;         // _13C the mesh radius times mScale
	int mAct;            // _140 0 spinning 1 breaking 2 waiting out mTimer
};

extern "C" {
TObject* createTObjPurpleIceGearS(TObject*, SetDesc*);
}

#endif
