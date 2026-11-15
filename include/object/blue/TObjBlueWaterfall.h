#ifndef _TOBJBLUEWATERFALL_H
#define _TOBJBLUEWATERFALL_H
struct NJS_OBJECT;
struct CharaColli;

#include "engine/DrawTask.h"
#include "stage/SetManager.h"

extern "C" void objHeapFree(void*, void*);

/**
 * A placed waterfall. The set record supplies a shared alpha, an indirect offset and two scroll speeds.
 * One task scrolls the water model. The other draws a second model. Out of range it deletes itself.
 *
 * @size 0x88
 */
class TObjBlueWaterfall : public TObjAction {
public:
	TObjBlueWaterfall(TObject* parent, SetDesc* desc);
	virtual ~TObjBlueWaterfall();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                                              // _0C
	virtual void vfn_10();                                            // _10
	virtual void render();                                            // _1C
	virtual Vec3f* getPos();                                          // _20
	virtual int getRotY();                                            // _28
	virtual int getKind();                                            // _2C
	virtual void initFromDesc(SetDesc* desc);                         // _30
	virtual void debugPrint(void*);                                   // _34
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58
	virtual SetLink* getLink();                                       // _6C

	// The second draw task addDrawTask passes this object
	static void drawMistTask(TObjBlueWaterfall* self);

	void setup(); // 0x800BC740 the collider the models and the scroll
	void scroll();
	void drawMist();
	void drawWater();

	NJS_OBJECT* mWater;   // _30 gjDrawObject Filled by setup from the ge models
	void* mPlain;         // _34 drawn by drawObjectDefault Name is a guess
	void* mTex;           // _38
	DrawTask* mWaterTask; // _3C priority 0x2A drawObjectTask
	DrawTask* mPlainTask; // _40 priority 0x21
	CharaColli* mShape;   // _44
	SetLinkMember mLink;  // _48
	Vec3f mPos;           // _54
	SetRot mRot;          // _60
	int mMatId;           // _6C passed to genTexLoad Name is a guess
	f32 mScroll;          // _70
	f32 mScrollSpeed;     // _74
	f32 mIndOffset;       // _78
	f32 mIndScroll;       // _7C
	f32 mIndSpeed;        // _80
	int mStopped;         // _84 non-zero skips the scroll and leaves the link
};

extern "C" void objHeapFree(void*, void*);

class SetDesc;
class TObject;

extern "C" {
TObject* createTObjBlueWaterfall(TObject*, SetDesc*);
}

extern "C" {
extern u32 sWaterfallTevStage;
extern u32 sWaterfallColor;
extern u32 sBlueWaterfallWhite;
}

void initBlueWaterfallSet();
void termBlueWaterfallSet();

#endif
