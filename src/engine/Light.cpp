#define LIGHT_CPP
#define DLIST_DEFINE
#include "stage/Stage.h"

#include "engine/Light.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "engine/Color.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "core/SdkCalls.h"
#include "player/Player.h"

extern "C" const u32 sLightBits[8];

// 0xRRGGBBAA as a GXColor
static inline void lightColor(GXColor* out, u32 c)
{
	out->r = c >> 24;
	out->g = c >> 16;
	out->b = c >> 8;
	out->a = c;
}

// 0x80042D60
LightBase::LightBase()
{
	mFlags       = 0;
	mEnabled     = 1;
	mPriority    = 0;
	mGxId        = 0;
	mLink.mLight = this;
}

// 0x80042D98
LightSet::LightSet()
	: mLights(0x10)
{
}

// 0x80042DCC
void LightSet::add(LightBase* light)
{
	mLights.push_back(light);
}

// 0x80042DF4
void LightSet::remove(LightBase* light)
{
	for (dlist<LightBase*>::iterator it = mLights.begin(); it.valid(); ++it) {
		if (*it == light) {
			mLights.erase(it);
			return;
		}
	}
}

// 0x80042E58
LightGroup::LightGroup()
{
	mDiffuse  = 0;
	mSpecular = 0;
	mAlpha    = 0;
}

// 0x80042E98
LightGroup::~LightGroup()
{
}

// 0x80042EE4
// Goes before the first light with a higher priority number
void LightGroup::add(LightBase* light)
{
	if (light->mLink.mList != NULL) {
		mList.remove(&light->mLink);
	}
	LightLink* next = NULL;
	for (LightLink* link = mList.mFirst; link != NULL; link = link->mNext) {
		if (light->mPriority < link->mLight->mPriority) {
			next = link;
			break;
		}
	}
	if (next != NULL) {
		mList.insertBefore(&light->mLink, next);
	} else {
		mList.append(&light->mLink);
	}
}

// 0x80042F7C
void LightGroup::remove(LightBase* light)
{
	if (light->mLink.mList != NULL) {
		mList.remove(&light->mLink);
	}
}

// 0x80042FAC
void LightGroup::addAll(LightSet* set)
{
	for (dlist<LightBase*>::node* node = set->mLights.mHead; node != NULL; node = node->mNext) {
		add(node->mData);
	}
}

// 0x80042FF4
void LightGroup::removeAll(LightSet* set)
{
	for (dlist<LightBase*>::node* node = set->mLights.mHead; node != NULL; node = node->mNext) {
		remove(node->mData);
	}
}

// 0x8004303C
void LightGroup::set(LightSet* set)
{
	clear();
	addAll(set);
}

// 0x80043078
void LightGroup::clear()
{
	mList.clear();
}

// 0x80043098
// Sends up to four lights to GX and gathers their channel masks by the flags of each
void lightGroupLoad(LightGroup* group)
{
	LightLink* node;
	int count = 0;
	u32 diffuse = 0;
	u32 specular = 0;
	u32 alpha = 0;
	u32 alphaSpec = 0;
	PSMTXCopy(_gj_matrix_, group->mMatrix);
	for (node = group->mList.mFirst; node != NULL && count < 4; node = node->mNext) {
		LightBase* light = node->mLight;
		if ((int)light->mEnabled == 0) {
			continue;
		}
		if (light->mFlags & 1) {
			light->init(NULL, group->mMatrix);
			continue;
		}
		u32 id = lightGxId(count);
		u32 flags = light->mFlags;
		light->mGxId = id;
		if (flags & 2) {
			diffuse |= id;
		}
		if (flags & 4) {
			specular |= id;
		}
		if (flags & 8) {
			alpha |= id;
		}
		if (flags & 0x10) {
			alphaSpec |= id;
		}
		light->init(&light->mGxLight, group->mMatrix);
		GXLoadLightObjImm(&light->mGxLight, light->mGxId);
		count++;
	}
	group->mDiffuse   = diffuse;
	group->mSpecular  = specular;
	group->mAlpha     = alpha;
	group->mAlphaSpec = alphaSpec;
}

// 0x800431B4
AmbientLight::AmbientLight()
{
	mFlags = 1;
	mColor = gColorBlack;
}

// 0x80043200
void AmbientLight::init(GXLightObj* gxLight, const f32* matrix)
{
	GXColor c;
	lightColor(&c, mColor);
	GXSetChanAmbColor(GX_COLOR0A0, c);
}

// 0x80043250
DirLight::DirLight()
{
	mColor = gColorWhite;
	mDir   = gAxisZ;
}

