#include "io/nt_arc.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/ctype_api.h"
#include "core/SdkCalls.h"

inline int tolower(int c)
{
	return (c == -1 ? -1 : (int)__lower_map[(u8)c]);
}

typedef struct FSTEntry {
	u32 isDirAndStringOff;
	u32 parentOrPosition;
	u32 nextEntryOrLength;
} FSTEntry;

#define entryIsDir(fstStart, i)   (((fstStart)[i].isDirAndStringOff & 0xff000000) ? TRUE : FALSE)
#define stringOff(fstStart, i)    ((fstStart)[i].isDirAndStringOff & ~0xff000000)
#define parentDir(fstStart, i)    ((fstStart)[i].parentOrPosition)
#define nextDir(fstStart, i)      ((fstStart)[i].nextEntryOrLength)
#define filePosition(fstStart, i) ((fstStart)[i].parentOrPosition)
#define fileLength(fstStart, i)   ((fstStart)[i].nextEntryOrLength)

static BOOL ARCConvertEntrynumToPath(ARCHandle* handle, s32 entrynum, char* path, u32 maxlen);

BOOL ARCInitHandle(void* arcStart, ARCHandle* handle)
{
	FSTEntry* FSTEntries;
	ARCHeader* arcHeader = (ARCHeader*)arcStart;

	if (arcHeader->magic != DARCH_MAGIC) {
		OSPanic(__FILE__, 70, "ARCInitHandle: bad archive format");
	}

	handle->archiveStartAddr = arcStart;
	handle->FSTStart = FSTEntries = (FSTEntry*)((u32)arcStart + arcHeader->fstStart);
	handle->fileStart             = (void*)((u32)arcStart + arcHeader->fileStart);
	handle->entryNum              = nextDir(FSTEntries, 0);
	handle->FSTStringStart        = (char*)&(FSTEntries[handle->entryNum]);
	handle->FSTLength             = arcHeader->fstSize;
	handle->currDir               = 0;

	return TRUE;
}

// 0x80153524
BOOL ARCOpen(ARCHandle* handle, const char* fileName, ARCFileInfo* af)
{
	s32 entry;
	char currentDir[128];
	FSTEntry* FSTEntries = (FSTEntry*)handle->FSTStart;

	entry = ARCConvertPathToEntrynum(handle, fileName);

	if (0 > entry) {
		ARCGetCurrentDir(handle, currentDir, 128);
		return FALSE;
	}

	if ((entry < 0) || entryIsDir(FSTEntries, entry)) {
		return FALSE;
	}

	af->handle      = handle;
	af->startOffset = filePosition(FSTEntries, entry);
	af->length      = fileLength(FSTEntries, entry);

	return TRUE;
}

// 0x801535B8
BOOL ARCFastOpen(ARCHandle* handle, s32 entrynum, ARCFileInfo* af)
{
	FSTEntry* FSTEntries = (FSTEntry*)handle->FSTStart;

	if ((entrynum < 0) || (entrynum >= handle->entryNum) || entryIsDir(FSTEntries, entrynum)) {
		return FALSE;
	}

	af->handle      = handle;
	af->startOffset = filePosition(FSTEntries, entrynum);
	af->length      = fileLength(FSTEntries, entrynum);

	return TRUE;
}

static BOOL isSame(const char* path, const char* string)
{
	while (*string != '\0') {
		if (tolower(*path++) != tolower(*string++)) {
			return FALSE;
		}
	}

	if ((*path == '/') || (*path == '\0')) {
		return TRUE;
	}

	return FALSE;
}

