#include "io/ResFile.h"
#include "core/Mem.h"
#include "io/NinjaFile.h"

// 0x8004CCFC
BinFile::BinFile()
{
	mData  = 0;
	mSize  = 0;
	mState = 0;
}

// 0x8004CD48
BinFile::BinFile(void* data, u32 size)
{
	mData  = data;
	mSize  = size;
	mState = 3;
}

// 0x8004CD9C
BinFile::~BinFile()
{
	free();
}

// 0x8004CDFC
// Asks the file for its size then reads it all in one go
int BinFile::load(TFile* file)
{
	switch (mState) {
	case 0:
		mSize  = file->getSize();
		mData  = memAlloc(mSize);
		mState = 1;
	case 1:
		if (file->read(mData, mSize) <= 0) {
			break;
		}
		mState = 2;
		return 1;
	case 2:
		return 1;
	}
	return 0;
}

// 0x8004CEBC
int BinFile::getState()
{
	switch (mState) {
	case 0:
	case 1:
		return 0;
	case 2:
		return 1;
	}
	return 2;
}

// 0x8004CEF0
void BinFile::free()
{
	if (mState != 3 && mData != 0) {
		memFree(mData);
	}
	mData = 0;
	mSize = 0;
}

// 0x8004CF40
ChunkFile::ChunkFile(u32 magic)
{
	mState = 0;
	mData  = 0;
	NinjaFile* file = (NinjaFile*)operator new(0x28);
	if (file != 0) {
		file = ninjaFileInit(file);
	}
	mFile = file;
	if (mFile != 0) {
		mKinds[0]     = magic;
		mKinds[1]     = 0;
		mFile->mKinds = mKinds;
	}
}

// 0x8004CFCC
ChunkFile::ChunkFile(u32 magic, void* mem)
{
	mState = 3;
	mData  = 0;
	NinjaFile* file = (NinjaFile*)operator new(0x28);
	if (file != 0) {
		file = ninjaFileInit(file);
	}
	mFile = file;
	if (mFile != 0) {
		mKinds[0]     = magic;
		mKinds[1]     = 0;
		mFile->mKinds = mKinds;
	}
	ninjaFileFromMemory(mFile, (u8*)mem);
	mData = ninjaFileTake(mFile, 0);
}

// 0x8004D07C
ChunkFile::~ChunkFile()
{
	if (mState != 3 && mData != 0) {
		memFree(mData);
	}
	mData = 0;
	ninjaFileFree(mFile, 1);
}

// 0x8004D108
int ChunkFile::load(TFile* file)
{
	switch (mState) {
	case 0:
		mState = 1;
	case 1: {
		int ret = ninjaFileRead(mFile, file);
		if (ret > 0) {
			mData  = ninjaFileTake(mFile, 0);
			ret    = 1;
			mState = 2;
		}
		return ret;
	}
	case 2:
		return 1;
	}
	return -1;
}

// 0x8004D19C
int ChunkFile::getState()
{
	switch (mState) {
	case 0:
	case 1:
		return 0;
	case 2:
		return 1;
	}
	return 2;
}
