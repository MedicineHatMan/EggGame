#ifndef _MOVIE_H
#define _MOVIE_H

#include "core/types.h"

enum MovieId {
	MOVIE_DEMO_MOVIE  = 0, // the first attract demo
	MOVIE_DEMO_PLAY   = 1, // the second
	MOVIE_OPENING     = 2,
	MOVIE_ENDING      = 3,
	MOVIE_BOSS_CLEAR1 = 4,
	MOVIE_BOSS_CLEAR2 = 5,
	MOVIE_BOSS_CLEAR3 = 6,
	MOVIE_BOSS_CLEAR4 = 7,
	MOVIE_BOSS_CLEAR5 = 8,
	MOVIE_BOSS_CLEAR6 = 9,
	MOVIE_BOSS_CLEAR7 = 10,
	MOVIE_STAFFROLL   = 11,
	MOVIE_COUNT       = 12,
};

/** The decoder's player object Only the slots used here have names */
struct MoviePlayer;

// CRI's C-style function table every call passes the player back in by hand
struct MoviePlayerFuncs {
	void* _00[5];
	void (*stop)(MoviePlayer* player);                   // _14
	void (*play)(MoviePlayer* player, const char* file); // _18 a .sfd on the disc
	void (*pause)(MoviePlayer* player);                  // _1C
	int (*getState)(MoviePlayer* player);                // _20 3 once it has ended
	void* _24;
	void (*setPaused)(MoviePlayer* player, int paused); // _28 while the disc can't be read
};

struct MoviePlayer {
	MoviePlayerFuncs* mFuncs; // _00
};

/**
 * Decodes the open movie into a 640 by 448 RGBA8 texture.
 * @size 0x1C
 */
class TMovie {
public:
	TMovie(int width, int height);
	~TMovie();

	void open();
	void play(const char* file);
	void pause();
	void close();
	u8* getFrame();
	void checkDisc();
	int isDone();
	int decode();

	u8* mWork;      // _00 the decoder's memory
	u32 _04;        // _04
	u32 _08;        // _08
	int mWidth;     // _0C
	int mHeight;    // _10
	u32 mFrameSize; // _14
	u8* mFrame;     // _18
};

extern "C" {
void movieLoad();
void movieFree();
void movieOpen(int width, int height);
void movieStart();
void movieStop();
int movieDecode();
u8* movieGetFrame();
void movieCheckDisc();
void moviePlay(const char* file);
void moviePause();
int movieIsDone();
}

#endif
