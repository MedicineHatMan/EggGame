#ifndef _TAEOBJLICENSE_H
#define _TAEOBJLICENSE_H

#include "ui/TAeLayer.h"

/**
 * @size 0x3C
 */
class TAeObjLicense : public TAeObject {
public:
	TAeObjLicense(TObject* parent, void* data, void* assets);
	virtual ~TAeObjLicense();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_10(); // _10

	TAeLayer* mLayer; // _34
	int mDone;        // _38 set when the clip finishes
};

extern "C" {
extern const char* gTAeObjLicenseName;
}

void licenseLoad();            // 0x8014080C loads ae_license.arc
void licenseFree();            // 0x80140870
TAeObjLicense* licenseCreate(); // 0x801408BC NULL if the archive did not load

#endif
