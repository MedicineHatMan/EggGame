#include "ui/TSimpleText.h"

const f32 gTextGapX   = 2.0f;  // 0x802C7740
const f32 gTextGapY   = 2.0f;  // 0x802C7744
const f32 gTextGlyphW = 22.0f; // 0x802C7748
const f32 gTextGlyphH = 22.0f; // 0x802C774C

// 0x80256D90
// \tc0 to \tc7 \tc
u32 sTextColors[8] = {
	0x000000FF, 0xFF9955FF, 0x44A7E5FF, 0xFFFF00FF, 0xFF00FFFF, 0x00FF00FF, 0xFFA050FF, 0xFFFFFFFF,
};

// 0x80256DB0
// \tw0 to \tw2 \tw
int sTextWaits[3] = { 1, 5, 20 };

// 0x8006AD74
#pragma scheduling off
TextGlyph::TextGlyph(const FontGlyph* glyph, u32 color)
{
	mGlyph.mSlot   = glyph->mSlot;
	mGlyph.mBlock  = glyph->mBlock;
	mGlyph.mIndex  = glyph->mIndex;
	mGlyph.mWidth  = glyph->mWidth;
	mGlyph.mHeight = glyph->mHeight;
	mGlyph._10     = glyph->_10;
	mGlyph.mU0     = glyph->mU0;
	mGlyph.mU1     = glyph->mU1;
	mGlyph.mV0     = glyph->mV0;
	mGlyph.mV1     = glyph->mV1;
	mColor         = color;
}
#pragma scheduling reset

// 0x8006ADCC
TextGlyph::~TextGlyph()
{
	fontReleaseGlyph(&mGlyph);
}

// 0x8006AE14
int TextGlyph::isNarrow()
{
	return mGlyph.mWidth <= 11.0f;
}

// 0x8006AE30
void TextGlyph::update() { }

// 0x8006AE34
f32 TextGlyph::draw(f32 x, f32 y, const Vec3f* scale)
{
	textSetColor(mColor);
	textDrawQuad(mGlyph.mU0, mGlyph.mV0, mGlyph.mU1, mGlyph.mV1, x, y, mGlyph.mWidth * scale->x,
	             mGlyph.mHeight * scale->y);
	return mGlyph.mWidth * scale->x;
}

// 0x8006AED0
TextLine::TextLine(int max, int maxWidth)
{
	mMax    = max;
	mCount  = 0;
	mGlyphs = new TextGlyph*[mMax];
	for (int i = 0; i < mMax; i++) {
		mGlyphs[i] = 0;
	}
	mWidth    = 0;
	mMaxWidth = maxWidth;
}

// 0x8006AF54
TextLine::~TextLine()
{
	clear();
	if (mGlyphs != 0) {
		delete[] mGlyphs;
		mGlyphs = 0;
	}
}

// 0x8006AFB4
void TextLine::clear()
{
	for (int i = mMax - 1; i >= 0; i--) {
		if (mGlyphs[i] != 0) {
			delete mGlyphs[i];
			mGlyphs[i] = 0;
		}
	}
	mCount = 0;
	mWidth = 0;
}

// 0x8006B02C
// Zero when the line is full
int TextLine::add(f32 scale, TextGlyph* glyph)
{
	if (mCount < mMax) {
		int width = (int)(scale * glyph->mGlyph.mWidth + 2.0f);
		if (mWidth + width <= mMaxWidth) {
			mGlyphs[mCount++] = glyph;
			mWidth += width;
			return 1;
		}
	}
	return 0;
}

// 0x8006B0A0
void TextLine::update()
{
	for (int i = 0; i < mCount; i++) {
		mGlyphs[i]->update();
	}
}

// 0x8006B0F4
// Narrow glyphs get half the gap
void TextLine::draw(f32 x, f32 y, f32 gap, const Vec3f* scale)
{
	f32 half = 0.5f * gap;
	for (int i = 0; i < mCount; i++) {
		x += mGlyphs[i]->draw(x, y, scale);
		if (mGlyphs[i]->isNarrow()) {
			x += half;
		} else {
			x += gap;
		}
	}
}

// 0x8006B1D0
TextReader::TextReader()
{
	start(0);
}

// 0x8006B204
void TextReader::start(const void* text)
{
	mColor = sTextColors[7];
	mWait  = sTextWaits[1];
	mPos   = 0;
	mText  = (const u8*)text;
}

// 0x8006B234
// Zero at the end of the message
int TextReader::read(u8* out, int n)
{
	for (int i = 0; i < n; i++) {
		u8 c = mText[mPos];
		if ((int)c == 0) {
			return 0;
		}
		mPos++;
		*out++ = c;
	}
	return 1;
}

// 0x8006B278
int TextReader::readByte(u8* out)
{
	return read(out, 1);
}

// 0x8006B29C
// The word after a tab
int TextReader::readControl(int* kind)
{
	int more = 0;
	u8 c;
	u8 hex[6];

	*kind = TEXT_END;
	if (readByte(&c)) {
		switch (c) {
		case 'p':
			*kind = TEXT_PAGE;
			break;
		case 'b':
			*kind = TEXT_NEWLINE;
			break;
		case 'c':
			if (readByte(&c)) {
				if ((u32)c == '!') {
					mColor = sTextColors[7];
				} else {
					int i = c - '0';
					if (i >= 0 && i < 8) {
						mColor = sTextColors[i];
					}
				}
				more = 1;
			} else {
				*kind = TEXT_END;
			}
			break;
		case 'd':
			if (read(hex, 6)) {
				u32 color = 0;
				for (int i = 0; i < 6; i++) {
					color <<= 4;
					color |= (hex[i] - 0x20) & 0xF;
				}
				more   = 1;
				mColor = (color << 8) | 0xFF;
			} else {
				*kind = TEXT_END;
			}
			break;
		case 'w':
			if (readByte(&c)) {
				if ((u32)c == '!') {
					mWait = sTextWaits[1];
				} else {
					int i = c - '0';
					if (i >= 0 && i < 3) {
						mWait = sTextWaits[i];
					}
				}
				more = 1;
			} else {
				*kind = TEXT_END;
			}
			break;
		}
	}
	return more;
}

// 0x8006B460
int TextReader::readChar(u8 c, u16* code)
{
	if (fontIsLead(c)) {
		*code = c << 8;
		if (readByte(&c)) {
			*code |= c;
			return TEXT_WIDE_CHAR;
		}
		return TEXT_END;
	}
	*code = c;
	return TEXT_CHAR;
}

// 0x8006B4EC
int TextReader::next(u16* code)
{
	u8 c;
	int kind;
	int more;
	do {
		kind = TEXT_END;
		more = 0;
		if (readByte(&c)) {
			if (c == '\t') {
				more = readControl(&kind);
			} else {
				kind = readChar(c, code);
			}
		}
	} while (more);
	return kind;
}
