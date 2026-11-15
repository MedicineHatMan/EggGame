#ifndef _OBJECT_YELLOW_TOBJYELLOWGATESET_H_
#define _OBJECT_YELLOW_TOBJYELLOWGATESET_H_
#include "Ninja/ninja.h"
struct DrawTask;
#include "Ninja/njtypes.h"
#include "camera/EventCamRef.h"
#include "engine/ModelDraw.h"
#include "object/ObjAction.h"
#include "sound/SoundHandle.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct CharaColli;
struct Fx;
struct ObjMsg;
struct SetLink;
struct TCamPath;
struct TObjWidgetGate;
struct TObjYellowGateSet;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjYellowGateSet(TObject*, SetDesc*);
}

/**
 * One piece of the gate.
 *
 * @size 0xD4
 */
class TObjWidgetGate : public TObjAction {
public:
	TObjWidgetGate(TObject* parent, int index, NJS_OBJECT* model, NJS_TEXLIST* texList, int effect);
	virtual ~TObjWidgetGate();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();

	void setPlace(const Vec3f* pos, int rotY);
	void startEffect();
	void stopEffect();

	int mIndex;             // _30
	NJS_OBJECT* mModelData; // _34
	NJS_TEXLIST* mTexList;  // _38
	DrawTask* mDrawTask;    // _3C
	ModelDraw mModel;       // _40
	Vec3f mPos;             // _5C
	int mRotY;              // _68
	f32 mMatrix[12];        // _6C
	int mEffectId;          // _9C
	Fx* mEffect;            // _A0
	SoundHandle mSound;     // _A4
	int _D0;                // _D0
};

/**
 * @size 0xE8
 */
class TObjYellowGateSet : public TObjAction {
public:
	TObjYellowGateSet(TObject* parent, const SetDesc* desc);
	virtual ~TObjYellowGateSet();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual Vec3f* getPos();
	virtual int getRotY();
	virtual int getKind();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual int sendToHandle(ObjMsg* msg);
	virtual void setPos(const Vec3f* pos);
	virtual SetLink* getLink();

	void createParts(int opened);
	void updateMatrix();
	int onCameraMessage(ObjMsg* msg);
	void stateWait();
	void stateCamera();
	void stateEvent();
	void stateFinish();
	void stateDone();
	void startEvent();
	void updateEvent();
	void endEvent();

	NJS_OBJECT* mModelData; // _30
	NJS_TEXLIST* mTexList;  // _34
	DrawTask* mDrawTask;    // _38
	SetLinkMember mLink;    // _3C
	CharaColli* mShape;     // _48
	Vec3f mPos;             // _4C
	SetRot mRot;            // _58
	f32 mMatrix[12];        // _64
	FlagByte mFlagByte;     // _94
	u8 _95[3];
	TObjWidgetGate* mParts[7]; // _98 the six pedestals and then the centre
	TCamPath* mEvent;          // _B4
	u16 mSwitchId;             // _B8
	int mEventBefore;          // _BC ":EVID ANTE"
	int mEventAfter;           // _C0 ":EVID POST"
	int mEventFailed;          // _C4 ":EVID FAILED"
	EventCamRef mCam;          // _C8
	int mSound;                // _D0
	ObjRef mToucher;           // _D4
	int mAct;                  // _E0 0 = wait 1 = camera 2 = event 3 = finish 4 = done
	int mStep;                 // _E4
};

extern "C" {
extern const char* gTObjWidgetGateName;
extern const char* gTObjYellowGateSetName;
extern const Vec3f sGateSetOffsets[7];
}

#endif
