#include "core/types.h"
#include "ui/MessageTable.h"

static void** sMessages;  // 0x802C5790
static int sMessageCount; // 0x802C5794

// 0x80153F70
void setMessageTable(MessageFile* file)
{
	u32 text;
	int i;

	if (file == NULL) {
		sMessages     = NULL;
		sMessageCount = 0;
		return;
	}
	u32 size = file->mTableSize;
	void** msgs = (void**)file->mOffsets;
	text = file->mTextOffset + (u32)file;
	sMessages = msgs;
	sMessageCount = size >> 2;
	for (i = 0; i < sMessageCount; i++) {
		((char**)sMessages)[i] += text + 8;
	}
}

// 0x80153FDC
void* getMessage(int id)
{
	if (id >= sMessageCount) {
		return NULL;
	}
	return sMessages[id];
}

// 0x80154000
int getMessageCount()
{
	return sMessageCount;
}
