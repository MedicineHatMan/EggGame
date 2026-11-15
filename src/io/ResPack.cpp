#include "io/ResPack.h"
#include "sdk/Dolphin/ar.h"
#include "core/Mem.h"
#include "Ninja/ninja.h"

#define RESPACK_CHUNK 0x8000

static volatile int sDmaDone;

// 0x801B9FD4
static void dmaDone(u32 request)
{
	sDmaDone = 1;
}

// 0x801B9FE0
ResPack::ResPack()
{
	mState  = 0;
	mHeader = 0;
	mAram   = 0;
	mBuffer = (u8*)(((u32)this + 0x4F) & ~0x1F);
}

// 0x801BA03C
ResPack::~ResPack()
{
	if (mHeader != 0) {
		memFree(mHeader);
	}
	if (mAram != 0) {
		ARFree(0);
	}
}

// 0x801BA0C4
// 1 once the whole archive is in
int ResPack::load(TFile* file)
{
	switch (mState) {
	case 0:
		if (file->read(mBuffer, 0x20) > 0) {
			ArcFileHeader* header = (ArcFileHeader*)mBuffer;
			mHeader               = (ArcFileHeader*)memAlloc(header->mFileStart);
			syMemCopy(mHeader, mBuffer, 0x20);
			mFileStart = header->mFileStart;
			mDataSize  = file->getSize() - header->mFileStart;
			mFst       = (u8*)mHeader + header->mFstStart;
			mChunk     = header->mFstSize;
			mAram      = ARAlloc((mDataSize + 0x1F) & ~0x1F);
			mState     = 1;
		}
		break;
	case 1:
		if (file->read(mFst, mChunk) > 0) {
			file->seek(mHeader->mFileStart, 0);
			mChunk = mDataSize < RESPACK_CHUNK ? mDataSize : RESPACK_CHUNK;
			mDone  = 0;
			mState = 2;
		}
		break;
	case 2:
		if (file->read(mBuffer, mChunk) > 0) {
			ARQRequest request;
			sDmaDone = 0;
			ARQPostRequest(&request, 0, ARQ_TYPE_MRAM_TO_ARAM, ARQ_PRIORITY_HIGH, (u32)mBuffer, mAram + mDone,
			               (mChunk + 0x1F) & ~0x1F, dmaDone);
			while (sDmaDone == 0) { }
			mDone += mChunk;
			if (mDone == mDataSize) {
				mState = 3;
				ARCInitHandle(mHeader, &mHandle);
			} else {
				int left = mDataSize - mDone;
				mChunk   = left < RESPACK_CHUNK ? left : RESPACK_CHUNK;
			}
		}
		break;
	case 3:
		return 1;
	}
	return 0;
}

// 0x801BA344
// 0 loading 1 loaded 2 bad state
int ResPack::getState()
{
	switch (mState) {
	case 0:
	case 1:
	case 2:
		return 0;
	case 3:
		return 1;
	default:
		return 2;
	}
}
