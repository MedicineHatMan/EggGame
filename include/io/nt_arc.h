#ifndef _NT_ARC_H
#define _NT_ARC_H

#include "core/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// The Dolphin SDK's ARC U8 archive reader built into the game as nt_arc.c the name is from its OSPanic string
// Names and layouts are the SDK's

#define DARCH_MAGIC 0x55AA382D

/**
 * The header at the start of an ARC file.
 * @size 0x20
 */
typedef struct ARCHeader {
	u32 magic;     // _00
	u32 fstStart;  // _04
	u32 fstSize;   // _08
	u32 fileStart; // _0C everything from here on is file data
	u32 reserve[4];
} ARCHeader;

/**
 * What ARCInitHandle fills in.
 * @size 0x1C
 */
typedef struct ARCHandle {
	void* archiveStartAddr; // _00
	void* FSTStart;         // _04
	void* fileStart;        // _08
	u32 entryNum;           // _0C
	char* FSTStringStart;   // _10
	u32 FSTLength;          // _14
	u32 currDir;            // _18
} ARCHandle;

/**
 * One open file inside an archive.
 * @size 0xC
 */
typedef struct ARCFileInfo {
	ARCHandle* handle; // _00
	u32 startOffset;   // _04
	u32 length;        // _08
} ARCFileInfo;

BOOL ARCInitHandle(void* arcStart, ARCHandle* handle);
BOOL ARCOpen(ARCHandle* handle, const char* fileName, ARCFileInfo* af);
BOOL ARCFastOpen(ARCHandle* handle, s32 entrynum, ARCFileInfo* af);
s32 ARCConvertPathToEntrynum(ARCHandle* handle, const char* pathPtr);
BOOL ARCGetCurrentDir(ARCHandle* handle, char* path, u32 maxlen);
void* ARCGetStartAddrInMem(ARCFileInfo* af);
u32 ARCGetStartOffset(ARCFileInfo* af);
u32 ARCGetLength(ARCFileInfo* af);
BOOL ARCClose(ARCFileInfo* af);

#ifdef __cplusplus
}
#endif

#endif
