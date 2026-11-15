#ifndef _OBJECT_TOBJDIVINEVOICE_H_
#define _OBJECT_TOBJDIVINEVOICE_H_
#include "camera/TCamera.h"
struct DrawTask;
struct NJS_TEXLIST;
#include "engine/Fx.h"
#include "engine/MotionPlayer.h"
#include "object/ObjAction.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
#include "ui/TSimpleText.h"
struct TObjDivineVoice;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjDivineVoice(TObject*, SetDesc*);
}

// Vtable 0x8025E1A0 Both draw states set their own ambient colour
class TObjDivineVoice : public TSetActor {
public:
	TObjDivineVoice(TObject* parent, SetDesc* desc);
	virtual ~TObjDivineVoice();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C the update
	virtual void vfn_10(); // _10 empty
	virtual void render(); // _1C
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void* arg);
	virtual void reset(); // _74

	void wait();
	void talk();
	void close();

	void* mModelData;     // _64
	void* mModelData2;    // _68
	NJS_TEXLIST* mTexList;       // _6C
	ObjModel mModel;      // _70
	ObjModel mModel2;     // _90
	MotionPlayer mMotion; // _B0
	ObjDrawState mState;  // _EC
	ObjDrawState mState2; // _110
	DrawTask* mDrawTask;      // _134
	u16 mMsgId;           // _138 set int 0
	u8 mShapeKind;        // _13A set int 1 0 sphere 1 box 2 cylinder
	int mMsgOnly;         // _13C set int 2 no camera model or sound
	ObjRef mRef;          // _140 who walked in
	Vec3f mAt;            // _14C where they stood
	int mAtRotY;          // _158
	f32 mMtx[12];         // _15C the model's place
	TCamPath* mCamera;    // _18C
	Fx* mFx;              // _190
	TSimpleText* mText;   // _194
	int mSound;           // _198 -1 for none
	int mMode;            // _19C
};

extern "C" {
extern const char* gTObjDivineVoiceName;
}

#endif
