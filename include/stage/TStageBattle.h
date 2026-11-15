#ifndef _STAGE_TSTAGEBATTLE_H_
#define _STAGE_TSTAGEBATTLE_H_
#include "Ninja/ninja.h"
#include "stage/Stage.h"
struct TStageBattle;
struct TStage;

#include "core/types.h"

extern "C" {
void battleStageSetHuman(int, int);
void battleStageSetupCameras();
}

extern "C" {
TStage* createBattleStage();
}

class TStageBattle : public TStage {
public:
	TStageBattle();
	virtual ~TStageBattle();
	virtual void loadAssets();
	virtual void run(u32 flags);

	void setPlayers(int n);
	void setCamera(int slot, int camera);
	void setHuman(int slot, int human);
	void setWideCamera(int camera);
	void setQuarterCamera(int camera);
	void setupCameras();
	void useCamera(int camera);
	void viewTop();
	void viewBottom();
	void viewTopLeft();
	void viewTopRight();
	void viewBottomLeft();
	void viewBottomRight();
	void drawView();

	int mPlayers;   // _30 1 to 4
	int mCamera[4]; // _34
	int mHuman[4];  // _44 zero puts a computer player in the slot
};

#endif
