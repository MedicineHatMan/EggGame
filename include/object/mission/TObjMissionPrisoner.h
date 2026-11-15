#ifndef _OBJECT_MISSION_TOBJMISSIONPRISONER_H_
#define _OBJECT_MISSION_TOBJMISSIONPRISONER_H_
struct TArc;
#include "Ninja/ninja.h"
struct DrawTask;
#include "engine/MotionPlayer.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct CharaColli;
struct ObjMsg;
struct SetLink;
struct TCamConversation;
class TPlyComb;
struct TCamPath;
struct SetDesc;
struct TObjMissionPrisoner;
struct TObject;

#include "core/types.h"

extern "C" {
void initMissionPrisoner();
void termMissionPrisoner();
}

extern "C" {
int loadTObjMissionPrisoner(u8*);
TObject* createTObjMissionPrisoner(TObject*, SetDesc*);
}

class TObjMissionPrisoner : public TObjAction {
public:
	TObjMissionPrisoner(TObject* parent, const SetDesc* desc);
	virtual ~TObjMissionPrisoner();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual Vec3f* getPos();
	virtual int getKind();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual int sendToHandle(ObjMsg* msg);
	virtual SetLink* getLink(); // _6C

	void loadModels();
	void pose();
	void updateMotion();
	void setState(int state);
	void runState(int state, int phase);
	void stateCaptive(int phase);
	void stateEvent(int phase);
	void stateFreed(int phase);
	void stateWait(int phase);
	void stateTalk(int phase);
	void stateHit(int phase);
	void stateTurn(int phase);
	int onTalk(ObjMsg* msg);
	int onGetFlags();
	int onMeet(ObjMsg* msg);
	int onFree();

	static int startEvent(u32 id);

	u8 mKind;             // _30 one of four people
	s8 mState;            // _31
	FlagByte mFlagBits;         // _32
	u16 mEventId;         // _34
	u16 mTimer;           // _36
	u16 mMsgId;           // _38
	int  mLightSet;      // _3C
	TPlyComb* mActor;     // _40 draws the body
	CharaColli* mColli;         // _44
	CharaColli* mSense;         // _48
	DrawTask* mDrawTask;      // _4C
	SetLinkMember mLink;  // _50
	MotionPlayer mMotion; // _5C
	f32 mMatrix[12];      // _98
	f32 mBondsMatrix[12]; // _C8
	f32 mFaceMatrix[12];  // _F8
	f32 mLeftMatrix[12];  // _128
	f32 mRightMatrix[12]; // _158
	f32 mBodyMatrix[12];  // _188
	f32 mEventMatrix[12]; // _1B8
	Vec3f mPos;           // _1E8
	int mRotY;            // _1F4
	ObjShadow mShadow;    // _1F8
	void** mModels;       // _230
	void** mTexLists;     // _234
	GjActor* mBody;          // _238
	GjActor* mLeft;          // _23C
	GjActor* mRight;         // _240
	GjActor* mFace;          // _244
	int mTargetRotY;      // _248
	ObjRef mRescuer;      // _24C
	TCamConversation* mCamera; // _258
	TCamPath* mEvent;     // _25C
	int* mWindow;         // _260 the message window
};

extern "C" {
extern const char* gTObjMissionPrisonerName;
extern TArc* sMissionPrisonerArc;
extern NJS_OBJECT** sPrisonerModels;
extern NJS_MOTION** sPrisonerMotionData;
extern NJS_TEXLIST** sPrisonerTexLists;
}

#endif
