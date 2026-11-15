#ifndef MODE_BATTLEFLAGS_H
#define MODE_BATTLEFLAGS_H

#include "types.h"

struct BattleFlags {
	BattleFlags();     // 0x8002FACC
	void set(int bit); // 0x8002FAD8
	void clear();      // 0x8002FAF0

	u32 mBits; // _00
};

#endif
