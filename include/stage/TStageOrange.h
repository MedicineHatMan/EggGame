#ifndef _STAGE_TSTAGEORANGE_H_
#define _STAGE_TSTAGEORANGE_H_
#include "Ninja/ninja.h"
#include "stage/Stage.h"
struct TStageOrange;

#include "core/types.h"

class TStage;
struct Vec3f;

extern "C" {
TStage* createOrangeStage();
void mirrorBeginReflect();
int mirrorCopyPending();
void mirrorCopyTex();
void mirrorEndReflect();
int mirrorIsReflecting();
void mirrorRequestCopy();
void mirrorSetPlane(const Vec3f*, const Vec3f*);
void mirrorSetTexture();
void mirrorTexInit();
void mirrorTexRelease();
}

class TStageOrange : public TStage {
public:
	TStageOrange();
	virtual ~TStageOrange();
	virtual void loadAssets();
	virtual void freeAssets();
	virtual void vfn_14();
	virtual void run(u32 flags);
};

extern "C" {
extern u32 sStageOrangeClearColor;
}

#endif
