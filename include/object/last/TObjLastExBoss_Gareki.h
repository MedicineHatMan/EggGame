#ifndef _TOBJLASTEXBOSS_GAREKI_H
#define _TOBJLASTEXBOSS_GAREKI_H

#include "stage/Stage.h"
struct DrawTask;
#include "stage/SetManager.h"
#include "Ninja/ninja.h"

extern "C" {
void objHeapFree(void*, void*);
extern void* gObjHeap;
unsigned int rand();
}

static inline f32 randUnit()
{
	return 3.0517578125e-05f * (f32)(int)rand();
}
static inline f32 randSpan(f32 span, f32 base)
{
	return span * randUnit() + base;
}

/**
 * A rubble piece: a model with a spin and an alpha.
 *
 * @size 0x80
 */
class TObjLastExBoss_GarekiBase : public TObject {
public:
	TObjLastExBoss_GarekiBase(TObject* parent, const Vec3f* pos, int modelId); // 0x8017B4F8
	virtual ~TObjLastExBoss_GarekiBase();                                      // 0x8017B5DC

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	SetTransform mXform; // _20
	int mSpinX;          // _64
	int mSpinY;          // _68
	int _6C;             // _6C
	int mAlpha;          // _70
	int mRadius;         // _74
	NJS_OBJECT* mModel;  // _78
	DrawTask* mDraw;     // _7C
};

class TObjLastExBoss_Gareki : public TObjLastExBoss_GarekiBase {
public:
	TObjLastExBoss_Gareki(TObject* parent, const Vec3f* pos, int modelId);
	virtual ~TObjLastExBoss_Gareki();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();

	f32 mVelY; // _80
	f32 mTop;  // _84 where the rise starts to quicken
};

#endif
