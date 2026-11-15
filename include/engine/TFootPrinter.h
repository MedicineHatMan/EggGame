#ifndef _ENGINE_TFOOTPRINTER_H_
#define _ENGINE_TFOOTPRINTER_H_
struct TObject;
struct FootCorner;
#include "stage/Stage.h"
struct NJS_TEXLIST;
struct FootPrint;
struct TFootPrinter;

#include "core/types.h"

struct Vec3f;

extern "C" {
void footPrintAdd(const Vec3f*, const Vec3f*, int, f32);
void* footPrinterGet();
}

extern "C" {
void* footPrinterCreate();
}

/**
 * @size 0x38
 */
struct FootPrint {
	u16 mLife;        // _00
	u16 mKind;        // _02
	Vec3f mCorner[4]; // _04
	FootPrint* mNext; // _34
};

/**
 * @size 0x382C
 */
class TFootPrinter : public TObject {
public:
	TFootPrinter(TObject* parent);
	virtual ~TFootPrinter();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();

	void add(const Vec3f* pos, const Vec3f* dir, int rotY, int kind);
	void initRing();

	NJS_TEXLIST* mTexList;    // _20
	DrawTask* mTask;          // _24
	FootPrint mPrints[0x100]; // _28
	FootPrint* mNext;         // _3828
};

/**
 * A corner of the print's square, as model space and as texture space.
 *
 * @size 0x18
 */
struct FootCorner {
	Vec3f mPos;  // _00
	f32 mU;      // _0C
	f32 mV;      // _10
	u32 mColour; // _14
};

extern "C" {
extern const char* gTFootPrinterName;
extern TObject* sFootPrinterObj;
extern const FootCorner sFootCorners[4];
}

#endif
