#include "io/AramFile.h"
#include "Dolphin/OS/OSCache.h"
#include "Ninja/ninja.h"

extern "C" {
void ARQPostRequest(AramRequest*, u32 owner, u32 type, u32 priority, u32 source, u32 dest, u32 length,
                    void (*callback)(AramRequest*));

// 0x80065FD0
int fileReadAll(TFile* file, void* dest, int size)
{
	int ret = 0;
	while (ret == 0) {
		ret = file->read(dest, size);
		if (ret < 0) {
			return -1;
		}
	}
	return 1;
}
}

// 0x80066040
TAramFile::TAramFile(u32 aram, u32 size)
{
	mBuffer     = (u8*)(((u32)mBuf + 0x1F) & ~0x1F);
	mAram       = aram;
	mSize       = size;
	mBlockCount = (mSize + 0x1F) >> 5;
	mBufPos     = 0;
	mBlockSize  = 0x20;
	mFillBlocks = 1;
	mPos        = 0;
	mReqBlock   = 0;
	mBlock      = -1;
	mBufBytes   = 0;
	mState      = 0;
	mDone       = 0;
}

// 0x800660B4
TAramFile::~TAramFile() { }

// 0x80066110
int TAramFile::seek(int offset, int from)
{
	switch (from) {
	case 1:
		mPos += offset;
		break;
	case 0:
		mPos = offset;
		break;
	case 2:
		mPos = mSize - offset;
		break;
	}
	return mPos;
}

// 0x80066160
int TAramFile::tell()
{
	return mPos;
}

// 0x80066168
void TAramFile::cancel() { }

// 0x8006616C
int TAramFile::getSize()
{
	return mSize;
}

// 0x80066174
int TAramFile::getStatus()
{
	switch (mState) {
	case 0:
		return 0;
	case 1:
	case 2:
		return 1;
	case 3:
		return 2;
	}
	return 4;
}

// 0x800661B4
int TAramFile::read(void* dest, int size)
{
	if (size == 0) {
		return 1;
	}

	switch (mState) {
	case 2:
		if (mDone == 1) {
			if (mDirect == 1) {
				int len = mRequest.mLength;
				mLeft -= len;
				mPos += len;
				mDest += len;
				if (mLeft <= 0) {
					mState = 3;
					return 1;
				}
				mState = 1;
			} else {
				mState  = 1;
				mBufPos = 0;
			}
		}
		return 0;
	case 0:
	case 3:
		mLeft  = size;
		mDest  = (u8*)dest;
		mState = 1;
	case 1:
		while (mState != 3) {
			if (mSize < mPos + mLeft) {
				mLeft = mSize - mPos;
			}
			mReqBlock = mPos >> 5;
			if (mReqBlock > mBlockCount) {
				return -1;
			}
			if (isBuffered(mReqBlock) > 0) {
				mBufPos   = mPos - mBlock * 0x20;
				int avail = mBufBytes - mBufPos;
				if (mLeft <= avail) {
					syMemCopy(mDest, mBuffer + mBufPos, mLeft);
					mPos += mLeft;
					mLeft     = 0;
					mDest     = 0;
					mState    = 3;
					mReqBlock = mPos >> 5;
					mBufPos   = mPos - mBlock * 0x20;
					if (mBufPos == 0 && mReqBlock < mBlockCount) {
						fill(mReqBlock);
					}
					return 1;
				}
				syMemCopy(mDest, mBuffer + mBufPos, avail);
				mPos += avail;
				mLeft -= avail;
				mDest += avail;
			} else {
				return fetch(mReqBlock, mLeft);
			}
		}
		break;
	case 4:
		return -1;
	}
	return -1;
}

// 0x8006641C
extern "C" void aramFileDone(AramRequest* request)
{
	((TAramFile*)request->mOwner)->mDone = 1;
}

// 0x8006642C
int TAramFile::fill(int block)
{
	if (block >= mBlockCount || block < 0) {
		return -1;
	}
	if (mBlock != block) {
		mBlock = block;
		if ( (u32)mBlockCount > (u32)(block + mFillBlocks)) {
			mBufBlocks = mFillBlocks;
			mBufBytes  = mBufBlocks << 5;
		} else {
			mBufBlocks = mBlockCount - block;
			mBufBytes  = mSize - block * 0x20;
		}
		mReqBlock = block;
		mState    = 2;
		mDirect   = 0;
		mDone     = 0;
		DCInvalidateRange(mBuffer, (mBufBytes + 0x1F) & ~0x1F);
		ARQPostRequest(&mRequest, (u32)this, 1, 1, mAram + block * 0x20, (u32)mBuffer, (mBufBytes + 0x1F) & ~0x1F,
		               aramFileDone);
	}
	return 0;
}

// 0x8006652C
int TAramFile::fetch(int block, int size)
{
	if (size >= 0x20 && ((u32)mDest & 0x1F) == 0) {
		if (block < 0 || block >= mBlockCount) {
			return -1;
		}
		u32 len   = size & ~0x1F;
		mReqBlock = block;
		mBlock    = -1;
		mState    = 2;
		mDirect   = 1;
		mDone     = 0;
		DCInvalidateRange(mDest, (len + 0x1F) & ~0x1F);
		ARQPostRequest(&mRequest, (u32)this, 1, 1, mAram + block * 0x20, (u32)mDest, len, aramFileDone);
	} else {
		return fill(block);
	}
	return 0;
}

// 0x80066608
int TAramFile::isBuffered(int block)
{
	if (mBlock < 0) {
		return -1;
	}
	int last = mBufBlocks + mBlock;
	for (int i = 0; i < 1; i++) {
		last--;
		if (mBlock <= block && block <= last) {
			return 1;
		}
	}
	return -1;
}
