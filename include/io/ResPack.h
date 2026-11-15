#ifndef _RESPACK_H
#define _RESPACK_H

#include "io/ResManager.h"
#include "io/nt_arc.h"

/**
 * @size 0x20
 */
struct ArcFileHeader {
	u32 mMagic;     // _00
	u32 mFstStart;  // _04
	u32 mFstSize;   // _08
	u32 mFileStart; // _0C everything from here on is file data
	u8 _10[0x10];
};

/**
 * An ARC archive kept in ARAM, read with addResPack (amem_boot.nrc). Loading keeps the header and file table in main
 * memory and streams the file data into ARAM through a 0x8000 byte buffer. Files are then found through mHandle and
 * read back out of ARAM by readFromResPack. The name is ours.
 *
 * @size 0x808C
 */
class ResPack : public TResource {
public:
	ResPack();
	virtual ~ResPack();
	virtual int load(TFile* file);
	virtual int getState();

	u8 _28[4];              // _28 as in TArc
	int mState;             // _2C 0 header 1 file table 2 file data 3 done
	u8 mBufferMem[0x8020];  // _30 mBuffer is this rounded up to 32
	u8* mBuffer;            // _8050
	u8* mFst;               // _8054 where the file table goes inside mHeader
	ArcFileHeader* mHeader; // _8058 the header and file table from memAlloc
	u32 mDone;              // _805C how much file data is in ARAM
	u32 mFileStart;         // _8060 where the file data starts in the archive
	u32 mChunk;             // _8064 the size of the read in flight
	u32 mAram;              // _8068 the file data in ARAM
	s32 mDataSize;          // _806C long the first clamp copies its int limit into this value's register
	ARCHandle mHandle;      // _8070
};

#endif
