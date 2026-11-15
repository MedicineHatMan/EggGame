#include "Ninja/ninja.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "stl/math.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "engine/GenTex.h"
#include "egg/EggCreature.h"
#include "core/dolphin.h"
#include "ui/AeArc.h"
#include "core/SdkCalls.h"

static NJS_TEXINFO sTexInfo[8];
static NJS_TEXNAME sTexName[8];
static NJS_TEXLIST sTexList;
static u8* sNoiseMip;

static u8* makeSphereTex();
static u8* makeFoldTex();
static u8* makeWaveTex();
static u8* makeNoiseTex();
static void makeNoiseMip();
static u8* makeDomeTex();
static u8* makeRingTex();
static u8* makeRampTex();
static u8* makeGridTex();

// MSL's inline sqrtf
static inline f32 texSqrt(f32 x)
{
	static const f64 _half  = .5;
	static const f64 _three = 3.0;
	volatile f32 y;
	if (x > 0.0f) {
		f64 guess = __frsqrte((f64)x);
		guess     = _half * guess * (_three - guess * guess * x);
		guess     = _half * guess * (_three - guess * guess * x);
		guess     = _half * guess * (_three - guess * guess * x);
		y         = (f32)(x * guess);
		return y;
	}
	return x;
}

// 0x8002BE58
void genTexInit()
{
	u8* sphere         = makeSphereTex();
	u8* wave           = makeWaveTex();
	u8* noise          = makeNoiseTex();
	u8* dome           = makeDomeTex();
	u8* ring           = makeRingTex();
	u8* ramp           = makeRampTex();
	u8* fold           = makeFoldTex();
	u8* grid           = makeGridTex();
	sTexList.textures  = sTexName;
	sTexList.nbTexture = 8;
	njLoadTexture(&sTexList);
	delete[] grid;
	delete[] fold;
	delete[] ramp;
	delete[] ring;
	delete[] dome;
	delete[] noise;
	delete[] wave;
	delete[] sphere;
	makeNoiseMip();
}

// 0x8002BF20
void genTexExit()
{
	if (sNoiseMip != NULL) {
		delete[] sNoiseMip;
		sNoiseMip = NULL;
	}
	njReleaseTexture(&sTexList);
}

// 0x8002BF5C
static u8* genTexAlloc(int width, int height, u32 format, u32* size)
{
	*size = GXGetTexBufferSize(width, height, format, GX_FALSE, 0);
	return new u8[*size];
}

// 0x8002BFA4
static void genTexAdd(u8* buf, u32 size, int width, int height, int format, int slot, u32 texId)
{
	DCStoreRange(buf, size);
	NJS_TEXINFO* info = &sTexInfo[slot];
	njSetTextureInfo(info, buf, format, width, height);
	njSetTextureName(&sTexName[slot], (void*)info, (void*)texId, (u32)0x40800000);
}

// 0x8002C02C
// Texture 0 a hemisphere's x and y over 64x64 flat outside the circle
static u8* makeSphereTex()
{
	u32 size;
	u8* buf = genTexAlloc(64, 64, 3, &size);
	u8* p   = buf;
	for (int ty = 0; ty < 64; ty += 4) {
		for (int tx = 0; tx < 64; tx += 4) {
			for (int y = 0; y < 4; y++) {
				for (int x = 0; x < 4; x++) {
					f32 u = 2.0f * ((tx + x) / 63.0f) - 1.0f;
					f32 v = 2.0f * ((ty + y) / 63.0f) - 1.0f;
					int b, a;
					if (u * u + v * v > 1.0f) {
						a = 0x80;
						b = 0x80;
					} else {
						a = 255.0f * (0.5f * (1.0f + u));
						b = 255.0f * (0.5f * (1.0f + v));
					}
					p[0] = a;
					p[1] = b;
					p += 2;
				}
			}
		}
	}
	genTexAdd(buf, size, 64, 64, 3, 0, 0x1001);
	return buf;
}

