#ifndef _OBJECT_TOBJSETLIGHTCOLLISION_H_
#define _OBJECT_TOBJSETLIGHTCOLLISION_H_
#include "Ninja/ninja.h"
#include "engine/CharaColli.h"
#include "engine/LightZone.h"
#include "object/ObjAction.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct ObjLightZone;
struct TObjSetLightCollision;
struct TObject;

#include "core/types.h"

class SetDesc;

extern "C" {
void lightCollisionExit();
void lightCollisionInit();
void prepareTObjSetLightCollision(SetDesc*);
}

extern "C" {
TObject* createTObjSetLightCollision(TObject*, SetDesc*);
}

/** A light zone that belongs to a set object */
struct ObjLightZone : public LightZone {              // size 0x10
	ObjLightZone();                                   // 0x800A21E8
	virtual int test(u16 id, int kind, f32* dist);    // _08 0x800A228C
	virtual int getSun(u16 id, LightZoneSun* sun);    // _0C 0x800A22C0
	virtual int getSpot(u16 id, LightZoneSpot* spot); // _10 0x800A22F4
	virtual ~ObjLightZone();                          // 0x800A222C

	void setOwner(TObject* owner); // 0x800A2284

	TObject* mOwner; // _0C
};

class TObjSetLightCollision : public TObjAction {
public:
	TObjSetLightCollision(TObject* parent, const SetDesc* desc);
	~TObjSetLightCollision();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C
	virtual Vec3f* getPos();                  // _20
	virtual int getKind();                    // _2C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void* arg);       // _34

	void updateLight();
	LightParam* getParam(int type);
	int test(u16 id, int kind, f32* dist);
	int getSun(u16 id, LightZoneSun* sun);
	int getSpot(u16 id, LightZoneSpot* spot);

	SetLinkMember mLink; // _30
	CharaColli* mShape;  // _3C
	ObjLightZone mNode;  // _40
	int mParam;          // _50 set int 0
	Vec3f mPos;          // _54
	int mRotY;           // _60
	int mRotX;           // _64
	int mRotZ;           // _68
	f32 mTune[3];        // _6C set ints 1..3 stored in tenths
	int mKind;           // _78 set float 0 kept in 0..2
	f32 mExtent[3];      // _7C set floats 1..3 each at least 2
	int mRecord;         // _88 the record halfword at +0x3E 0 off 1 on 2 on at night
	Vec3f mBase;         // _8C the light's place before lightFarPoint
	Vec3f mLightPos;     // _98 mPos moved by mTune
	Vec3f mDir;          // _A4 the param's direction turned by mRotY and mRotX
	u32 mColor;          // _B0
	f32 mSpotA[3];       // _B4
	f32 mDistK[3];       // _C0
};

extern "C" {
extern const char* gTObjSetLightCollisionName;
}

#endif
