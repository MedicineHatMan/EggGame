#include "mode/TModeGallery.h"
#include "ui/TextVec.h"
#include "io/LangFile.h"
#include "io/ResFile.h"
#include "io/Resource.h"
#include "ui/TAeObjGallery.h"
#include "ui/TGalleryEggModel.h"
#include "ui/TSimpleText.h"
#include "core/TMainTask.h"
#include "core/Pad.h"
#include "Ninja/ninja.h"
#include "engine/ShadowTex.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "camera/TCamera.h"
#include "camera/TCameraCtrl.h"
#include "engine/Viewport.h"
#include "engine/DrawTask.h"
#include "sound/SoundCtrl.h"
#include "ui/Font.h"
#include "engine/TFogCtrl.h"
#include "camera/TCamEditor.h"
#include "engine/ParticleBank.h"
#include "ui/NowLoading.h"
#include "object/TObjLauncher.h"
#include "ui/TFade.h"
#include "io/SaveData.h"
#include "core/Mem.h"
#include "egg/EggCreature.h"
#include "animal/AniMount.h"
#include "core/MathUtil.h"
#include "stage/Stage.h"

BinFile* sGalleryMes; // 0x802C5AC0
int sGalleryMesCount; // 0x802C5AC4
int* sGalleryMesText; // 0x802C5AC8

#define GALLERY_STRING(i) ((void*)sGalleryMesText[i])

