#include "stage/Stage.h"

extern "C" {
const char* gTCustomGameContextName = "TCustomGameContext";
}

// 0x8019F7C0
TCustomGameContext::TCustomGameContext(TObject* parent)
    : TObject(parent)
{
	_00   = (u32)gTCustomGameContextName;
	mSize = 0x20;
}