// 0x8002C188
// Texture 6 like texture 0 with each half folded back to the edge
static u8* makeFoldTex()
{
	u32 size;
	u8* buf = genTexAlloc(64, 64, 3, &size);
	u8* p   = buf;
	for (int ty = 0; ty < 64; ty += 4) {
		for (int tx = 0; tx < 64; tx += 4) {
			for (int y = 0; y < 4; y++) {
				for (int x = 0; x < 4; x++) {
					f32 u = 2.0f * ((tx + x) / 63.0f) - 1.0f;
					f32 v = 2.0f * ((ty + y) / 63.0f) - 1.0f;
					int a, b;
					if (u * u + v * v > 1.0f) {
						a = 0x80;
						b = 0x80;
					} else {
						f32 fu, fv;
						if (u >= 0.0f) {
							fu = 1.0f - u;
						} else {
							fu = -1.0f - u;
						}
						if (v >= 0.0f) {
							fv = 1.0f - v;
						} else {
							fv = -1.0f - v;
						}
						a = 255.0f * (0.5f * (1.0f + fu));
						b = 255.0f * (0.5f * (1.0f + fv));
					}
					p[0] = a;
					p[1] = b;
					p += 2;
				}
			}
		}
	}
	genTexAdd(buf, size, 64, 64, 3, 6, 0x1007);
	return buf;
}

// 0x8002C32C
// Texture 1 one sine wave across and one down
static u8* makeWaveTex()
{
	u32 size;
	u8* buf = genTexAlloc(64, 64, 3, &size);
	u8* p   = buf;
	for (int ty = 0; ty < 64; ty += 4) {
		for (int tx = 0; tx < 64; tx += 4) {
			int b;
			for (int y = 0; y < 4; y++) {
				b = 255.0f * (0.5f * (1.0f + njSin(((ty + y) << 16) / 64)));
				for (int x = 0; x < 4; x++) {
					p[0] = (int)(255.0f * (0.5f * (1.0f + njSin(((tx + x) << 16) / 64))));
					p[1] = b;
					p += 2;
				}
			}
		}
	}
	genTexAdd(buf, size, 64, 64, 3, 1, 0x1002);
	return buf;
}

// 0x8002C470
// Texture 2 noise
static u8* makeNoiseTex()
{
	u32 size = GXGetTexBufferSize(64, 64, 3, GX_TRUE, 100);
	u8* buf  = genTexAlloc(64, 64, 3, &size);
	u8* p    = buf;
	for (int ty = 0; ty < 64; ty += 4) {
		for (int tx = 0; tx < 64; tx += 4) {
			for (int y = 0; y < 4; y++) {
				for (int x = 0; x < 4; x++) {
					int a = 255.0f * (rand() * (1.0f / 32768.0f));
					int b = 255.0f * (rand() * (1.0f / 32768.0f));
					p[0]  = a;
					p[1]  = b;
					p += 2;
				}
			}
		}
	}
	genTexAdd(buf, size, 64, 64, 3, 2, 0x1003);
	return buf;
}

// 0x8002C5D4
// The noise again with every mip level each 0.8 as strong as the last
static void makeNoiseMip()
{
	u8* p = sNoiseMip = new u8[GXGetTexBufferSize(64, 64, 3, GX_TRUE, 100)];
	f32 amp           = 255.0f;
	int size          = 64;
	for (int ty = 0; ty < 64; ty += 4) {
		for (int tx = 0; tx < 64; tx += 4) {
			for (int y = 0; y < 4; y++) {
				for (int x = 0; x < 4; x++) {
					int a = amp * (rand() * (1.0f / 32768.0f) - 0.5f) + 128.0f;
					int b = amp * (rand() * (1.0f / 32768.0f) - 0.5f) + 128.0f;
					p[0]  = a;
					p[1]  = b;
					p += 2;
				}
			}
		}
	}
	for (amp *= 0.8f, size >>= 1, p = (u8*)(((u32)p + 31) & ~31); size > 0; amp *= 0.8f, size >>= 1, p = (u8*)(((u32)p + 31) & ~31)) {
		for (int ty = 0; ty < size; ty += 4) {
			for (int tx = 0; tx < size; tx += 4) {
				for (int y = 0; y < 4; y++) {
					for (int x = 0; x < 4; x++) {
						int a = amp * (rand() * (1.0f / 32768.0f) - 0.5f) + 128.0f;
						int b = amp * (rand() * (1.0f / 32768.0f) - 0.5f) + 128.0f;
						p[0]  = a;
						p[1]  = b;
						p += 2;
					}
				}
			}
		}
	}
}

