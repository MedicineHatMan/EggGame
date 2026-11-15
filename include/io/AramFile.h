#ifndef _ARAMFILE_H
#define _ARAMFILE_H

#include "io/Resource.h"

struct AramRequest {
	AramRequest* mNext; // _00
	u32 mOwner;         // _04
	u32 mType;          // _08
	u32 mPriority;      // _0C
	u32 mSource;        // _10
	u32 mDest;          // _14
	u32 mLength;        // _18
	void (*mCallback)(AramRequest*); // _1C
};

/**
 * Reads a file that a ResPack streamed into ARAM. Reads that start 32 byte aligned and are at least 32 bytes go
 * straight to the caller's memory; the rest go through a one-block buffer. Vtable 0x80256A90.
 *
 * @size 0xA8
 */
class TAramFile : public TFile {
public:
	TAramFile(u32 aram, u32 size); // 0x80066040
	virtual ~TAramFile();           // 0x800660B4
	virtual int seek(int offset, int from);  // 0x80066110
	virtual int tell();                      // 0x80066160
	virtual void cancel();                   // 0x80066168 does nothing
	virtual int getSize();                   // 0x8006616C
	virtual int getStatus();                 // 0x80066174
	virtual int read(void* dest, int size);  // 0x800661B4

	int fill(int block);                     // 0x8006642C into the buffer
	int fetch(int block, int size);          // 0x8006652C straight to mDest when it can
	int isBuffered(int block);               // 0x80066608 1 or -1

	AramRequest mRequest; // _04
	u8 mBuf[0x40];        // _24 mBuffer is the aligned block inside
	u8* mBuffer;          // _64
	int mBlock;           // _68 the first block in the buffer -1 for none
	int mBlockSize;       // _6C 0x20
	int mFillBlocks;      // _70 how many blocks a fill asks for 1
	int mBufBlocks;       // _74
	int mBufBytes;        // _78
	int mSize;            // _7C
	int mBlockCount;      // _80
	int mBufPos;          // _84
	int mDirect;          // _88 1 when the request went straight to mDest
	int mState;           // _8C 0 = new 1 = reading 2 = waiting for ARAM 3 = done 4 = failed
	int mReqBlock;        // _90
	int mPos;             // _94
	u32 mAram;            // _98
	int mLeft;            // _9C
	u8* mDest;            // _A0
	int mDone;            // _A4 set by the request's callback
};

extern "C" int fileReadAll(TFile* file, void* dest, int size); // 0x80065FD0

#endif
