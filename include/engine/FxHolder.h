#ifndef _FXHOLDER_H
#define _FXHOLDER_H

#include "core/types.h"
#include "stage/Stage.h"
#include "stage/SetManager.h"

struct FxVec : public SetRot {
	inline FxVec();
};

/** A matrix that starts as the identity where an effect sits */
struct FxMtx { // size 0x30
	FxMtx();   // 0x8018938C

	void setPos(const Vec3f* pos);                                                     // 0x801893BC
	void setPosRot(const Vec3f* pos, int rotX, int rotY);                              // 0x801893DC
	void setPosRotScale(const Vec3f* pos, int rotX, int rotY, f32 sx, f32 sy, f32 sz); // 0x80189430

	f32 m[3][4]; // _00
};

/** An effect an object keeps alive such as a glow or a trail */
struct FxHolder { // size 0x34
	FxHolder();   // 0x801894B8
	~FxHolder();  // 0x801894F0

	void start(int bank, int id);                                       // 0x80189578 does nothing while one runs
	void start(int bank, int id, const Vec3f* pos);                     // 0x801895BC
	void start(int bank, int id, const Vec3f* pos, int rotX, int rotY); // 0x80189604

	FxMtx mMtx;    // _00
	void* mEffect; // _30 the running effect or 0
};

/** An effect that follows an owner fxSetOwner */
struct FxAttach { // size 0x34
	FxAttach();   // 0x8018965C
	~FxAttach();  // 0x80189694

	void stop();                  // 0x801896DC
	void start(int bank, int id); // 0x80189710

	FxMtx mMtx;  // _00
	int mActive; // _30 set while an effect follows mMtx
};

extern "C" void fxHolderStop(FxHolder* holder); // 0x80189538

#endif
