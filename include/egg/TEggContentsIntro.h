#ifndef _TEGGCONTENTSINTRO_H
#define _TEGGCONTENTSINTRO_H
struct TObject;

#include "ui/TSimpleText.h"

extern "C" void objHeapFree(void*, void*);

/**
 * Shows one contents line. Index is 1-based into the message table. mDone is set once the text
 * reaches state 3.
 *
 * @size 0x28
 */
class TEggContentsIntro : public TObject {
public:
	TEggContentsIntro(TObject* parent, int index);
	virtual ~TEggContentsIntro();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw(); // _0C

	TSimpleText* mText; // _20
	int mDone;          // _24
};

extern "C" {
TEggContentsIntro* createTEggContentsIntro(TObject*, int);
}

extern "C" {
extern const char* gTEggContentsIntroName;
}

#endif
