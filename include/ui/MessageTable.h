#ifndef _UI_MESSAGETABLE_H_
#define _UI_MESSAGETABLE_H_
struct MessageFile;

#include "core/types.h"

extern "C" {
int getMessageCount();
}

extern "C" {
void* getMessage(int);
}

extern "C" {
void setMessageTable(MessageFile*);
}

struct MessageFile {
	int mTextOffset; // _00 from the end of the table header
	u32 mTableSize;  // _04 in bytes
	int mOffsets[1]; // _08
};

#endif
