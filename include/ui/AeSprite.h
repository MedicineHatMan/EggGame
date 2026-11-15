#ifndef _UI_AESPRITE_H_
#define _UI_AESPRITE_H_
#include "Ninja/ninja.h"
struct NJS_QUAD_TEXTURE_EX;

#include "core/types.h"

struct AeFrame;

extern "C" {
void aeDrawSprite(AeFrame*, const u32*);
}

/**
 * The quad njDrawQuadTextureEx takes: a corner, the two edges from it, and the matching
 * texture coordinates.
 *
 * @size 0x40
 */
struct NJS_QUAD_TEXTURE_EX {
	f32 x, y, z;  // _00
	f32 vx1, vy1; // _0C
	f32 vx2, vy2; // _14
	f32 _1C;      // _1C
	f32 u, v;     // _20
	f32 vu1, vv1; // _28
	f32 vu2, vv2; // _30
	f32 _38;      // _38
	f32 _3C;      // _3C
};

extern "C" {
extern const NJS_QUAD_TEXTURE_EX sAeZeroQuad;
}

#endif
