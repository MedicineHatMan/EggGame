#ifndef _MODE_TMODEBATTLEMENU_H
#define _MODE_TMODEBATTLEMENU_H
#include "core/TArray.h"
#include "stage/Stage.h"
struct TModeBattleMenu;

#include "core/types.h"

class TModeBattleMenu : public TMode {
public:
	TModeBattleMenu();
	static TModeBattleMenu* create(TModeAllocator* allocator);

	virtual ~TModeBattleMenu() { }
	virtual int init();
	virtual void update();

	u8 mPage;                    // _0C 0 the rule 1 the stage
	u8 mRule;                    // _0D
	u8 mStage;                   // _0E
	TArray<const char*> mStages; // _10
};

#endif
