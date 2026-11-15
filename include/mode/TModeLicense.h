#ifndef _TMODELICENSE_H
#define _TMODELICENSE_H

#include "stage/Stage.h"

class TAeObjLicense;

// The license screen

/**
 * The game mode that shows the license screen until its clip ends. k_license.prd. Vtable 0x8025FD98.
 *
 * @size 0x10
 */
class TModeLicense : public TMode {
public:
	TModeLicense();
	static TModeLicense* create(TModeAllocator* allocator);

	virtual ~TModeLicense();
	virtual void loadFiles();
	virtual int init();
	virtual int shutdown();
	virtual void update();

	TAeObjLicense* mLicense; // _0C
};

extern "C" {
extern u32 sModeLicenseClearColor;
}

#endif
