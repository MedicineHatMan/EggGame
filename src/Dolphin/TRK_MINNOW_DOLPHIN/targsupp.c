#include "types.h"

#pragma function_align 16

asm void TRKAccessFile(void)
{
#ifdef __MWERKS__ // clang-format off
	nofralloc
	twi 31, r0, 0
	blr
#endif // clang-format on
}

#pragma function_align 4

asm void TRKOpenFile(void)
{
#ifdef __MWERKS__ // clang-format off
	nofralloc
	twi 31, r0, 0
	blr
#endif // clang-format on
}

asm void TRKCloseFile(void)
{
#ifdef __MWERKS__ // clang-format off
	nofralloc
	twi 31, r0, 0
	blr
#endif // clang-format on
}

asm void TRKPositionFile(void)
{
#ifdef __MWERKS__ // clang-format off
	nofralloc
	twi 31, r0, 0
	blr
#endif // clang-format on
}
