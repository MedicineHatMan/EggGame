#ifndef _STAGE_TSTAGEPURPLE_H
#define _STAGE_TSTAGEPURPLE_H

#include "stage/Stage.h"

class TStagePurple : public TStage {
public:
	TStagePurple();
	virtual ~TStagePurple();
	virtual void loadAssets();
	virtual void freeAssets();
	virtual void vfn_14();
};

// Theme "purpleboss" Vtable 0x8025CA70
class TStagePurpleBoss : public TStage {
public:
	TStagePurpleBoss();
	virtual ~TStagePurpleBoss();
	virtual void loadAssets();
	virtual void freeAssets();
	virtual void vfn_14();
};

typedef char TStagePurple_size_check[(sizeof(TStagePurple) == 0x30) ? 1 : -1];
typedef char TStagePurpleBoss_size_check[(sizeof(TStagePurpleBoss) == 0x30) ? 1 : -1];

#endif
