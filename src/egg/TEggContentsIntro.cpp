#include "egg/TEggContentsIntro.h"
#include "egg/EggMessage.h"
#include "io/LangFile.h"
#include "io/Resource.h"
#include "io/ResFile.h"

const char* gTEggContentsIntroName = "TEggContentsIntro";

extern "C" {
BinFile* sEggMessageFile;
int sEggMessageCount;
char** sEggMessages;
}

/** mes_egg.bin The table holds one offset per message from 8 bytes past mTextStart */
struct EggMessageFile {
	u32 mTextStart;  // _00 from the start of the file
	u32 mTableSize;  // _04 in bytes
	char* mTable[1]; // _08
};

// 0x801BE1C0
// Built with propagation off like the other text-table loaders the pointer steps past the header in place data += 2 and
// the table shares its register
extern "C" void loadEggMessages()
{
	u32* data;
	u32 textStart;
	int i;

	loadSizedResource(getLangFileName("mes_egg.bin"), &sEggMessageFile);
	data      = (u32*)sEggMessageFile->mData;
	textStart = *data;
	textStart += (u32)data++;
	u32 size         = *data++;
	sEggMessages     = (char**)data;
	sEggMessageCount = size >> 2;
	for (i = 0; i < sEggMessageCount; i++) {
		sEggMessages[i] += textStart + 8;
	}
}

// 0x801BE248
extern "C" void freeEggMessages()
{
	if (sEggMessageFile != 0) {
		releaseResource(sEggMessageFile);
		sEggMessageFile = 0;
	}
}

typedef char TEggContentsIntro_size_check[(sizeof(TEggContentsIntro) == 0x28) ? 1 : -1];

// 0x801BE27C
extern "C" TEggContentsIntro* createTEggContentsIntro(TObject* parent, int index)
{
	return new TEggContentsIntro(parent, index);
}

// 0x801BE2D0
TEggContentsIntro::TEggContentsIntro(TObject* parent, int index)
    : TObject(parent)
{
	int slot;

	_00   = (u32)gTEggContentsIntroName;
	mSize = 0x28;
	mText = 0;
	slot  = index - 1;
	if (slot >= 0 && slot < sEggMessageCount) {
		mText = createSimpleTextTimed(sEggMessages[slot], 0xB4);
	}
	if (mText != 0) {
		mText->_A8 = 1;
	}
	mDone = 0;
}

// 0x801BE370
TEggContentsIntro::~TEggContentsIntro()
{
	if (mText != 0) {
		mText->mHold = 1;
		mText        = 0;
	}
}

// 0x801BE3EC
void TEggContentsIntro::draw()
{
	if (mDone != 0) {
		return;
	}
	if (mText == 0) {
		return;
	}
	if (mText->mState != 3) {
		return;
	}
	mDone = 1;
}
