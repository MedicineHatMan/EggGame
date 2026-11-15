#ifndef _LIGHT_H
#define _LIGHT_H

#include "core/types.h"
#include "engine/dlist.h"
#include "Dolphin/GX/GXTypes.h"

struct LightBase;
struct LightList;

/** A light's place in a group. @size 0x10 */
struct LightLink {
	LightLink()
	{
		mNext = NULL;
		mPrev = NULL;
		mList = NULL;
	}

	LightLink* mNext;  // _00
	LightLink* mPrev;  // _04
	LightBase* mLight; // _08
	LightList* mList;  // _0C NULL when it isn't in one
};

/** An intrusive list of light links, 0x80043638-0x800437D8. @size 0xC */
struct LightList {
	LightList();  // 0x80043638
	~LightList(); // 0x8004364C

	void append(LightLink* link);                        // 0x80043694
	void clear();                                        // 0x800436D4
	void remove(LightLink* link);                        // 0x80043710
	void insertBefore(LightLink* link, LightLink* next); // 0x80043770
	void unlinkAll();                                    // 0x800437AC leaves the counts alone

	LightLink* mLast;  // _00
	LightLink* mFirst; // _04
	int mCount;        // _08
};

/** Base of the lights */
struct LightBase {
	LightBase(); // 0x80042D60

	u32 mFlags; // _00

	// _04 = vtable
	inline virtual ~LightBase() { }                                // _08
	virtual void init(GXLightObj* gxLight, const f32* matrix) = 0; // _0C

	u8 mPriority;        // _08 lower comes first in a group
	u8 mEnabled;         // _09
	u32 mGxId;           // _0C
	LightLink mLink;     // _10
	GXLightObj mGxLight; // _20
};

struct AmbientLight : public LightBase { // size 0x64
	AmbientLight();
	inline virtual ~AmbientLight() { }
	virtual void init(GXLightObj* gxLight, const f32* matrix);

	u32 mColor; // _60
};

struct DirLight : public LightBase { // size 0x70
	DirLight();
	inline virtual ~DirLight() { }
	virtual void init(GXLightObj* gxLight, const f32* matrix);

	u32 mColor; // _60
	Vec3f mDir; // _64
};

struct PointLight : public LightBase { // size 0x7C
	PointLight();                      // 0x800433A8
	inline virtual ~PointLight() { }
	virtual void init(GXLightObj* gxLight, const f32* matrix);

	u32 mColor; // _60
	Vec3f mPos; // _64
	f32 mKa[3]; // _70
};

struct SpotLight : public LightBase { // size 0x94
	SpotLight();
	inline virtual ~SpotLight(); // 0x800435DC defined after lightGxId in Light.cpp
	virtual void init(GXLightObj* gxLight, const f32* matrix);

	u32 mColor; // _60
	Vec3f mPos; // _64
	Vec3f mDir; // _70
	f32 mA[3];  // _7C
	f32 mK[3];  // _88
};

/** The lights to give to a group, with nodes from a pool of 16; ~dlist frees the pool. @size 0x14 */
struct LightSet {
	LightSet(); // 0x80042D98

	void add(LightBase* light);    // 0x80042DCC
	void remove(LightBase* light); // 0x80042DF4

	dlist<LightBase*> mLights; // _00
};

/** The lights that are on, in priority order, and the channel masks lightGroupLoad gathers. @size 0x4C */
struct LightGroup {
	LightGroup();  // 0x80042E58
	~LightGroup(); // 0x80042E98

	void add(LightBase* light);    // 0x80042EE4 by priority
	void remove(LightBase* light); // 0x80042F7C
	void addAll(LightSet* set);    // 0x80042FAC
	void removeAll(LightSet* set); // 0x80042FF4
	void set(LightSet* set);       // 0x8004303C
	void clear();                  // 0x80043078

	LightList mList; // _00
	u32 mDiffuse;    // _0C
	u32 mSpecular;   // _10
	u32 mAlpha;      // _14
	s32 mAlphaSpec;  // _18
	f32 mMatrix[12]; // _1C
};

extern "C" void lightGroupLoad(LightGroup* group);

// Add or remove a light on a player's light set by the player's object id
extern "C" void playerAddLight(int id, LightBase* light);
extern "C" void playerRemoveLight(int id, LightBase* light);

struct Vec3f;

extern "C" {
u32 lightGxId(int);
void lightInitDirectional(GXLightObj*, const f32*, const Vec3f*, const u32*);
}

#ifndef LIGHT_CPP
inline SpotLight::~SpotLight()
{
}
#endif

#endif
