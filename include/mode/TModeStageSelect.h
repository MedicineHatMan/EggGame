#ifndef _TMODESTAGESELECT_H
#define _TMODESTAGESELECT_H

#include "core/TArray.h"
#include "stage/Stage.h"

/**
 * @size 0x20
 */
class TModeStageSelect : public TMode {
public:
	TModeStageSelect();
	static TModeStageSelect* create(TModeAllocator* allocator);

	virtual ~TModeStageSelect();
	virtual int init();
	virtual void update();

	const char* getStageName();

	u8 mCursor;        // _0C
	int mPlayer;       // _10 0 to 4
	TArray<const char*> mStages;  // _14
};

#endif