// 0x8002C82C
// Texture 3 the slope of a dome flattened by 1.3 toward the centre
static u8* makeDomeTex()
{
	u32 size;
	u8* buf = genTexAlloc(64, 64, 3, &size);
	u8* p   = buf;
	for (int ty = 0; ty < 64; ty += 4) {
		for (int tx = 0; tx < 64; tx += 4) {
			for (int y = 0; y < 4; y++) {
				for (int x = 0; x < 4; x++) {
					f32 u  = 2.0f * ((x + tx) / 64.0f - 0.5f);
					f32 v  = 2.0f * ((y + ty) / 64.0f - 0.5f);
					f32 d2 = u * u + v * v;
					int b, a;
					if (d2 > 1.0f) {
						a = 0x80;
						b = 0x80;
					} else if (d2 == 0.0f) {
						a = 0x80;
						b = 0x80;
					} else {
						f32 c;
						f32 s = texSqrt(1.0f - d2) / 1.3f;
						c     = texSqrt(d2);
						njSinCos(10430.38043493439 * (f32)atan2(s, c), &s, &c);
						f32 t = 1.3f * c;
						f32 k = s / texSqrt(s * s + t * t);
						s *= 1.3f;
						f32 n = texSqrt(c * c + s * s);
						c /= n;
						s /= n;
						f32 r = texSqrt(d2);
						a     = 255.0f * (0.5f * (k * u / r - 1.0f));
						b     = 255.0f * (0.5f * (k * v / r - 1.0f));
					}
					p[0] = a;
					p[1] = b;
					p += 2;
				}
			}
		}
	}
	genTexAdd(buf, size, 64, 64, 3, 3, 0x1004);
	return buf;
}

// 0x8002CC98
// Texture 4 a ring of radius 20 texels both channels the same
static u8* makeRingTex()
{
	u32 size;
	u8* buf = genTexAlloc(64, 64, 3, &size);
	u8* p   = buf;
	for (int ty = 0; ty < 64; ty += 4) {
		for (int tx = 0; tx < 64; tx += 4) {
			for (int y = 0; y < 4; y++) {
				for (int x = 0; x < 4; x++) {
					f32 u = (tx + x - 32) / 20.0f;
					f32 v = (ty + y - 32) / 20.0f;
					f32 t = u * u + v * v;
					f32 e = t - 1.0f;
					if (e > 1.0f) {
						t = 0.0f;
					} else if (e > 0.0f) {
						t = 1.0f - e;
					}
					u8 a = 127.0f * t + 128.0f;
					u8 b = 127.0f * t + 128.0f;
					p[0] = a;
					p[1] = b;
					p += 2;
				}
			}
		}
	}
	genTexAdd(buf, size, 64, 64, 3, 4, 0x1005);
	return buf;
}

// 0x8002CDE8
// Texture 5 an 8x8 I4 ramp across
static u8* makeRampTex()
{
	u32 size;
	u8* buf = genTexAlloc(8, 8, 0, &size);
	u8* p   = buf;
	for (int i = 0; i < 8; i++) {
		p[0] = 0x02;
		p[1] = 0x46;
		p[2] = 0x8A;
		p[3] = 0xCF;
		p += 4;
	}
	genTexAdd(buf, size, 8, 8, 0, 5, 0x1006);
	return buf;
}

// 0x8002CE7C
// Texture 7 16x16 IA4 with the row in the alpha and the column in the intensity
static u8* makeGridTex()
{
	u32 size;
	u8* buf = genTexAlloc(16, 16, 2, &size);
	u8* p   = buf;
	for (int ty = 0; ty < 16; ty += 4) {
		for (int tx = 0; tx < 16; tx += 8) {
			for (int y = 0; y < 4; y++) {
				for (int x = 0; x < 8; x++) {
					*p++ = ((ty + y) & 15) << 4 | ((tx + x) & 15);
				}
			}
		}
	}
	genTexAdd(buf, size, 16, 16, 2, 7, 0x1008);
	return buf;
}

// 0x8002CF40
// Loads texture num into map
void genTexLoadWrap(int num, int map, s32 wrap)
{
	if (num == 2) {
		GXTexObj obj;
		GXInitTexObj(&obj, sNoiseMip, 64, 64, GX_TF_IA8, wrap, wrap, GX_TRUE);
		GXLoadTexObj(&obj, map);
	} else {
		njSetTexture(&sTexList);
		gjLoadTextureNum(num, wrap, wrap, map);
	}
}

// 0x8002CFC8
// Repeating
void genTexLoad(int num, int map)
{
	genTexLoadWrap(num, map, 1);
}
