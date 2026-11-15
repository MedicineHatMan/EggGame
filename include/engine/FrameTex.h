#ifndef FRAMETEX_H
#define FRAMETEX_H

#include "Ninja/ninja.h"

/**
 * @size 0x2C
 */
class FrameTex {
public:
	FrameTex(u16 width, u16 height, u32 texId, s32 format);
	~FrameTex();

	void request();
	void update();
	void copy();
	void set();
	void setRect(u16 x, u16 y, u16 width, u16 height);
	void setOrigin(u16 x, u16 y);
	void setDest(u16 x, u16 y);
	void setClear(int clear);
	void setFormat(int format);

	NJS_TEXNAME mTexName; // _00
	NJS_TEXLIST mTexList; // _0C
	u16 mX;               // _14 the screen rectangle to copy
	u16 mY;               // _16
	u16 mWidth;           // _18
	u16 mHeight;          // _1A
	void* mDest;          // _1C where in the texture the copy goes
	int mClear;           // _20 clears the frame buffer after the copy
	int mFormat;          // _24 the copy's texture format
	u32 mFlags;           // _28 1 a copy is waiting
};

extern "C" {
void frameTexBoot();
void frameTexInit();
void frameTexExit();
int frameTexIsOn();
void frameTexSelect(u16 id);
void frameTexCreate(u16 id, u16 width, u16 height, int format);
void frameTexDelete(u16 id);
void frameTexRequest(u16 id);
void frameTexRequestCur();
void frameTexUpdate(u16 id);
void frameTexUpdateCur();
void frameTexSet(u16 id);
void frameTexSetCur();
void frameTexSetOrigin(u16 id, u16 x, u16 y);
void frameTexSetRect(u16 id, u16 x, u16 y, u16 width, u16 height);
void frameTexSetDest(u16 id, u16 x, u16 y);
void frameTexSetClear(u16 id, int clear);
void frameTexSetFormat(u16 id, int format);
}

#endif
