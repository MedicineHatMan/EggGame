#ifndef _TMODEGALLERY_H
#define _TMODEGALLERY_H
struct GjActor;

#include "stage/Stage.h"
struct NJS_TEXLIST;
#include "stage/SetManager.h"
#include "object/ObjCommon.h"

class TSimpleText;
class TAeObjGallery;
class TGalleryEggModel;
class TParticleGenerator;
struct NJS_OBJECT;

#define GALLERY_EGG_COUNT 0x48

/** One of the 72 gallery eggs, in gGalleryEggTable (0x80267E18). @size 0x14 */
struct GalleryEggInfo {
	s16 mFoundBit;     // _00 the save bit that says it was found
	s16 mModel;        // _02 the egg model it shows
	f32 mScale;        // _04
	f32 mSpeed;        // _08 motion frames per update
	int mNear;         // _0C shown in the near view
	const char* mArc;  // _10
};

/** What a gallery egg's archive holds */
struct GalleryEggAsset {
	NJS_OBJECT** mModels; // _00 a second model for eggs 0x3F and 0x40
	void** mMotions;      // _04 NJS_MOTIONs
	NJS_TEXLIST* mTexList;       // _08
	void* mGvm;           // _0C
	u8 _10[8];
	int mFxCount;         // _18 at most 5
	u16* mFx;             // _1C pairs the node then the particle it emits
};

/**
 * The egg turning in the gallery's view page. Its code is at 0x801BB234-0x801BB998, inside the range of
 * TObjClosedEgg.cpp. All names are ours.
 *
 * @size 0x68
 */
class GalleryEgg {
public:
	GalleryEgg();  // 0x801BB234
	~GalleryEgg(); // 0x801BB274

	void setEgg(int egg); // 0x801BB2BC
	void freeParts();     // 0x801BB458
	void start();         // 0x801BB4E8
	void stop();          // 0x801BB534
	void update();        // 0x801BB56C
	void draw();          // 0x801BB8BC

	GalleryEggAsset* mAsset; // _00
	SnowFlags mFlags;        // _04 bit 0 picks the near view
	u8 mShown;               // _06
	u8 _07;
	GjActor* mActor;            // _08 for a skinned model
	GjActor* mActor2;           // _0C the second model's
	f32 mScale;              // _10
	f32* mMtx;               // _14 where the egg sits
	f32 mLocal[12];          // _18
	int mMotion;             // _48
	f32 mFrame;              // _4C
	f32 mSpeed;              // _50
	TParticleGenerator* mPart[5]; // _54
};

extern "C" {
void loadGalleryEggArcs();        // 0x801BB10C
void releaseGalleryEggArcs();     // 0x801BB198
s16 galleryEggFoundBit(s8 egg);   // 0x801BB200
s16 galleryEggModel(s8 egg);      // 0x801BB218
}

/** The pages of the gallery */
enum GalleryState {
	GALLERY_INTRO = 0, // until the grid's clip ends
	GALLERY_PICK  = 1,
	GALLERY_OPEN  = 2, // until the view frame's clip ends
	GALLERY_VIEW  = 3,
	GALLERY_LEAVE = 4,
};

/**
 * The egg gallery: a grid of the 72 eggs, and a page that turns the one picked. k_egg_gallery.prd. Vtable
 * 0x80267B00. The class name is ours.
 *
 * @size 0x1FC
 */
class TModeGallery : public TMode {
public:
	TModeGallery();
	static TModeGallery* create(TModeAllocator* allocator); // 0x801B4E9C

	virtual ~TModeGallery();
	virtual void loadFiles();
	virtual int init();
	virtual int shutdown();
	virtual void update();

	void loadText();          // 0x801B516C
	void build();             // 0x801B51F4
	void step();              // 0x801B5464
	void drawEgg();           // 0x801B5568
	void enterPick();         // 0x801B55D8
	void enterView();         // 0x801B5624
	void updateIntro();       // 0x801B56F0
	void updatePick();        // 0x801B5734
	void updateOpen();        // 0x801B59D4
	void updateView();        // 0x801B5A18
	void updateLeave();       // 0x801B5B34
	int decided();            // 0x801B5B58
	int padCancel();          // 0x801B5B7C
	int padRepeat(u32 mask);  // 0x801B5BA8
	void hideTexts();         // 0x801B5C4C
	void makeName(int text);  // 0x801B5C88
	void showName(int text);  // 0x801B5CE0
	void hideName();          // 0x801B5D88
	void makeInfo(int text);  // 0x801B5DA4
	void showInfo(int text);  // 0x801B5DFC
	void hideInfo();          // 0x801B5EA4
	void makeHelp();          // 0x801B5EC0
	void showHelp();          // 0x801B5F38
	void hideHelp();          // 0x801B6030

	int mState;                     // _0C a GalleryState
	int mTimer;                     // _10
	int mCursor;                    // _14
	int mFound[GALLERY_EGG_COUNT];  // _18
	TGalleryEggModel* mModel;       // _138 the egg in the grid's corner
	int mSpin;                      // _13C
	GalleryEgg mView;               // _140
	SetRot mRot;                    // _1A8
	f32 mMtx[12];                   // _1B4
	TAeObjGallery* mGallery;        // _1E4
	TSimpleText* mName;             // _1E8
	TSimpleText* mInfo;             // _1EC
	TSimpleText* mHelp[2];          // _1F0
	int mDecide;                    // _1F8 A this frame
};

extern "C" {
extern u32 sModeGalleryClearColor;
}

#endif
