#ifndef _STAFFROLL_H
#define _STAFFROLL_H

#include "stage/Stage.h"

void staffRollLoad();   // 0x801C6FAC loads both files
void staffRollFree();   // 0x801C7044
void staffRollCreate(); // 0x801C709C starts the roll under gTL_02

/**
 * The game mode that plays the ending movie with the staff roll over it. k_staff_roll.prd.
 *
 * @size 0xC
 */
class TModeStaffRoll : public TMode {
public:
	TModeStaffRoll();
	static TModeStaffRoll* create(TModeAllocator* allocator);

	virtual ~TModeStaffRoll();
	virtual void loadFiles();
	virtual void freeFiles();
	virtual int init();
	virtual int shutdown();
	virtual void update();
};

#endif
