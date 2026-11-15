#ifndef _ANIMAL_TANISARUKOUMORI_H
#define _ANIMAL_TANISARUKOUMORI_H
#include "Ninja/ninja.h"
struct DrawTask;
#include "egg/EggCreature.h"
#include "object/ObjAction.h"
#include "stage/Stage.h"
struct ElectricVtx;
struct TAniElectric;
struct TAniSaruKoumori;

#include "core/types.h"

struct ElectricVtx {
	f32 x, y, z;
	f32 u, v;
	u32 color;
};

/**
 * Four vertices hung under a position that the owner keeps up to date. Its texture is the owner's.
 *
 * @size 0xE0
 */
class TAniElectric : public TObject {
public:
	TAniElectric(TEggCreatureTypeE* owner);
	virtual ~TAniElectric();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual int vfn_20();
	virtual const Vec3f* vfn_24();

	u32 mTexList;         // _20
	ElectricVtx mVtx[4];  // _24
	const Vec3f* mPosRef; // _84
	Vec3f mGround;        // _88
	f32 mWidth;           // _94
	f32 mHeight;          // _98
	f32 mFrame;           // _9C
	u32 mColor;           // _A0
	DrawTask* mDrawTask;  // _A4
	u8 _A8[4];
	void* mFx;       // _AC
	f32 mMatrix[12]; // _B0
};

/**
 * @size 0x3C0
 */
class TAniSaruKoumori : public TEggCreatureTypeE {
public:
	TAniSaruKoumori(TObject* parent, int player, int kind, const Vec3f* pos);
	virtual ~TAniSaruKoumori();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void render();
	virtual int onHit(const TObjAction* other, int arg, int* result);
	virtual void resetBones();
	virtual void onPhase(int phase);

	void runAttack();
	void runEnd();
	void killElectric();

	Vec3f mStart;            // _384
	f32 mSwing;              // _390
	f32 mSwingSpeed;         // _394
	int mStartRot[3];        // _398
	f32 mSpeed;              // _3A4
	f32 mTravelled;          // _3A8
	f32 _3AC;                // _3AC
	TAniElectric* mElectric; // _3B0
	Vec3f mElectricPos;      // _3B4
};

extern "C" {
extern const char* gTAniElectricName;
extern const char* gTAniSaruKoumoriName;
extern f32 gSaruRate20;
extern f32 gSaruRate30;
extern f32 gSaruRate55;
extern f32 gSaruRate80;
extern f32 gSaruRate40;
extern void* const sSaruKoumoriCallbacks[5];
}

#endif
