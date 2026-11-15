#ifndef _ANIRIDER_H
#define _ANIRIDER_H

#include "core/types.h"
#include "engine/WorldMesh.h"

class MeshColli;
struct AniRider;

// Rider's constructor and destructor run by hand on memory laid out like a Rider
inline void riderInit(AniRider* rider)
{
	((Rider*)rider)->__ct();
}

inline void riderDtor(AniRider* rider, s16)
{
	((Rider*)rider)->~Rider();
}

/**
 * What stands on a collision mesh: a rider (see WorldMesh.cpp), and the triangle and point it touched.
 *
 * @size 0x24
 */
struct AniRider {
	AniRider() { riderInit(this); }        // 0x801C1228
	~AniRider() { riderDtor(this, -1); } // 0x801C11DC

	AniRider* mNext;  // _00
	AniRider* mPrev;  // _04
	AniRider* mSelf;  // _08
	void* mList;      // _0C
	MeshColli* mMesh; // _10 the mesh it stands on or null
	u16 mTri;         // _14 the triangle it touched
	u8 _16[2];
	Vec3f mPos;       // _18 where it touched in the mesh's space
};

#endif
