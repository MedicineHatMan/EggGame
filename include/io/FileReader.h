#ifndef _FILEREADER_H
#define _FILEREADER_H

#include "core/types.h"
#include "io/LangFile.h"
#include "io/ResManager.h"

typedef void (*FileReadCallback)(int event, int id, void* target, void* user);

enum FileReadType {
	FILEREAD_RESOURCE = 0, // a TResource loaded from a TDvdFile
	FILEREAD_ADX      = 1, // streamed through an ADXF handle
};

/**
 * One queued read.
 * @size 0x44
 */
struct FileReadNode {
	FileReadNode() { }

	FileReadNode* mNext;    // _00
	FileReadNode* mPrev;    // _04
	int mType;              // _08 FileReadType
	ResName mName;          // _0C
	int mId;                // _2C
	void* mTarget;          // _30 the TResource or the ADXF handle
	void* mUser;            // _34
	FileReadCallback mDone; // _38
	TFile* mFile;           // _3C
	void* mFileBuffer;      // _40 where mFile was built
};

/**
 * @size 0xC
 */
struct FileReadPool {
	FileReadPool();

	FileReadNode* alloc();
	void free(FileReadNode* node);
	void create(int count);

	FileReadNode* mNodes; // _00
	FileReadNode* mFree;  // _04
	int mCount;           // _08
};

/**
 * @size 0xC
 */
struct FileReadList {
	FileReadList()
	{
		mHead  = 0;
		mTail  = 0;
		mCount = 0;
	}

	void append(FileReadNode* node);
	void remove(FileReadNode* node);

	FileReadNode* mHead; // _00
	FileReadNode* mTail; // _04
	int mCount;          // _08
};

/**
 * Reads files one at a time, ticked once a frame from the main loop.
 * @size 0x30
 */
class FileReader {
public:
	FileReader();

	static FileReader* get();

	int read(const char* name, void* target, FileReadCallback done, void* user);
	int readAdx(const char* name, void* handle, void* user);
	void cancelAll();
	int getState();
	int startNext();
	void update();

	int mState;             // _00 0 = reading 1 = cancelling 2 = idle
	FileReadPool mPool;     // _04
	FileReadList mPending;  // _10
	FileReadList mActive;   // _1C
	void* mFileBuffer;      // _28 0xA088 bytes for a TDvdFile
	int mNextId;            // _2C
};

#endif
