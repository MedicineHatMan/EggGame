#ifndef _BINFILEEX_H
#define _BINFILEEX_H

#include "io/Resource.h"

/**
 * @size 0x20
 */
struct PrdHeader {
	s32 mVersion;    // _00 1 = compressed
	u32 mAllocSize;  // _04 how much memory the file needs the decompressed size plus room for the packed data
	u32 mPackedAt;   // _08 where the packed data is read to inside that memory
	u32 mSize;       // _0C the decompressed size
	u32 mPackedSize; // _10
	u8 _14[0xC];
};

/**
 * Reads a .prd file: the header, then the packed data into the top of one block, which is then decoded into the
 * bottom of it. Shaped like BinFile (mData and mSize at the same place) so PrdFile can use it as one.
 * Vtable 0x80266FD0.
 *
 * @size 0x5C
 */
class BinFileEx : public TResource {
public:
	BinFileEx();                   // 0x801AC4D8
	virtual ~BinFileEx();          // 0x801AC53C
	virtual int load(TFile* file); // 0x801AC59C
	virtual int getState();        // 0x801AC6D4 0 = loading 1 = done 2 = failed

	void free(); // 0x801AC708

	u8 _28[4];
	void* mData;       // _2C
	u32 mSize;         // _30
	int mState;        // _34 0 header 1 packed data 2 decode 3 done 4 memory it does not own
	PrdHeader mHeader; // _38
	u8* mPacked;       // _58 where the packed data goes
};

extern "C" {
void prsInit();                            // 0x801AC34C
int prsDecode(u8* dst, u8* src, u32 size); // 0x801AC3C4 returns the decoded size
}

#endif
