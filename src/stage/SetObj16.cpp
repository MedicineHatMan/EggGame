#include "core/types.h"

class TObject;
class SetDesc;

/** A colour as one word RGBA */
struct SetObjColor {
	SetObjColor(u32 color)
	    : mColor(color)
	{
	}

	u32 mColor;
};

// 0x80280088
SetObjColor gSetObjColors[7] = {
	0xFF7F3FFF, 0xFF7F3FFF, 0xFF7F3FFF, 0xFF3F7FFF, 0x3FFF7FFF, 0x7FFF3FFF, 0x3F7FFFFF,
};

// 0x800959F0
extern "C" TObject* createSetObj16(TObject* parent, SetDesc* desc)
{
	return 0;
}
