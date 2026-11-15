#include "ui/TAeObjLicense.h"
#include "io/Resource.h"
#include "io/LangFile.h"
#include "core/TMainTask.h"
#include "ui/AeArc.h"
#include "core/Mem.h"
#include "Ninja/ninja.h"

struct NJS_TEXLIST;

const char* gTAeObjLicenseName = "TAeObjLicense";

/** The data block of ae_license.arc */
struct LicenseFile {
	void* mAe;             // _00 relocated by relocateAe
	NJS_TEXLIST* mTexList; // _04
	void* mGvm;            // _08
};

static TArc* sLicenseArc;        // 0x802C5700
static void* sLicenseAe;         // 0x802C5704
static NJS_TEXLIST* sLicenseTex; // 0x802C5708

typedef char TAeObjLicense_size_check[(sizeof(TAeObjLicense) == 0x3C) ? 1 : -1];

// 0x8014080C
void licenseLoad()
{
	loadArc((const char*)getLangFileName("ae_license.arc"), &sLicenseArc);
	if (sLicenseArc != 0) {
		LicenseFile* file = (LicenseFile*)sLicenseArc->mData;
		sLicenseAe        = file->mAe;
		relocateAe(sLicenseAe);
		sLicenseTex = file->mTexList;
		bindGvmTextures((u8*)file->mGvm, sLicenseTex);
	}
}

// 0x80140870
void licenseFree()
{
	if (sLicenseArc != 0) {
		releaseTextures(sLicenseTex);
		sLicenseTex = 0;
		releaseResource(sLicenseArc);
		sLicenseArc = 0;
		sLicenseAe  = 0;
	}
}

// 0x801408BC
TAeObjLicense* licenseCreate()
{
	if (sLicenseArc == 0) {
		return 0;
	}
	return new TAeObjLicense(gTL_07, sLicenseAe, sLicenseTex);
}

// 0x80140918
TAeObjLicense::TAeObjLicense(TObject* parent, void* data, void* assets)
    : TAeObject(parent, data, assets)
{
	_00    = (u32)gTAeObjLicenseName;
	mSize  = 0x3C;
	mLayer = createLayer(0x2F, 0x10);
	mDone  = 0;
}

// 0x80140980
TAeObjLicense::~TAeObjLicense()
{
}

// 0x801409DC
void TAeObjLicense::draw()
{
	if (mLayer != 0) {
		if (mLayer->isDone() != 0) {
			mDone = 1;
		} else {
			mLayer->update();
		}
	}
}

// 0x80140A34
void TAeObjLicense::vfn_10()
{
	if (mLayer != 0) {
		mLayer->present();
	}
}