// 0x800432B0
// A directional light in GX is a point light very far away
void lightInitDirectional(GXLightObj* gxLight, const f32* matrix, const Vec3f* dir, const u32* color)
{
	GXColor c;
	Vec3f pos;
	PSMTXMultVecSR(matrix, (const NJS_VECTOR*)dir, (NJS_VECTOR*)&pos);
	pos.x *= -1000000.0f;
	pos.y *= -1000000.0f;
	pos.z *= -1000000.0f;
	lightColor(&c, *color);
	GXInitLightPos(gxLight, pos.x, pos.y, pos.z);
	GXInitLightAttn(gxLight, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	GXInitLightColor(gxLight, c);
}

// 0x80043370
void DirLight::init(GXLightObj* gxLight, const f32* matrix)
{
	lightInitDirectional(gxLight, matrix, &mDir, &mColor);
}

// 0x800433A8
PointLight::PointLight()
{
	mColor = gColorWhite;
	mPos   = gVecZero;
	mKa[0] = 0.0f;
	mKa[1] = 0.0f;
	mKa[2] = 0.0f;
}

// 0x8004341C
void PointLight::init(GXLightObj* gxLight, const f32* matrix)
{
	Vec3f pos;
	GXColor c;
	lightColor(&c, mColor);
	PSMTXMultVec(matrix, (const NJS_VECTOR*)&mPos, (NJS_VECTOR*)&pos);
	GXInitLightPos(gxLight, pos.x, pos.y, pos.z);
	GXInitLightAttn(gxLight, 1.0f, 0.0f, 0.0f, mKa[0], mKa[1], mKa[2]);
	GXInitLightColor(gxLight, c);
}

// 0x800434C0
SpotLight::SpotLight()
{
}

// 0x800434FC
void SpotLight::init(GXLightObj* gxLight, const f32* matrix)
{
	Vec3f v;
	GXColor c;
	lightColor(&c, mColor);
	PSMTXMultVec(matrix, (const NJS_VECTOR*)&mPos, (NJS_VECTOR*)&v);
	GXInitLightPos(gxLight, v.x, v.y, v.z);
	PSMTXMultVecSR(matrix, (const NJS_VECTOR*)&mDir, (NJS_VECTOR*)&v);
	GXInitLightDir(gxLight, v.x, v.y, v.z);
	GXInitLightAttn(gxLight, mA[0], mA[1], mA[2], mK[0], mK[1], mK[2]);
	GXInitLightColor(gxLight, c);
}

// 0x800435C8
u32 lightGxId(int index)
{
	return sLightBits[index];
}

// Stripped keeps the dtor below where it is defined
static void spotLightKill(SpotLight* light)
{
	light->~SpotLight();
}

// 0x800435DC
inline SpotLight::~SpotLight() { }

// 0x80043638
LightList::LightList()
{
	mFirst = NULL;
	mLast  = NULL;
	mCount = 0;
}

// 0x8004364C
LightList::~LightList()
{
	unlinkAll();
}

// 0x80043694
void LightList::append(LightLink* link)
{
	if (mFirst == NULL) {
		mLast  = link;
		mFirst = link;
	} else {
		link->mPrev   = mLast;
		mLast->mNext  = link;
		mLast         = link;
	}
	link->mList = this;
	mCount++;
}

// 0x800436D4
void LightList::clear()
{
	unlinkAll();
	mFirst = NULL;
	mLast  = NULL;
	mCount = 0;
}

// 0x80043710
void LightList::remove(LightLink* link)
{
	if (link->mPrev != NULL) {
		link->mPrev->mNext = link->mNext;
	} else {
		mFirst = link->mNext;
	}
	if (link->mNext != NULL) {
		link->mNext->mPrev = link->mPrev;
	} else {
		mLast = link->mPrev;
	}
	link->mNext = NULL;
	link->mPrev = NULL;
	link->mList = NULL;
	mCount--;
}

// 0x80043770
void LightList::insertBefore(LightLink* link, LightLink* next)
{
	link->mPrev = next->mPrev;
	link->mNext = next;
	if (next->mPrev != NULL) {
		next->mPrev->mNext = link;
	} else {
		mFirst = link;
	}
	next->mPrev = link;
	link->mList = this;
	mCount++;
}

// 0x800437AC
void LightList::unlinkAll()
{
	LightLink* link = mFirst;
	while (link != NULL) {
		LightLink* next = link->mNext;
		link->mList     = NULL;
		link->mNext     = NULL;
		link->mPrev     = NULL;
		link            = next;
	}
}

// Stripped takes a node straight from a set's pool
static void lightSetTakeNode(LightSet* set)
{
	set->mLights.alloc();
}
