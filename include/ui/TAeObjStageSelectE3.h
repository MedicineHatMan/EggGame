#ifndef _TAEOBJSTAGESELECTE3_H
#define _TAEOBJSTAGESELECTE3_H

#include "ui/TAeLayer.h"

/**
 * @size 0x44
 */
class TAeObjStageSelectE3 : public TAeObject {
public:
	TAeObjStageSelectE3(TObject* parent, void* data, void* assets);
	virtual ~TAeObjStageSelectE3();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_10(); // _10

	TAeLayer* mLayer;  // _34 id 0x81
	TAeLayer* mPreview; // _38 id 0x17 the row's preview bound into mLayer
	u32 _3C;           // _3C
	int mStage;        // _40 the row mPreview shows
};

extern "C" {
void initStageSelectE3Set();
void showStageSelectE3(int);
void termStageSelectE3Set();
}

extern "C" {
extern u32 sAeStageSelectE3Name[2];
extern const u32 gAeStageSelectE3Banks[8];
}

#endif
