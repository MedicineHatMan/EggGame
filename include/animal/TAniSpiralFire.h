#ifndef _TANISPIRALFIRE_H
#define _TANISPIRALFIRE_H

#include "animal/TAniKijiSai.h"
struct DrawTask;
struct NJS_TEXLIST;
#include "egg/TEggCreatureTypeE.h"

/**
 * @size 0x98
 */
class TAniSpiralFire : public TObject {
public:
	TAniSpiralFire(TEggCreatureTypeE* owner, const Vec3f* pos, u16 ownerId);
	virtual ~TAniSpiralFire();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void setPos(const Vec3f* pos);
	virtual int vfn_24();
	virtual Vec3f* getPos();

	CharaColli* mColli; // _20
	Vec3f mPos;         // _24
	f32 mRadius;        // _30 the outside of the ring
	f32 mMid;           // _34
	f32 mInner;         // _38
	f32 _3C;            // _3C
	f32 mFrame;         // _40
	int _44;            // _44
	int _48;            // _48
	int _4C;            // _4C
	f32 mGrowth;        // _50
	u32 mColor;         // _54
	int _58;            // _58
	int _5C;            // _5C
	u8 _60[0x30];
	DrawTask* mDrawTask;   // _90
	NJS_TEXLIST* mTexList; // _94
};

typedef char TAniSpiralFire_size_check[(sizeof(TAniSpiralFire) == 0x98) ? 1 : -1];

#endif
