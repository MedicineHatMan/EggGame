#include "mode/BattleFlags.h"

// 0x8002FACC
BattleFlags::BattleFlags()
{
	mBits = 0;
}

// 0x8002FAD8
void BattleFlags::set(int bit)
{
	mBits |= 1 << bit;
}

// 0x8002FAF0
void BattleFlags::clear()
{
	mBits = 0;
}