// 0x801536AC
s32 ARCConvertPathToEntrynum(ARCHandle* handle, const char* pathPtr)
{
	const char* ptr;
	char* stringPtr;
	BOOL isDir;
	s32 length;
	u32 dirLookAt;
	u32 i;
	FSTEntry* FSTEntries;

	dirLookAt  = handle->currDir;
	FSTEntries = (FSTEntry*)handle->FSTStart;

	while (1) {
		if (*pathPtr == '\0') {
			return (s32)dirLookAt;
		} else if (*pathPtr == '/') {
			dirLookAt = 0;
			pathPtr++;
			continue;
		} else if (*pathPtr == '.') {
			if (*(pathPtr + 1) == '.') {
				if (*(pathPtr + 2) == '/') {
					dirLookAt = parentDir(FSTEntries, dirLookAt);
					pathPtr += 3;
					continue;
				} else if (*(pathPtr + 2) == '\0') {
					return (s32)parentDir(FSTEntries, dirLookAt);
				}
			} else if (*(pathPtr + 1) == '/') {
				pathPtr += 2;
				continue;
			} else if (*(pathPtr + 1) == '\0') {
				return (s32)dirLookAt;
			}
		}

		for (ptr = pathPtr; (*ptr != '\0') && (*ptr != '/'); ptr++) {
			;
		}

		isDir  = (*ptr == '\0') ? FALSE : TRUE;
		length = (s32)(ptr - pathPtr);

		ptr = pathPtr;

		for (i = dirLookAt + 1; i < nextDir(FSTEntries, dirLookAt);
		     i = entryIsDir(FSTEntries, i) ? nextDir(FSTEntries, i) : (i + 1)) {
			if ((entryIsDir(FSTEntries, i) == FALSE) && (isDir == TRUE)) {
				continue;
			}

			stringPtr = handle->FSTStringStart + stringOff(FSTEntries, i);

			if (isSame(ptr, stringPtr) == TRUE) {
				goto next_hier;
			}
		}

		return -1;

	next_hier:
		if (!isDir) {
			return (s32)i;
		}

		dirLookAt = i;
		pathPtr += length + 1;
	}
}

static u32 myStrncpy(char* dest, char* src, u32 maxlen)
{
	u32 i = maxlen;

	while ((i > 0) && (*src != 0)) {
		*dest++ = *src++;
		i--;
	}

	return (maxlen - i);
}

// 0x801DC1F8
static u32 entryToPath(ARCHandle* handle, u32 entry, char* path, u32 maxlen)
{
	char* name;
	u32 loc;
	FSTEntry* FSTEntries = (FSTEntry*)handle->FSTStart;

	if (entry == 0) {
		return 0;
	}

	name = handle->FSTStringStart + stringOff(FSTEntries, entry);

	loc = entryToPath(handle, parentDir(FSTEntries, entry), path, maxlen);

	if (loc == maxlen) {
		return loc;
	}

	*(path + loc++) = '/';

	loc += myStrncpy(path + loc, name, maxlen - loc);

	return loc;
}

static BOOL ARCConvertEntrynumToPath(ARCHandle* handle, s32 entrynum, char* path, u32 maxlen)
{
	u32 loc;
	FSTEntry* FSTEntries = (FSTEntry*)handle->FSTStart;

	loc = entryToPath(handle, (u32)entrynum, path, maxlen);

	if (loc == maxlen) {
		path[maxlen - 1] = '\0';
		return FALSE;
	}

	if (entryIsDir(FSTEntries, entrynum)) {
		if (loc == maxlen - 1) {
			path[loc] = '\0';
			return FALSE;
		}

		path[loc++] = '/';
	}

	path[loc] = '\0';
	return TRUE;
}

BOOL ARCGetCurrentDir(ARCHandle* handle, char* path, u32 maxlen)
{
	return ARCConvertEntrynumToPath(handle, (s32)handle->currDir, path, maxlen);
}

void* ARCGetStartAddrInMem(ARCFileInfo* af)
{
	ARCHandle* handle = af->handle;
	return (void*)((u32)handle->archiveStartAddr + af->startOffset);
}

u32 ARCGetStartOffset(ARCFileInfo* af)
{
	return af->startOffset;
}

u32 ARCGetLength(ARCFileInfo* af)
{
	return af->length;
}

BOOL ARCClose(ARCFileInfo* af)
{
	return TRUE;
}
