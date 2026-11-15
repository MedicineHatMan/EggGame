#include "io/BinFileEx.h"
#include "Ninja/ninja.h"
#include "core/Mem.h"

extern "C" {

static u8* sPrsSrc;     // 0x802C5A68
static u8 sPrsBitCount; // 0x802C5A6C
static u8 sPrsBits;     // 0x802C5A6D
}

// 0x801AC34C
extern "C" void prsInit()
{
	sPrsSrc      = 0;
	sPrsBitCount = 0;
	sPrsBits     = 0;
}

// 0x801AC360
static u8 prsReadByte()
{
	return *sPrsSrc++;
}

// 0x801AC374
// The control bits come eight to a byte low bit first
static u8 prsReadBit()
{
	if (--sPrsBitCount == 0) {
		sPrsBits     = prsReadByte();
		sPrsBitCount = 8;
	}
	u8 bit   = sPrsBits & 1;
	sPrsBits = sPrsBits >> 1;
	return bit;
}

// 0x801AC3C4
// 1 = a literal byte
extern "C" int prsDecode(u8* dst, u8* src, u32 size)
{
	int n = 0;
	int offset;
	u32 len;
	u8* out = dst;

	sPrsBitCount = 1;
	sPrsSrc      = src;
	while (true) {
		while (prsReadBit()) {
			*out++ = prsReadByte();
			n++;
		}
		if (prsReadBit()) {
			len   = prsReadByte();
			u32 b = prsReadByte();
			if ((b | len) == 0) {
				break;
			}
			offset = (b << 5) + (len >> 3) - 0x2000;
			len &= 7;
			if (len != 0) {
				len += 2;
			} else {
				len = prsReadByte() + 1;
			}
		} else {
			u8 hi  = prsReadBit();
			u8 lo  = prsReadBit();
			len    = (hi << 1) + lo + 2;
			offset = prsReadByte() - 0x100;
		}
		offset += n;
		while (len-- != 0) {
			n++;
			*out++ = dst[offset++];
		}
	}
	return n;
}

// 0x801AC4D8
BinFileEx::BinFileEx()
{
	mData  = 0;
	mSize  = 0;
	mState = 0;
	memset(&mHeader, 0, sizeof(mHeader));
	mPacked = 0;
}

// 0x801AC53C
BinFileEx::~BinFileEx()
{
	free();
}

// 0x801AC59C
// Reads the header then the packed data to mPacked then decodes it into mData
int BinFileEx::load(TFile* file)
{
	int again;
	int result = 0;
	do {
		again = false;
		switch (mState) {
		case 0:
			if (file->read(&mHeader, 0x20) > 0) {
				again   = true;
				mSize   = mHeader.mAllocSize;
				mData   = memAlloc(mSize);
				mPacked = (u8*)mData;
				mPacked += mHeader.mPackedAt;
				mState = 1;
			}
			break;
		case 1:
			if (file->read(mPacked, mHeader.mPackedSize) > 0) {
				if (mHeader.mVersion == 1) {
					again  = true;
					mState = 2;
				} else {
					mState = 3;
				}
			}
			break;
		case 2:
			prsInit();
			prsDecode((u8*)mData, mPacked, mHeader.mPackedSize);
			result = 1;
			break;
		case 3:
			result = 1;
			break;
		}
	} while (again);
	return result;
}

// 0x801AC6D4
int BinFileEx::getState()
{
	switch (mState) {
	case 0:
	case 1:
	case 2:
		return 0;
	case 3:
		return 1;
	}
	return 2;
}

// 0x801AC708
void BinFileEx::free()
{
	if (mState != 4 && mData != 0) {
		memFree(mData);
	}
	mData = 0;
	mSize = 0;
}
