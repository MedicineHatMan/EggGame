#ifndef _TMSNOBJCAGE_H
#define _TMSNOBJCAGE_H
struct SetDesc;
struct TObject;
struct DrawTask;
struct MeshObject;

#include "stage/SetActor.h"
#include "object/ObjCommon.h"
#include "object/ObjModel.h"
#include "ui/TextVec.h"
#include "camera/EventCamRef.h"
#include "enemy/EnemyLight.h"

class TCamera;

/**
 * A mission cage. The constructor stores the name. It waits on a key, or on a camera watch, then
 * opens and removes itself.
 *
 * Field names are ours. The shadow, the placement and the watch are byte arrays: their constructors
 * run in the body, and the placement has no destructor.
 *
 * @size 0x198
 */
class TMsnObjCage : public TSetActor {
public:
	// 0 waiting 1 counting down the open 2 waiting for the effect to finish
	enum Phase {
		PHASE_WAIT = 0,
		PHASE_OPEN = 1,
		PHASE_DONE = 2
	};

	TMsnObjCage(TObject* parent, SetDesc* desc);
	virtual ~TMsnObjCage();

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual int getKind();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual int sendToHandle(ObjMsg* msg);

	SetLink* getLink();

	inline void* operator new(u32 size);
	inline void operator delete(void* mem);

private:
	// 0x20 bytes at _64 The ObjModel constructor leaves the first word alone

	// 0x5C bytes at _84 No destructor

	void loadModel();
	void fitModel();
	int acceptMessage(ObjMsg* msg);
	void spawnBeam();
	void clearBeam();
	void checkOpen();
	void playOpen();
	void finish();
	void setHidden(int hidden);

	ObjModel mShadow;  // _64
	ObjPlace mPlace;    // _84
	EnemyLight mLight; // _E0
	ObjDrawState mSaved; // _120 copied from getDefaultDrawState
	DrawTask* mTask;     // _144
	MeshObject* mModel;    // _148
	TCamera* mBeam;  // _14C
	int mTimer;      // _150
	EventCamRef mWatch; // _154
	int mCamera;     // _15C 0 or 1
	int mKeyId;      // _160
	int mPhase;      // _164
	f32 mScale;      // _168 1..3
	u8 _16C[0xC];
	int mMirror;     // _178 1 draws into the mirror
	int mCamPos;     // _17C 1 uses the set offset as the aim
	Vec3f mAim;      // _180
	Vec3f mMark;     // _18C where the beam starts
};

extern "C" {
TObject* createTMsnObjCage(TObject*, SetDesc*);
}

extern "C" {
extern InitVec sCageExtent;
}

#endif
