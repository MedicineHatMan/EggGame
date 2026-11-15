#ifndef _ANTBRAIN_H
#define _ANTBRAIN_H

#include "enemy/TEneAm01.h"
#include "enemy/TEneAntsQueen.h"

/**
 * @size 0x68
 */
struct Am01AntBrain : public EnemySeekBrain {
	enum {
		STATE_APPEAR_AIMED = 6,
	};

	Am01AntBrain(AntLink* head, int slot); // 0x80095164
	virtual ~Am01AntBrain();               // 0x800951C4

	virtual void onPhase(int state, int phase); // _0C
	virtual void onAppearDone();                // _18
	virtual void onFlipDone();                  // _1C
	virtual void attach(TEneAm01* owner);       // _24

	void setChase(u16 player);
	void pickTarget();
	int look();
	void phaseAppearAimed(int phase);
	void phaseWander(int phase);
	void phaseChase(int phase);

	AntLink mLink; // _54
};

#endif
