#include "types.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/console_io.h"
#include "PowerPC_EABI_Support/MetroTRK/trk.h"
#include "Dolphin/os.h"

static BOOL initialized;

/**
 * @note Address: 0x800CC2E0
 * @note Size: 0xD0
 */
WEAKFUNC int __write_console(__file_handle file, char* buff, size_t* count, __ref_con ref_con)
{

	if ((OSGetConsoleType() & 0x20000000) == 0) {
		int r3_cond = 0;
		if (initialized == FALSE) {
			r3_cond = InitializeUART(0xE100);
			;
			if (r3_cond == 0) {
				initialized = TRUE;
			}
		}
		if (r3_cond != 0) {
			return TRUE;
		}
		if (WriteUARTN(buff, *count) != 0) {
			*count = 0;
			return TRUE;
		}
	}
	__TRK_write_console(file, (u8*)buff, count, ref_con);
	return FALSE;
}

/**
 * @note Address: N/A
 * @note Size: 0x48
 */
void __init_uart_console(void)
{
	// UNUSED FUNCTION
}

/**
 * @note Address: N/A
 * @note Size: 0x8
 */
void __delete_file(void)
{
	// UNUSED FUNCTION
}

/**
 * @note Address: N/A
 * @note Size: 0x8
 */
void __rename_file(void)
{
	// UNUSED FUNCTION
}

/**
 * @note Address: N/A
 * @note Size: 0xCC
 */
void __temp_file_name(void)
{
	// UNUSED FUNCTION
}
