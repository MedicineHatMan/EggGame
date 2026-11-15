#include "engine/Movie.h"
#include "io/ResFile.h"
#include "io/Resource.h"
#include "io/FileReader.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "Dolphin/OS/OSCache.h"
#include "core/dolphin.h"
#include "Ninja/ninja.h"
#include "Dolphin/vi.h"
#include "core/SdkCalls.h"

struct MovieModule {
	u8 _00[0x20];
	u32 mBssSize;          // _20
	u8 _24[0x10];
	void (*mProlog)();     // _34
	void (*mEpilog)();     // _38
};

/** What the decoder starts with. @size 0x20 */
struct MovieConfig {
	f32 mFps;    // _00
	int _04;     // _04
	int _08;     // _08
	int _0C;     // _0C
	u8 _10[0x10];
};

/** What a player is made from. @size 0x30 */
struct MoviePlayerDesc {
	int _00;       // _00
	int mBitRate;  // _04
	int mWidth;    // _08
	int mHeight;   // _0C
	int _10;       // _10
	int _14;       // _14
	u8* mWork;     // _18
	u32 mWorkSize; // _1C
	int _20;       // _20
	u8 _24[0xC];
};

/** One decoded frame. @size 0x54 */
struct MovieFrame {
	u32 mValid; // _00
	u8 _04[0x40];
};

static MoviePlayer* sMoviePlayer;
static TMovie* sMovie;
static BinFile* sMovieFile;
static MovieModule* sMovieModule;
static u8* sMovieBss;

// Filled in by the module's prolog
void (*sMovieInit)(MovieConfig* config);
void (*sMovieQuit)();
u32 (*sMovieGetWorkSize)(MoviePlayerDesc* desc);
MoviePlayer* (*sMovieCreate)(MoviePlayerDesc* desc);
void (*sMovieGetDecoded)(MoviePlayer* player, MovieFrame* frame);
void (*sMovieReleaseDecoded)(MoviePlayer* player);
// The module fills in many more entries than the game ever calls
void (*sMovieEntry7C)();
void (*sMovieEntry80)();
void (*sMovieEntry84)();
void (*sMovieEntry88)();
void (*sMovieEntry8C)();
void (*sMovieEntry90)();
int (*sMovieIsPaused)(MoviePlayer* player);
void (*sMovieEntry98)();
void (*sMovieEntry9C)();
void (*sMovieEntryA0)();
void (*sMovieEntryA4)();
void (*sMovieEntryA8)();
void (*sMovieEntryAC)();
void (*sMovieEntryB0)();
void (*sMovieEntryB4)();
void (*sMovieEntryB8)();
void (*sMovieEntryBC)();
void (*sMovieEntryC0)();
void (*sMovieEntryC4)();
void (*sMovieEntryC8)();
void (*sMovieEntryCC)();
void (*sMovieEntryD0)();
void (*sMovieEntryD4)();
void (*sMovieEntryD8)();
void (*sMovieEntryDC)();
void (*sMovieEntryE0)();
void (*sMovieEntryE4)();
void (*sMovieEntryE8)();
void (*sMovieEntryEC)();
void (*sMovieEntryF0)();
void (*sMovieConvert)(MoviePlayer* player, MovieFrame* frame, u8* out);
void (*sMovieEntryF8)();
void (*sMovieEntryFC)();
void (*sMovieSetSize)(MoviePlayer* player, int width, int height);
void (*sMovieEntry104)();
void (*sMovieEntry108)();
void (*sMovieEntry10C)();
void (*sMovieEntry110)();
void (*sMovieEntry114)();
void (*sMovieEntry118)();
void (*sMovieEntry11C)();
void (*sMovieEntry120)();
void (*sMovieEntry124)();
void (*sMovieEntry128)();
void (*sMovieEntry12C)();
void (*sMovieEntry130)();
void (*sMovieEntry134)();

// 0x801BE41C
static void movieLoaded(int event, int id, void* target, void* user)
{
	if (event == 2) {
		sMovieModule = (MovieModule*)sMovieFile->mData;
		sMovieBss    = new u8[sMovieModule->mBssSize];
		OSLink((OSModuleInfo*)sMovieModule, sMovieBss);
		sMovieModule->mProlog();
	}
}

// 0x801BE474
void movieLoad()
{
	BinFile* file = new BinFile;
	sMovieModule = 0;
	sMovieBss    = 0;
	sMovieFile   = file;
	FileReader::get()->read("movie_module.rel", sMovieFile, movieLoaded, 0);
}

// 0x801BE4D8
void movieFree()
{
	if (sMovieModule != 0) {
		sMovieModule->mEpilog();
		OSUnlink((OSModuleInfo*)sMovieModule);
		if (sMovieBss != 0) {
			delete[] sMovieBss;
		}
	}
	sMovieModule = 0;
	sMovieBss    = 0;
	releaseResource(sMovieFile);
	sMovieFile = 0;
}

// 0x801BE540
void movieOpen(int width, int height)
{
	if (sMovie != 0) {
		delete sMovie;
		sMovie = 0;
	}
	sMovie = new TMovie(width, height);
}

// 0x801BE5B0
void movieStart()
{
	movieOpen(640, 448);
}

