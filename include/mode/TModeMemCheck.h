#ifndef _MODE_TMODEMEMCHECK_H
#define _MODE_TMODEMEMCHECK_H
#include "stage/Stage.h"
struct TModeMemCheck;

#include "core/types.h"

class TModeMemCheck : public TMode {
public:
	TModeMemCheck();
	static TModeMemCheck* create(TModeAllocator* allocator);

	virtual ~TModeMemCheck() { }
	virtual void update();

	int mHideHelp; // _0C
};

#endif
