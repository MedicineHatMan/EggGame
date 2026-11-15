#ifndef _TMODEMOVIE_H
#define _TMODEMOVIE_H

#include "stage/Stage.h"
#include "engine/Movie.h"

enum TModeMovieFlags {
	MOVIE_MODE_SKIPPABLE = 1, // START ends it
	MOVIE_MODE_SKIPPED   = 2, // START was pressed
};

/**
 * @size 0x14
 */
class TModeMovie : public TMode {
public:
	TModeMovie();
	static TModeMovie* create(TModeAllocator* allocator);

	virtual ~TModeMovie();
	virtual void loadFiles();
	virtual void freeFiles();
	virtual int init();
	virtual int shutdown();
	virtual void update();

	void setMovie(int movie, int skippable);

	int mMovie; // _0C a MovieId
	u32 mFlags; // _10 TModeMovieFlags
};

extern TModeMovie* gModeMovie; // the one that is running or NULL

void movieModeCheckDisc();        // 0x80151CF4
const char* movieName(int movie); // 0x80151EE0 the .sfd for a MovieId clamped
void movieModeDrawFrame();        // 0x80151FF4

#endif
