#ifndef _TAEOBJGALLERY_H
#define _TAEOBJGALLERY_H
struct TArc;

#include "ui/TAeLayer.h"
struct DrawTask;

/**
 * Gallery screen. Four layers and a draw task on list 0x3C. The first layer is linked
 * to a long id table. draw() steps the layers and records when the first two finish.
 * render() draws the first three. The destructor does not remove the task.
 *
 * @size 0x54
 */
class TAeObjGallery : public TAeObject {
public:
	TAeObjGallery(TObject* parent, void* data, void* assets);
	virtual ~TAeObjGallery();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void render(); // _1C

	// TModeGallery's calls
	void showEgg(int egg);    // 0x801B4C14 puts egg's cell in the grid
	void setCursor(int egg);  // 0x801B4C58 moves the cursor clip to egg
	void setCount(int count); // 0x801B4CDC the number found 0 to 72
	void close();             // 0x801B4D30
	void openView();          // 0x801B4D84
	void closeView();         // 0x801B4DD8
	void markFound();         // 0x801B4E2C
	void markMissing();       // 0x801B4E64

	DrawTask* mTask;    // _34
	TAeLayer* _38;  // _38 the grid
	TAeLayer* _3C;  // _3C the view frame
	TAeLayer* _40;  // _40 the found mark
	TAeLayer* _44;  // _44 the cursor
	int mCursor;    // _48
	u32 mGridDone;  // _4C set when _38's clip ends
	u32 mFrameDone; // _50 set when _3C's clip ends
};

extern "C" {
void loadGallery();             // loads ae_gallery.arc
void freeGallery();             // frees it
TAeObjGallery* createGallery();   // creates the screen null without the archive
}

#include "Ninja/njtypes.h"
extern "C" {
extern const char* gTAeObjGalleryName;
extern TArc* sGalleryArc;
extern void* sGalleryAe;
extern NJS_TEXLIST* sGalleryTexList;
}

#endif
