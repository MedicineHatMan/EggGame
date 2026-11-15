#include "io/FileReader.h"
#include "Dolphin/os.h"
#include "core/PlacementNew.h"
#include "core/Mem.h"
#include "io/NinjaFile.h"
#include "sound/AudioLib.h"

static FileReader* sFileReader;

// 0x801A3288
FileReader* FileReader::get()
{
	if (sFileReader == 0) {
		sFileReader = new FileReader;
	}
	return sFileReader;
}

// 0x801A32D0
FileReader::FileReader()
    : mState(2)
{
	mNextId = 0;
	mPool.create(0x20);
	mFileBuffer = memAlloc(0xA088);
	mState      = 2;
}

// 0x801A3350
int FileReader::read(const char* name, void* target, FileReadCallback done, void* user)
{
	FileReadNode* node = mPool.alloc();
	if (node == 0) {
		return -1;
	}
	mPending.append(node);
	node->mType = FILEREAD_RESOURCE;
	resNameSet(&node->mName, name);
	node->mId     = mNextId++;
	node->mTarget = target;
	node->mUser   = user;
	node->mDone   = done;
	mState        = 0;
	return node->mId;
}

// 0x801A33E8
// The read's id or -1 if the queue is full
int FileReader::readAdx(const char* name, void* handle, void* user)
{
	FileReadNode* node = mPool.alloc();
	if (node == 0) {
		return -1;
	}
	mPending.append(node);
	node->mType = FILEREAD_ADX;
	resNameSet(&node->mName, name);
	node->mId     = mNextId++;
	node->mTarget = handle;
	node->mUser   = user;
	mState        = 0;
	return node->mId;
}

// 0x801A3478
void FileReader::cancelAll()
{
	FileReadNode* node;
	while ((node = mPending.mHead) != 0) {
		if (node->mType == FILEREAD_RESOURCE && node->mDone != 0) {
			node->mDone(1, node->mId, node->mTarget, node->mUser);
		}
		mPending.remove(node);
		mPool.free(node);
	}

	FileReadNode* next;
	for (node = mActive.mHead; node != 0; node = next) {
		next = node->mNext;
		switch (node->mType) {
		case FILEREAD_RESOURCE:
			node->mFile->cancel();
			break;
		case FILEREAD_ADX:
			mActive.remove(node);
			ADXF_StopPtLd();
			break;
		}
	}
	mState = 1;
}

// 0x801A3558
// 0 = busy 1 = idle
int FileReader::getState()
{
	int state = mState;
	if (state == 0 || state == 1) {
		return 0;
	}
	return state == 2 ? 1 : 2;
}

// 0x801A3588
// Opens the next pending file once nothing else is open
int FileReader::startNext()
{
	FileReadNode* node = mPending.mHead;
	if (node == 0) {
		return 0;
	}
	if (mActive.mCount != 0) {
		return 0;
	}

	mPending.remove(node);
	mActive.append(node);
	switch (node->mType) {
	case FILEREAD_RESOURCE:
		node->mFileBuffer = mFileBuffer;
		node->mFile       = new (node->mFileBuffer) TDvdFile(node->mName.mText);
		if (node->mDone != 0) {
			node->mDone(0, node->mId, node->mTarget, node->mUser);
		}
		break;
	case FILEREAD_ADX:
		ADXF_LoadPartitionNw(node->mTarget, &node->mName, 0, node->mUser);
		break;
	}
	return 1;
}

// 0x801A366C
void FileReader::update()
{
	if (mState == 0) {
		FileReadNode* next;
		FileReadNode* node;
		if (mPending.mCount == 0 && mActive.mCount == 0) {
			mState = 2;
			return;
		}
		startNext();
		for (node = mActive.mHead; node != 0; node = next) {
			next = node->mNext;
			switch (node->mType) {
			case FILEREAD_RESOURCE:
				if (((TResource*)node->mTarget)->load(node->mFile) == 1) {
					if (node->mDone != 0) {
						node->mDone(2, node->mId, node->mTarget, node->mUser);
					}
					node->mFile->~TFile();
					mActive.remove(node);
					mPool.free(node);
					OSReport("load:%s\n", node->mName.mText);
				}
				break;
			case FILEREAD_ADX:
				if (ADXF_GetPtStatEx(node->mTarget) == 3) {
					mActive.remove(node);
					mPool.free(node);
				}
				break;
			}
		}
	} else if (mState == 1) {
		FileReadNode* node;
		FileReadNode* next;
		for (node = mActive.mHead; node != 0; node = next) {
			next = node->mNext;
			if (node->mType == FILEREAD_RESOURCE && node->mFile->getStatus() == 3) {
				if (node->mDone != 0) {
					node->mDone(1, node->mId, node->mTarget, node->mUser);
				}
				node->mFile->~TFile();
				mActive.remove(node);
				mPool.free(node);
			}
		}
		if (mActive.mCount == 0) {
			mState = 2;
		}
	}
}

// 0x801A3868
void FileReadList::append(FileReadNode* node)
{
	if (mHead == 0) {
		mTail = node;
		mHead = node;
	} else {
		node->mPrev  = mTail;
		mTail->mNext = node;
		mTail        = node;
	}
	mCount++;
}

// 0x801A38A4
void FileReadList::remove(FileReadNode* node)
{
	if (node->mPrev != 0) {
		node->mPrev->mNext = node->mNext;
	} else {
		mHead = node->mNext;
	}
	if (node->mNext != 0) {
		node->mNext->mPrev = node->mPrev;
	} else {
		mTail = node->mPrev;
	}
	node->mNext = 0;
	node->mPrev = 0;
	mCount--;
}

// 0x801A3900
FileReadPool::FileReadPool()
{
	mFree  = 0;
	mNodes = 0;
	mCount = 0;
}

// 0x801A3914
FileReadNode* FileReadPool::alloc()
{
	FileReadNode* node = mFree;
	if (node != 0) {
		mFree       = node->mNext;
		node->mNext = 0;
		node->mPrev = 0;
	}
	return node;
}

// 0x801A393C
void FileReadPool::free(FileReadNode* node)
{
	if (node == 0) {
		return;
	}
	node->mNext = mFree;
	mFree       = node;
}

// 0x801A3954
void FileReadPool::create(int count)
{
	mNodes = new FileReadNode[count];
	for (int i = 0; i < count - 1; i++) {
		mNodes[i].mNext = &mNodes[i + 1];
	}
	mNodes[count - 1].mNext = 0;
	mFree                   = mNodes;
	mCount                  = count;
}
