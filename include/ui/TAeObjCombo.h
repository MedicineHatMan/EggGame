#ifndef _TAEOBJCOMBO_H
#define _TAEOBJCOMBO_H
struct TArc;

#include "ui/TAeLayer.h"
struct DrawTask;

/**
 * Combo counter. Layer ids 0x85, 0x4C and 0x5A, plus three copies of id 0x58.
 * The draw task is on list 0x3D and is stored at _4C. draw() steps the layers and
 * bumps the backdrop after 30 frames. render() draws the digits from _50.
 *
 * @size 0x58
 */
class TAeObjCombo : public TAeObject {
public:
	TAeObjCombo(TObject* parent, void* data, void* assets);
	virtual ~TAeObjCombo();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void render(); // _1C

	void showRank(int rank, int word); // 0x801CAC30 rank 1 to 4 word 0 to 9
	void showCount(int count);         // 0x801CADB4 in the next of the three counters
	void fadeCounts();                 // 0x801CAF4C

	TAeLayer* mFrame;     // _34 holds the rank and the word
	TAeLayer* mRank;      // _38
	TAeLayer* mWord;      // _3C
	TAeLayer* mCounts[3]; // _40
	DrawTask* mTask;      // _4C
	int mNextCount;       // _50 the counter showCount uses next
	int mFrameTime;       // _54
};

class TObject;

extern "C" {
TAeObjCombo* createCombo(TObject*);
void freeCombo();
void loadCombo();
}

#include "Ninja/njtypes.h"
extern "C" {
extern const char* gTAeObjComboName;
extern const char sAeComboArcName[];
extern TArc* sComboArc;
extern void* sComboAe;
extern NJS_TEXLIST* sComboTexList;
}

#endif
