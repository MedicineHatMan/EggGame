#include "types.h"

#ifndef _MSL_WIDE_CHAR
#define _MSL_WIDE_CHAR
#endif

#include "PowerPC_EABI_Support/MSL_C/MSL_Common/critical_regions.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/FILE_POS.h"
#include "errno.h"

#define SEEK_SET (0)
#define SEEK_CUR (1)
#define SEEK_END (2)

/**
 * @note Address: 0x800C6BE0
 * @note Size: 0xE4
 */
// 0x80203B18
int ftell(FILE* stream)
{
	int retval;

	__begin_critical_region(stdin_access);
	retval = (s32)_ftell(stream);
	__end_critical_region(stdin_access);
	return retval;
}

/**
 * @note Address: N/A
 * @note Size: 0xAC
 */
int _ftell(FILE* file)
{
	int charsInUndoBuffer = 0;
	int position;

	u8 tmp_kind = file->mMode.file_kind;
	if (!(tmp_kind == __disk_file || tmp_kind == __console_file) || file->mState.error) {
		errno = EFPOS;
		return -1;
	}

	if (file->mState.io_state == __neutral)
		return (file->mPosition);

	position = file->mBufferPosition + (file->mBufferPtr - file->mBuffer);

	if (file->mState.io_state >= __rereading) {
		charsInUndoBuffer = file->mState.io_state - __rereading + 1;
		position -= charsInUndoBuffer;
	}

	return (position);
}

/**
 * @note Address: 0x800C6970
 * @note Size: 0x270
 */
// 0x802038DC
int _fseek(FILE* file, u32 offset, int whence)
{
	int bufferCode;

	if (file->mMode.file_kind != __disk_file || file->mState.error != 0) {
		errno = EFPOS;
		return -1;
	}

	if (file->mState.io_state == __writing) {
		if (__flush_buffer(file, nullptr) != 0) {
			file->mState.error  = 1;
			file->mBufferLength = 0;
			errno               = EFPOS;
			return -1;
		}
	}

	if (whence == SEEK_CUR) {
		whence = SEEK_SET;
		offset += _ftell(file);
	}

	if ((whence != SEEK_END) && (file->mMode.io_mode != 3)
	    && (file->mState.io_state == __reading || file->mState.io_state == __rereading)) {
		if ((offset >= file->mPosition) || !(offset >= file->mBufferPosition)) {
			file->mState.io_state = __neutral;
		} else {
			file->mBufferPtr      = file->mBuffer + (offset - file->mBufferPosition);
			file->mBufferLength   = file->mPosition - offset;
			file->mState.io_state = __reading;
		}
	} else {
		file->mState.io_state = __neutral;
	}

	if (file->mState.io_state == __neutral) {
		if (file->positionFunc != nullptr && (int)file->positionFunc(file->mHandle, &offset, whence, file->ref_con)) {
			file->mState.error  = 1;
			file->mBufferLength = 0;
			errno               = EFPOS;
			return -1;
		} else {
			file->mState.eof    = 0;
			file->mPosition     = offset;
			file->mBufferLength = 0;
		}
	}

	return 0;
}

/**
 * @note Address: 0x800C6904
 * @note Size: 0x6C
 */
int fseek(FILE* stream, u32 offset, int whence)
{
	int code;
	__begin_critical_region(stdin_access);
	code = _fseek(stream, offset, whence); // 0 if successful -1 if error
	__end_critical_region(stdin_access);
	return code;
}
