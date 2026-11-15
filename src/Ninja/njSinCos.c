#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"

// 0x8020CB54
asm f32 njSin(s32 angle)
{
	nofralloc
	lis r6, njSinTable@ha
	clrlslwi r7, r3, 16, 3
	addi r6, r6, njSinTable@l
	lwz r6, 0x0(r6)
	add r6, r6, r7
	lfs f1, 0x0(r6)
	blr
}

// 0x8020CB70
asm f32 njCos(s32 angle)
{
	nofralloc
	lis r6, njSinTable@ha
	clrlslwi r7, r3, 16, 3
	addi r6, r6, njSinTable@l
	lwz r6, 0x0(r6)
	add r6, r6, r7
	lfs f1, 0x4(r6)
	blr
}

// 0x8020CB8C
asm void njSinCos(s32 angle, f32* sinValue, f32* cosValue)
{
	nofralloc
	lis r6, njSinTable@ha
	clrlslwi r7, r3, 16, 3
	addi r6, r6, njSinTable@l
	lwz r6, 0x0(r6)
	add r6, r6, r7
	lfs f0, 0x0(r6)
	lfs f1, 0x4(r6)
	stfs f0, 0x0(r4)
	stfs f1, 0x0(r5)
	blr
}