// 0x801BE5D8
void movieStop()
{
	if (sMovie != 0) {
		delete sMovie;
		sMovie = 0;
	}
}

// 0x801BE610
// 1 when a new frame was decoded
int movieDecode()
{
	if (sMovie != 0) {
		return sMovie->decode();
	}
	return 0;
}

// 0x801BE644
u8* movieGetFrame()
{
	if (sMovie != 0) {
		return sMovie->getFrame();
	}
	return 0;
}

// 0x801BE678
void movieCheckDisc()
{
	if (sMovie != 0) {
		sMovie->checkDisc();
	}
}

// 0x801BE6A4
void moviePlay(const char* file)
{
	if (sMovie != 0) {
		sMovie->play(file);
	}
}

// 0x801BE6D8
void moviePause()
{
	if (sMovie != 0) {
		sMovie->pause();
	}
}

// 0x801BE704
int movieIsDone()
{
	if (sMovie != 0) {
		return sMovie->isDone();
	}
	return 1;
}

// 0x801BE738
TMovie::TMovie(int width, int height)
{
	mWork      = 0;
	_04        = 0;
	_08        = 0;
	mWidth     = width;
	mHeight    = height;
	mFrameSize = GXGetTexBufferSize(640, 448, 6, GX_FALSE, 0); // GX_TF_RGBA8
	mFrame     = new u8[mFrameSize];
	memset(mFrame, 0, mFrameSize);
	DCFlushRange(mFrame, mFrameSize);
	open();
}

// 0x801BE7C8
TMovie::~TMovie()
{
	close();
	if (mWork != 0) {
		delete[] mWork;
		mWork = 0;
	}
	if (mFrame != 0) {
		delete[] mFrame;
		mFrame = 0;
	}
	if (sMovie == this) {
		sMovie = 0;
	}
}

// 0x801BE854
void TMovie::open()
{
	if (sMovieModule == 0) {
		return;
	}

	MovieConfig config;
	memset(&config, 0, sizeof(config));
	config.mFps = VIGetTvFormat() != 1 ? 59.94f : 50.0f; // 1 = VI_PAL
	config._04  = 1;
	config._08  = 1;
	config._0C  = 0;
	sMovieInit(&config);

	MoviePlayerDesc desc;
	memset(&desc, 0, sizeof(desc));
	desc._20       = 0;
	desc._00       = 1;
	desc.mBitRate  = 6000000;
	desc.mWidth    = 640;
	desc.mHeight   = 480;
	desc._10       = 3;
	desc._14       = 1;
	desc.mWorkSize = sMovieGetWorkSize(&desc);
	mWork          = new u8[desc.mWorkSize];
	desc.mWork     = mWork;
	sMoviePlayer   = sMovieCreate(&desc);
}

// 0x801BE958
void TMovie::play(const char* file)
{
	if (sMovieModule != 0) {
		memset(mFrame, 0, mFrameSize);
		DCFlushRange(mFrame, mFrameSize);
		GXInvalidateTexAll();
		sMoviePlayer->mFuncs->play(sMoviePlayer, file);
	}
}

// 0x801BE9C8
void TMovie::pause()
{
	if (sMovieModule != 0) {
		sMoviePlayer->mFuncs->pause(sMoviePlayer);
	}
}

// 0x801BEA04
void TMovie::close()
{
	if (sMovieModule != 0) {
		sMoviePlayer->mFuncs->stop(sMoviePlayer);
		sMovieQuit();
	}
}

// 0x801BEA4C
u8* TMovie::getFrame()
{
	return mFrame;
}

// 0x801BEA54
// Holds the movie while the disc can't be read
void TMovie::checkDisc()
{
	if (sMovieModule == 0) {
		return;
	}
	switch (DVDGetDriveStatus()) {
	case -1: // DVD_STATE_FATAL_ERROR
	case 4:  // DVD_STATE_NO_DISK
	case 5:  // DVD_STATE_COVER_OPEN
	case 6:  // DVD_STATE_WRONG_DISK
	case 11: // DVD_STATE_RETRY
		if (sMovieIsPaused(sMoviePlayer) == 0) {
			sMoviePlayer->mFuncs->setPaused(sMoviePlayer, 1);
		}
		break;
	default:
		if (sMovieIsPaused(sMoviePlayer) != 0) {
			sMoviePlayer->mFuncs->setPaused(sMoviePlayer, 0);
		}
		break;
	}
}

// 0x801BEB10
int TMovie::isDone()
{
	if (sMovieModule != 0) {
		return sMoviePlayer->mFuncs->getState(sMoviePlayer) == 3;
	}
	return 1;
}

// 0x801BEB60
// Once a frame on the first field
int TMovie::decode()
{
	MovieFrame frame;
	if (VIGetNextField() == 1) {
		sMovieGetDecoded(sMoviePlayer, &frame);
		if (frame.mValid != 0) {
			sMovieSetSize(sMoviePlayer, mWidth, mHeight);
			sMovieConvert(sMoviePlayer, &frame, mFrame);
			sMovieReleaseDecoded(sMoviePlayer);
			DCFlushRange(mFrame, mFrameSize);
			return 1;
		}
	}
	return 0;
}
