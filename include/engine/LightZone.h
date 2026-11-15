#ifndef _LIGHTZONE_H
#define _LIGHTZONE_H
struct BinFile;

#include "stage/Stage.h"

/**
 * One preset of set_light_param.bin.
 * @size 0x40
 */
struct LightParam {
	s16 mType;       // _00 0 changes the sun 1 adds a spot light
	u16 mFlags;      // _02 LightZoneSun's
	Vec3f mDir;      // _04 the sun's direction
	u32 mColor;      // _10 the sun's colour
	u32 mAmbient;    // _14
	f32 mRefDist;    // _18 for GXInitLightDistAttn
	f32 mRefBright;  // _1C
	int mDistFn;     // _20 GXDistAttnFn 0 = none
	f32 mCutoff;     // _24 for GXInitLightSpot
	int mSpotFn;     // _28 GXSpotFn 0 = none
	u8 _2C[0x14];

	int hasDistAttn();                        // 0x800A1DEC
	void getDistAttn(f32* k0, f32* k1, f32* k2); // 0x800A1E00 1 0 0 when there is none
	int hasSpot();                            // 0x800A1E90
	void getSpotAttn(f32* a0, f32* a1, f32* a2); // 0x800A1EA4 1 0 0 when there is none
	int isPlain();                            // 0x800A1F30 neither
};

/**
 * What a zone does to the sun. A blend of 1 or more replaces the stage's values, less blends toward them.
 * @size 0x1C
 */
struct LightZoneSun {
	void init();  // 0x800A1528
	int isOn() const;   // 0x800A1548

	f32 mBlend;   // _00
	u32 mFlags;   // _04 4 = set the ambient too
	u32 mColor;   // _08
	u32 mAmbient; // _0C
	Vec3f mDir;   // _10
};

/**
 * The spot light a zone adds.
 * @size 0x34
 */
struct LightZoneSpot {
	void init(); // 0x800A156C

	Vec3f mPos;  // _00
	Vec3f mDir;  // _0C
	u32 mColor;  // _18
	f32 mA[3];   // _1C
	f32 mK[3];   // _28
};

/**
 * An area that changes the lighting. Each one links itself onto the end of sLightZones when it is built, and the
 * subclass says what to do. Its vtable, 0x80258918, is all pure.
 *
 * @size 0xC
 */
struct LightZone {
	LightZone* mNext; // _00
	LightZone* mPrev; // _04

	// _08 = vtable
	LightZone(); // 0x800A15B4
	virtual int test(u16 id, int kind, f32* dist) = 0;          // _08 kind 0 = the sun 1 = the spot
	virtual int getSun(u16 id, LightZoneSun* sun) = 0;          // _0C
	virtual int getSpot(u16 id, LightZoneSpot* spot) = 0;       // _10
	virtual ~LightZone() = 0; // 0x800A1604
};

/**
 * Applies the nearest zone for one object, and puts the stage's lights back afterwards. There is one, in sLightZoneCtrl.
 *
 * @size 0x100
 */
struct LightZoneCtrl {
	LightZoneCtrl();  // 0x800A17B8
	~LightZoneCtrl(); // 0x800A1838

	LightZone* find(u16 id, int kind);  // 0x800A18C8 the nearest zone that takes the object
	void begin(u16 id);                 // 0x800A1968
	int apply(u16 id);                  // 0x800A19A0
	void load(int force);               // 0x800A19F4
	void applySun(u16 id);              // 0x800A1A34
	void applySpot(u16 id);             // 0x800A1BB0
	int restore();                      // 0x800A1C98
	void end(u16 id);                   // 0x800A1D44

	LightZoneSun mSun;    // _00
	LightZoneSpot mSpot;  // _1C
	SpotLight mLight;     // _50
	u32 mApplied;         // _E4 1 = the sun was changed 2 = the spot was added
	u32 mSavedColor;      // _E8
	Vec3f mSavedDir;      // _EC
	u32 mSavedAmbient;    // _F8
	u16 mId;              // _FC
};

extern "C" {
void lightZoneInit();                   // 0x800A168C loads set_light_param.bin
void lightSetDefaultMain(int param);    // 0x800A16EC
void lightSetDefaultOther(int param);   // 0x800A1714
void lightZoneExit();                   // 0x800A173C
LightParam* getLightParam(int param);   // 0x800A18A0 null past 128
void lightZoneBegin(u16 id);            // 0x800A1D7C
void lightZoneEnd(u16 id);              // 0x800A1DB0
int lightFlagsOn(u32 flags);            // 0x800A1DE4 always 1
void lightFarPoint(const Vec3f* pos, const Vec3f* dir, Vec3f* out); // 0x800A1F80 100000 back along dir
void lightParamGetDefault(Vec3f* dir, u32* color, u32* ambient);   // 0x800A1FB8
void lightParamGet(int param, Vec3f* dir, u32* color, u32* ambient); // 0x800A203C
void lightParamBlend(f32 t, int from, int to, Vec3f* dir, u32* color, u32* ambient); // 0x800A20B4
}

extern "C" {
extern LightZone* sLightZones;
extern LightZoneCtrl* sLightZoneCtrl;
extern u8* sLightZoneA;
extern u8* sLightZoneB;
extern BinFile* sLightParamFile;
extern LightParam* sLightParams;
}

#endif
