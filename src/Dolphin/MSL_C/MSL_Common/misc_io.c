#include "PowerPC_EABI_Support/MSL_C/MSL_Common/ansi_files.h"
#include "PowerPC_EABI_Support/MSL_C/PPC_EABI/abort_exit.h"

/**
 * @note Address: N/A
 * @note Size: 0x10
 */
void clearerr(void)
{
	// UNUSED FUNCTION
}

/**
 * @note Address: N/A
 * @note Size: 0x8
 */
void feof(void)
{
	// UNUSED FUNCTION
}

/**
 * @note Address: N/A
 * @note Size: 0x8
 */
void ferror(void)
{
	// UNUSED FUNCTION
}

/**
 * @note Address: N/A
 * @note Size: 0x74
 */
void perror(void)
{
	// UNUSED FUNCTION
}

/**
 * @note Address: 0x800C7428
 * @note Size: 0x10
 */
void __stdio_atexit(void)
{
	__stdio_exit = __close_all;
}