inline Vec3f vec3f(f32 x, f32 y, f32 z)
{
	Vec3f v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

// Built by the static initializer at 0x801B6064
static TextVec sGalleryInfoGlyph = textVec(0.91f, 0.91f, 0.91f);   // 0x80286830
static TextVec sGalleryInfoBox   = textVec(264.0f, 66.0f, 0.0f);   // 0x8028683C
static TextVec sGalleryInfoAt    = textVec(430.0f, 345.0f, -2.0f); // 0x80286848
static TextVec sGalleryNameGlyph = textVec(0.91f, 0.91f, 0.91f);   // 0x80286854
static TextVec sGalleryNameBox   = textVec(264.0f, 22.0f, 0.0f);   // 0x80286860
static TextVec sGalleryNameAt    = textVec(430.0f, 306.0f, -2.0f); // 0x8028686C
static TextVec sGalleryHelpAt0   = textVec(565.0f, 38.0f, -2.0f);  // 0x80286878
static TextVec sGalleryHelpAt1   = textVec(565.0f, 75.0f, -2.0f);  // 0x80286884
static TextVec sGalleryEggPos    = textVec(20.0f, 1.5f, 64.0f);    // 0x80286890
// These two are built in place by a constructor
struct GalleryVec : public Vec3f {
	inline GalleryVec(f32 newX, f32 newY, f32 newZ)
	{
		x = newX;
		y = newY;
		z = newZ;
	}
};
static GalleryVec sGalleryNearPos(-12.0f, 0.0f, 65.0f); // 0x8028689C
static GalleryVec sGalleryFarPos(-12.0f, -5.0f, 65.0f); // 0x802868A8

struct GalleryMtx {
	f32 m[12];
};

inline void hideObj(TObject* obj)
{
	if (obj != 0) {
		obj->mFlags |= 0x10;
	}
}

// 0x801B4E9C
TModeGallery* TModeGallery::create(TModeAllocator* allocator)
{
	return new (allocator) TModeGallery;
}

// 0x801B4ED4
void TModeGallery::loadFiles()
{
	nowLoadingDisable();
	loadPrd("k_egg_gallery.prd");
}

// 0x801B4F10
int TModeGallery::init()
{
	GXSetCopyClear(*(GXColor*)&sModeGalleryClearColor, 0xFFFFFF);
	camCtrlInit();
	viewportInit();
	initDrawTasks();
	fogLoad();
	fogOff();
	spritesInit();
	loadParticles("ptcl2_blue.bin");
	loadText();
	loadGalleryEggs();
	loadGallery();
	loadGalleryEggArcs();
	gjSetAmbient(0.5f, 0.5f, 0.5f);
	gjSetLightSwitch(0, 1);
	gjSetLightVector(0, 0.5f, -1.0f, -0.5f);
	gjSetLightColor(0, 1.0f, 1.0f, 1.0f);
	gjLoadLight(0);
	objClearLayers(&gMainTask);
	createCameraCtrl();
	camSetDrawView(0);
	setViewport(0);
	fontCreateTask();
	build();
	bgmPlay(1);
	return 1;
}

// 0x801B500C
int TModeGallery::shutdown()
{
	objClearLayers(&gMainTask);
	releaseGalleryEggArcs();
	freeGallery();
	freeGalleryEggs();
	if (sGalleryMes != 0) {
		releaseResource(sGalleryMes);
		sGalleryMes = 0;
	}
	freeParticles();
	spritesTerm();
	fogUnload();
	termDrawTasks();
	viewportTerm();
	camCtrlTerm();
	bgmStop();
	return 1;
}

// 0x801B5078
TModeGallery::TModeGallery()
{
	mState  = GALLERY_INTRO;
	mTimer  = 0;
	mCursor = 0;
	// The original clears the word past mFound each time round so mFound is left as it was
	for (int i = 0; i < GALLERY_EGG_COUNT; i++) {
		mFound[GALLERY_EGG_COUNT] = 0;
	}
	mModel   = 0;
	mSpin    = 0;
	mGallery = 0;
	mName    = 0;
	mInfo    = 0;
	mHelp[0] = 0;
	mHelp[1] = 0;
	mDecide  = 0;
}

// 0x801B5110
TModeGallery::~TModeGallery()
{
}

// 0x801B516C
void TModeGallery::loadText()
{
	u32* data;
	u32 text;
	int i;

	loadSizedResource(getLangFileName("mes_gallery.bin"), &sGalleryMes);
	data = (u32*)sGalleryMes->mData;
	text = *data;
	text += (u32)data++;
	u32 size         = *data++;
	sGalleryMesText  = (int*)data;
	sGalleryMesCount = size >> 2;
	for (i = 0; i < sGalleryMesCount; i++) {
		sGalleryMesText[i] += text + 8;
	}
}

// 0x801B51F4
void TModeGallery::build()
{
	makeName(2);
	makeInfo(0x4A);
	makeHelp();
	hideTexts();
	mGallery = createGallery();
	mModel   = createTGalleryEggModel(gTL_05, 0x2D);
	if (mModel != 0) {
		f32 mtx[12];
		mtxSetTransV(mtx, &sGalleryEggPos);
		mtxRotateY(mtx, mtx, mSpin);
		*(GalleryMtx*)mModel->mMtx = *(GalleryMtx*)mtx;
		mModel->mFlags |= 0x10;
	}
	int found = 0;
	for (int i = 0; i < GALLERY_EGG_COUNT; i++) {
		if (saveGetKeyBit(galleryEggFoundBit(i)) != 0) {
			mFound[i] = 1;
			if (mGallery != 0) {
				mGallery->showEgg(i);
			}
			found++;
		} else {
			mFound[i] = 0;
		}
	}
	if (mGallery != 0) {
		mGallery->setCount(found);
	}
}

// 0x801B5338
// Waits for the GPU to reach this point
static void waitDrawSync()
{
	static u16 sToken = 0;
	GXSetDrawSync(sToken);
	while (sToken != GXReadDrawSync()) { }
	sToken++;
}

// 0x801B5394
void TModeGallery::update()
{
	spriteClear();
	gMainTask.draw();
	step();
	gjBeforeRender();
	gjSetRenderMode(3);
	GXSetCullMode(GX_CULL_NONE);
	setViewport(0);
	camSetDrawView(0);
	gMainTask.vfn_10();
	runDrawTasks(0x3C);
	runDrawTasks(0x21);
	runDrawTasks(0x3E);
	runDrawTasks(0x3F);
	waitDrawSync();
	drawEgg();
	waitDrawSync();
	GXSetCullMode(GX_CULL_NONE);
	spriteDrawAll();
	waitDrawSync();
	memDebugDrawBegin();
	memShowStats();
	memDebugDrawEnd();
	frameTickerGet()->update();
	gjPrintFlush();
	gjDoneRender();
	endFrameSwap();
}

// 0x801B5464
void TModeGallery::step()
{
	mDecide = padGet(0)->mPressed & 0x100;
	mSpin += 0x100;
	mSpin &= 0xFFFF;
	f32 mtx[12];
	mtxSetTransV(mtx, &sGalleryEggPos);
	mtxRotateY(mtx, mtx, mSpin);
	*(GalleryMtx*)mModel->mMtx = *(GalleryMtx*)mtx;
	switch (mState) {
	case GALLERY_INTRO:
		updateIntro();
		break;
	case GALLERY_PICK:
		updatePick();
		break;
	case GALLERY_OPEN:
		updateOpen();
		break;
	case GALLERY_VIEW:
		updateView();
		break;
	case GALLERY_LEAVE:
		updateLeave();
		break;
	}
}

// 0x801B5568
// The two statics are never read
void TModeGallery::drawEgg()
{
	static int sTilt = 0xE800;
	static int sTurn = 0;
	switch (mState) {
	case GALLERY_VIEW:
		mView.draw();
		break;
	}
}

// 0x801B55D8
void TModeGallery::enterPick()
{
	showHelp();
	if (mModel != 0) {
		mModel->mFlags &= ~0x10;
	}
	mState = GALLERY_PICK;
}

// 0x801B5624
void TModeGallery::enterView()
{
	showName(mCursor + 2);
	showInfo(mCursor + 0x4A);
	mView.setEgg(mCursor);
	if (mView.mFlags.mBits & 1) {
		mtxSetTransV(mMtx, &sGalleryNearPos);
	} else {
		mtxSetTransV(mMtx, &sGalleryFarPos);
	}
	mRot.x = 0;
	mRot.y = 0;
	mRot.z = 0;
	mRot.y = 0x16800;
	mtxRotateY(mMtx, mMtx, mRot.y);
	mView.mMtx = mMtx;
	mView.start();
	mView.update();
	mState = GALLERY_VIEW;
}

// 0x801B56F0
void TModeGallery::updateIntro()
{
	if (mGallery != 0) {
		if ((int)mGallery->mGridDone != 0) {
			enterPick();
		}
	} else {
		_08 = 0;
	}
}

// 0x801B5734
// The grid is 8 eggs a row
void TModeGallery::updatePick()
{
	if (decided() != 0) {
		if (mFound[mCursor] != 0) {
			mGallery->openView();
			mGallery->markMissing();
			sePlay(0x1C);
			mState = GALLERY_OPEN;
		} else {
			sePlay(0x21);
		}
	} else if (padCancel() != 0) {
		mGallery->close();
		hideTexts();
		mTimer = 60;
		fadeToWhite();
		sePlay(0x20);
		mState = GALLERY_LEAVE;
	} else {
		if (padRepeat(2) != 0) {
			mCursor++;
			mCursor = mCursor >= GALLERY_EGG_COUNT ? 0 : mCursor;
			sePlay(0x1B);
		} else if (padRepeat(1) != 0) {
			mCursor--;
			{
				int v = mCursor;
				if (v < 0) {
					v = GALLERY_EGG_COUNT - 1;
				}
				mCursor = v;
			}
			sePlay(0x1B);
		} else if (padRepeat(8) != 0) {
			if (mCursor < 8) {
				mCursor += 0x40;
			} else if (mCursor < 0x18) {
				mCursor -= 8;
			} else if (mCursor < 0x1C) {
				mCursor += 0x24;
			} else {
				mCursor -= 0xC;
			}
			sePlay(0x1B);
		} else if (padRepeat(4) != 0) {
			if (mCursor > 0x3F) {
				mCursor -= 0x40;
			} else if (mCursor > 0x3B) {
				mCursor -= 0x24;
			} else if (mCursor > 0xF) {
				mCursor += 0xC;
			} else {
				mCursor += 8;
			}
			sePlay(0x1B);
		}
		s16 bit = galleryEggFoundBit(mCursor);
		if (mGallery != 0) {
			mGallery->setCursor(mCursor);
			if (saveGetKeyBit(bit) != 0) {
				mGallery->markFound();
			} else {
				mGallery->markMissing();
			}
		}
		if (mModel != 0) {
			if (saveGetKeyBit(bit) != 0) {
				mModel->mEgg = galleryEggModel(mCursor);
			} else {
				mModel->mEgg = 0x2D;
			}
		}
	}
}

// 0x801B59D4
void TModeGallery::updateOpen()
{
	if (mGallery != 0) {
		if ((int)mGallery->mFrameDone != 0) {
			enterView();
		}
	} else {
		_08 = 0;
	}
}

// 0x801B5A18
// The stick turns the egg
void TModeGallery::updateView()
{
	PadState* pad = padGet(0);
	f32 x         = pad->mLength * njCos(pad->mAngle);
	f32 y         = pad->mLength * njSin(pad->mAngle);
	mRot.y += (int)(1024.0f * x);
	if (mView.mFlags.mBits & 1) {
		mtxSetTransV(mMtx, &sGalleryNearPos);
	} else {
		mtxSetTransV(mMtx, &sGalleryFarPos);
	}
	mtxRotateY(mMtx, mMtx, mRot.y);
	mtxRotateX(mMtx, mMtx, mRot.x);
	mView.update();
	if (padCancel() != 0) {
		mView.stop();
		if (mGallery != 0) {
			mGallery->closeView();
		}
		hideName();
		hideInfo();
		sePlay(0x20);
		mState = GALLERY_PICK;
	}
}

// 0x801B5B34
void TModeGallery::updateLeave()
{
	if (mTimer == 0) {
		_08 = 0;
		return;
	}
	mTimer--;
}

// 0x801B5B58
int TModeGallery::decided()
{
	if (mDecide != 0) {
		mDecide = 0;
		return 1;
	}
	return 0;
}

// 0x801B5B7C
int TModeGallery::padCancel()
{
	return padGet(0)->mPressed & 0x200;
}

// 0x801B5BA8
int TModeGallery::padRepeat(u32 mask)
{
	if (mask & 0x200004) {
		mask |= 0x200004;
	} else if (mask & 0x100008) {
		mask |= 0x100008;
	} else if (mask & 0x800002) {
		mask |= 0x800002;
	} else if (mask & 0x400001) {
		mask |= 0x400001;
	}
	return mask & padGet(0)->mRepeat;
}

// 0x801B5C4C
void TModeGallery::hideTexts()
{
	hideName();
	hideInfo();
	hideHelp();
}

// 0x801B5C88
void TModeGallery::makeName(int text)
{
	mName = createSimpleTextOnTask(1, -1, &sGalleryNameAt, &sGalleryNameBox, &sGalleryNameGlyph, GALLERY_STRING(text));
}

// 0x801B5CE0
void TModeGallery::showName(int text)
{
	Vec3f box = vec3f(0.0f, 24.0f, 0.0f);
	if (mName != 0) {
		mName->resetPage(GALLERY_STRING(text));
		TSimpleText* name = mName;
		name->mBox.x      = box.x;
		name->mBox.y      = box.y;
		name->mBox.z      = box.z;
		mName->_A8        = 1;
		mName->mFlags &= ~0x10;
	}
}

// 0x801B5D88
void TModeGallery::hideName()
{
	hideObj(mName);
}

// 0x801B5DA4
void TModeGallery::makeInfo(int text)
{
	mInfo = createSimpleTextOnTask(1, -1, &sGalleryInfoAt, &sGalleryInfoBox, &sGalleryInfoGlyph, GALLERY_STRING(text));
}

// 0x801B5DFC
void TModeGallery::showInfo(int text)
{
	Vec3f box = vec3f(0.0f, 66.0f, 0.0f);
	if (mInfo != 0) {
		mInfo->resetPage(GALLERY_STRING(text));
		TSimpleText* info = mInfo;
		info->mBox.x      = box.x;
		info->mBox.y      = box.y;
		info->mBox.z      = box.z;
		mInfo->_A8        = 1;
		mInfo->mFlags &= ~0x10;
	}
}

// 0x801B5EA4
void TModeGallery::hideInfo()
{
	hideObj(mInfo);
}

// 0x801B5EC0
void TModeGallery::makeHelp()
{
	mHelp[0] = createSimpleTextOnTask(1, -1, &sGalleryInfoAt, &sGalleryInfoBox, &sGalleryInfoGlyph, GALLERY_STRING(0));
	mHelp[1] = createSimpleTextOnTask(1, -1, &sGalleryInfoAt, &sGalleryInfoBox, &sGalleryInfoGlyph, GALLERY_STRING(1));
}

// 0x801B5F38
static inline Vec3f galleryVec(f32 x, f32 y, f32 z)
{
	Vec3f v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

void TModeGallery::showHelp()
{
	Vec3f box = galleryVec(0.0f, 24.0f, 0.0f);
	if (mHelp[0] != 0) {
		mHelp[0]->mFlags &= ~0x10;
		mHelp[0]->mAt  = sGalleryHelpAt0;
		mHelp[0]->mBox = box;
		mHelp[0]->_A8  = 1;
	}
	if (mHelp[1] != 0) {
		mHelp[1]->mFlags &= ~0x10;
		mHelp[1]->mAt  = sGalleryHelpAt1;
		mHelp[1]->mBox = box;
		mHelp[1]->_A8  = 1;
	}
}

// 0x801B6030
void TModeGallery::hideHelp()
{
	hideObj(mHelp[0]);
	hideObj(mHelp[1]);
}
