#ifndef _OBJECT_TOBJEGGSUIT_H_
#define _OBJECT_TOBJEGGSUIT_H_
struct TArc;
#include "object/ObjAction.h"
struct NJS_OBJECT;
struct NJS_TEXLIST;
#include "object/ObjCommon.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct CharaColli;
struct DrawTask;
struct EggSuitEvent;
struct Fx;
struct SetLink;
struct TCamPath;
struct TSimpleText;
struct SetDesc;
struct TObjEggSuit;
struct TObject;

#include "core/types.h"

extern "C" {
void initTObjEggSuit();
void termTObjEggSuit();
}

extern "C" {
TObject* createTObjEggSuit(TObject*, SetDesc*);
}

/** egg_suit.arc's "event" export what the cutscene plays */
struct EggSuitEvent {
	void* mCamObject;       // _00
	void* mCamMotion;       // _04
	int mPlayerMotions[5];  // _08
	void* mSuitMotion;      // _1C the ball's flight up to Billy
	void* mLightMotions[3]; // _20 one for each TLightEffect
};

class TObjEggSuit : public TObjAction {
public:
	enum {
		// mMode
		MODE_WAIT    = 0, // until the player touches it
		MODE_TOUCHED = 1, // until the event starts
		MODE_DEMO    = 2, // the cutscene
		MODE_END     = 3, // until the event script is free
		// what runMode does with a mode
		PHASE_ENTER  = 0,
		PHASE_LEAVE  = 1,
		PHASE_UPDATE = 2,
		PHASE_LATE   = 3,
	};

	TObjEggSuit(TObject* parent, const SetDesc* desc);
	virtual ~TObjEggSuit();

	virtual void draw();                      // _0C
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual Vec3f* getPos();                  // _20
	virtual int getKind();                    // _2C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void* arg);       // _34
	virtual f32 getRadiusV();                 // _54
	virtual SetLink* getLink();               // _6C

	void runMode(int mode, int phase);
	void setMode(int mode);
	void modeWait(int phase);
	void modeTouched(int phase);
	void modeDemo(int phase);
	void modeEnd(int phase);

	s8 mMode;              // _30
	u8 _31;                // _31
	u16 mEventId;          // _32 the event script it asks for at the end
	u16 mFrame;            // _34 of the cutscene
	u8 _36[2];             // _36
	Vec3f mPos;            // _38
	f32 mMtx[12];          // _44
	int mRotY;             // _74
	CharaColli* mColli;    // _78
	DrawTask* mDraw;       // _7C
	SetLinkMember mLink;   // _80
	ObjShadow mShadow;     // _8C
	Fx* mEffect;         // _C4 the sparkle while it waits
	TCamPath* mCam;        // _C8
	EggSuitEvent* mEvent;  // _CC
	TSimpleText* mText;    // _D0 the message after the cutscene
};

extern "C" {
extern TArc* gEggSuitArc;
extern NJS_OBJECT** gEggSuitModels;
extern NJS_TEXLIST** gEggSuitTexlists;
}

#endif
