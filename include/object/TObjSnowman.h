#ifndef _OBJECT_TOBJSNOWMAN_H_
#define _OBJECT_TOBJSNOWMAN_H_
struct TArc;
struct TObjPurpleBlizzard;
#include "Ninja/ninja.h"
struct DrawTask;
#include "engine/dlist.h"
#include "object/ObjAction.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct CharaColli;
struct SetLink;
struct TCamPath;
struct SetDesc;
struct TObjSnowman;
struct TObject;

#include "core/types.h"

extern "C" {
int feed__15TObjSnowmanHeadFv(void*);
void initSnowman();
void termSnowman();
}

extern "C" {
TObject* createTObjSnowman(TObject*, SetDesc*);
}

extern "C" {
TObject* createTObjSnowmanHead(TObject*, SetDesc*);
}

class TObjSnowman : public TObjAction {
public:
	TObjSnowman(TObject* parent, const SetDesc* desc);
	virtual ~TObjSnowman();
	virtual void draw();
	virtual void render();
	virtual Vec3f* getPos();
	virtual void getNodeMatrix(int node, f32* mtx);
	virtual int getKind();
	virtual void initFromDesc(SetDesc* desc);
	virtual SetLink* getLink(); // _6C

	void runState(int state, int phase);
	void setState(int state);
	void stateEmpty(int phase);
	void stateEvent(int phase);
	void stateComplete(int phase);

	s8 mState;           // _30
	u8 _31;              // _31
	u16 mTimer;          // _32
	u16 mHolder;         // _34
	u8 _36[2];
	int mRotY;           // _38
	f32 mHeight;         // _3C
	Vec3f mPos;          // _40
	CharaColli* mColli;        // _4C
	SetLinkMember mLink; // _50
	DrawTask* mDrawTask;     // _5C
	TCamPath* mEvent;    // _60
};

extern "C" {
extern TArc* sSnowmanArc;
extern NJS_OBJECT** sSnowmanModels;
extern NJS_TEXLIST** sSnowmanTexLists;
extern void** sSnowmanCameras;
extern TArc* gCageArc;
extern TObjPurpleBlizzard* gBlizzard;
extern const char* gTObjSnowmanHeadName;
}

#endif
