#include "Ninja/ninja.h"

f32 modff(f32, f32*);

static f32 sFracWhole; // 0x802C6130

// 0x8020CB30
f32 njFraction(f32 n)
{
	return modff(n, &sFracWhole);
}
