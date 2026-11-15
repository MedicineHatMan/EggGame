#ifndef _WATERSURFACE_H
#define _WATERSURFACE_H
struct NJS_OBJECT;

#include "stage/SetActor.h"
struct NJS_TEXLIST;

// The moving water drawn by the Green pond waterfall and boss objects

/**
 * Scrolls two texture layers over a model and draws it with an indirect texture. The data comes first so that the
 * vtable lands at _8C.
 *
 * @size 0x90
 */
struct WaterSurface {
	WaterSurface();  // 0x80184A34
	~WaterSurface(); // 0x80184AF0

	void setModel(void* model, void* texList);  // 0x80184B38
	void initFromDesc(SetDesc* desc);           // 0x80184B50
	void debugPrint();                          // 0x80184DB0
	void update();                              // 0x80184F14
	int isVisible();                            // 0x80184F8C

	NJS_OBJECT* mModel;     // _00
	NJS_TEXLIST* mTexList;   // _04
	Vec3f mCentre;    // _08
	f32 mRadius;      // _14
	Vec3f mPos;       // _18
	f32 mMatrix[12];  // _24
	f32 mScroll[4];   // _54 the U and V of the first layer then of the second
	f32 mSpeedU;      // _64
	f32 mSpeedV;      // _68
	u8 mColor[4];     // _6C from the record's rotation and int 1 is the alpha
	u8 mAddColor[4];  // _70
	int mIndExp;      // _74 set int 2 minus 2
	f32 mIndRate;     // _78
	f32 mIndScale;    // _7C
	int mFakeBump;    // _80 1 adds a TEV stage
	int mBlendMode;   // _84 1 minus byte 0x3E of the record
	f32 mFarDist;     // _88

	virtual void beginDraw(); // _08 0x80185058
	virtual void endDraw();   // _0C 0x801850E8
	virtual void render();    // _10 0x80185154
};

/**
 * The water of the Green boss's waterfall and lake, which also sets a white material colour.
 *
 * @size 0x90
 */
struct GreenBossWater : public WaterSurface {
	GreenBossWater() { }
	~GreenBossWater() { }

	virtual void beginDraw(); // _08 0x801934D8
	virtual void endDraw();   // _0C 0x8019355C
};

#endif
