#ifndef _TSIMPLETEXT_H
#define _TSIMPLETEXT_H

#include "stage/Stage.h"
struct DrawTask;
#include "ui/Font.h"

extern "C" void objHeapFree(void*, void*);

// Spacing in glyph units defined in Text.cpp and read by TSimpleText
extern const f32 gTextGapX;   // 0x802C7740 2
extern const f32 gTextGapY;   // 0x802C7744 2
extern const f32 gTextGlyphW; // 0x802C7748 22
extern const f32 gTextGlyphH; // 0x802C774C 22

// Kinds TextReader::next gives back
enum TextKind {
	TEXT_CHAR      = 0, // a one byte character
	TEXT_WIDE_CHAR = 1, // a two byte character
	TEXT_NEWLINE   = 2, // \tb
	TEXT_PAGE      = 3, // \tp
	TEXT_END       = 4,
};

/** A glyph on a line, with the colour it was read in. @size 0x20 */
class TextGlyph {
public:
	TextGlyph(const FontGlyph* glyph, u32 color);
	~TextGlyph();

	int isNarrow();
	void update();
	f32 draw(f32 x, f32 y, const Vec3f* scale);

	FontGlyph mGlyph; // _00
	u32 mColor;       // _1C
};

/** One line of glyphs, up to mMax of them and mMaxWidth wide. @size 0x14 */
class TextLine {
public:
	TextLine(int max, int maxWidth);
	~TextLine();

	void clear();
	int add(f32 scale, TextGlyph* glyph);
	void update();
	void draw(f32 x, f32 y, f32 gap, const Vec3f* scale);

	int mMax;            // _00
	int mCount;          // _04
	TextGlyph** mGlyphs; // _08
	int mWidth;          // _0C
	int mMaxWidth;       // _10
};

/**
 * Reads characters and control words out of a message. A tab starts a control word: \tb breaks the line, \tp the
 * page, \tc0-\tc7 pick a colour (\tc! the default), \td sets a colour from six hex digits and \tw0-\tw2 the
 * typing speed (\tw! the default). @size 0x10
 */
struct TextReader {
	TextReader();
	void start(const void* text);
	int read(u8* out, int n);
	int readByte(u8* out);
	int readControl(int* kind);
	int readChar(u8 c, u16* code);
	int next(u16* code);

	u32 mColor;      // _00 RGBA
	int mWait;       // _04 frames a character
	u32 mPos;        // _08
	const u8* mText; // _0C
};

/**
 * A block of speech text. mRows is how many lines fit in mBox. mId == -1 leaves mText null and starts in state 1.
 * Kind 1 and kind 2 set bit 0 of mBits. draw() advances mState.
 *
 * @size 0xAC
 */

class TSimpleText : public TObject {
public:
	TSimpleText(TObject* parent, int kind, int id, const Vec3f* at, const Vec3f* box, const Vec3f* glyph, void* desc);
	virtual ~TSimpleText();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C 0x8010FF84
	virtual void vfn_10(); // _10 0x80110048
	virtual void render(); // _1C 0x8011007C

	void buildLines();                // 0x8010FE50
	void waitReady();                 // 0x80110390
	void readInput();                 // 0x801103AC
	void finishLine();                // 0x8011061C
	void retire();                    // 0x80110690
	int bit0();                       // 0x801106A0 mBits bit 0
	int bit1();                       // 0x801106AC mBits bit 1
	int rowFull();                    // 0x801106B8
	void clearCount();                // 0x801106DC
	void scrollLine();                // 0x801106E8
	void applyPage();                 // 0x8011078C
	int advancePage();                // 0x801107E8
	int hasMoreText();                // 0x80110B20
	void resetPage(const void* text); // 0x80110B2C Restarts on text
	void closeText();                 // 0x80110B78

	DrawTask* mDraw;                // _20
	int mState;                     // _24 0 wait 1 reading 2 closing 4 retire
	class TAeObjMsgWindow* mWindow; // _28 null when mId is -1
	Vec3f mAt;                      // _2C
	Vec3f mBox;                     // _38 x and y size the lines
	int mId;                        // _44
	Vec3f mGlyph;                   // _48 glyph size
	int mTimer;                     // _54
	int mHold;                      // _58 non-zero keeps state 4 instead of 3
	TextLine** mLines;              // _5C
	int mKind;                      // _60
	int mRows;                      // _64
	int _68;                        // _68
	int _6C;                        // _6C
	u8 _70[8];                      // _70
	u32 mBits;                      // _78 bit 0 set for kind 1 and 2 bit 1 from the pad
	u32 _7C;                        // _7C
	u32 _80;                        // _80
	TextReader mBlock;              // _84
	int _94;                        // _94
	u32 _98;                        // _98
	u32 _9C;                        // _9C
	TextGlyph* mExtra;              // _A0 a glyph that did not fit for the next line
	u32 _A4;                        // _A4
	u8 _A8;                         // _A8
	u8 _A9;                         // _A9
	u8 _AA[2];                      // _AA
};

extern "C" TSimpleText* createSimpleTextTimed(void* desc, int timer); // 0x8010FA90 held for timer frames

class TObject;
struct Vec3f;

extern "C" {
TSimpleText* createSimpleTextByKind(TObject*, void*, s8);
TSimpleText* createSimpleTextId2(TObject*, void*);
TSimpleText* createSimpleTextOnTask(int, int, const Vec3f*, const Vec3f*, const Vec3f*, void*);
}

extern "C" {
TSimpleText* createSimpleText(TObject*, int, int, const Vec3f*, const Vec3f*, const Vec3f*, void*);
}

extern "C" {
TSimpleText* createSimpleTextPlain(void*);
}

extern "C" {
extern const char* gTSimpleTextName;
}

#endif
