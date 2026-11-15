#include "enemy/boss/EneBoss.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "egg/EggCreature.h"
#include "stl/math.h"
#include "object/ObjAction.h"
#include "ui/TFadeModel.h"
#include "Ninja/ninja.h"

f32 sFade0Pos[372] __attribute__((aligned(32))) = {
	7.44884682f,   5.58662081f,
	0.0f,          7.44884777f,
	-5.58661985f,  0.0f,
	-7.44884586f,  5.58661985f,
	0.0f,          -7.44884777f,
	-5.58661985f,  0.0f,
	-1.27664006f,  -1.463673f,
	0.0f,          -1.17301798f,
	-1.32474506f,  0.0f,
	-1.04265201f,  -1.21357203f,
	0.0f,          -0.896071017f,
	-1.12710595f,  0.0f,
	-0.743807018f, -1.06230402f,
	0.0f,          -0.61472702f,
	-1.14002395f,  0.0f,
	-0.476401001f, -1.20252299f,
	0.0f,          -0.329975992f,
	-1.24862301f,  0.0f,
	-0.176595002f, -1.27714598f,
	0.0f,          -0.0174010005f,
	-1.28691494f,  0.0f,
	0.389387995f,  -1.22129905f,
	0.0f,          0.742664993f,
	-1.03858495f,  0.0f,
	1.02124095f,   -0.759976983f,
	0.0f,          1.20392597f,
	-0.406673998f, 0.0f,
	1.26953101f,   0.000116000003f,
	0.0f,          1.20392704f,
	0.406902999f,  0.0f,
	1.02124095f,   0.760194004f,
	0.0f,          0.742664993f,
	1.03879201f,   0.0f,
	0.389387995f,  1.22149706f,
	0.0f,          -0.0174000002f,
	1.28710902f,   0.0f,
	-0.179565996f, 1.276963f,
	0.0f,          -0.335642993f,
	1.24735498f,   0.0f,
	-0.484432995f, 1.19953597f,
	0.0f,          -0.624741971f,
	1.13475299f,   0.0f,
	-0.759669006f, 1.05117297f,
	0.0f,          -1.43926501f,
	0.296550989f,  0.0f,
	-1.66129398f,  -0.428651005f,
	0.0f,          -1.60185802f,
	-1.04499996f,  0.0f,
	-1.43705595f,  -1.47306097f,
	0.0f,          -1.34298897f,
	-1.63339996f,  0.0f,
	2.03696895f,   0.124430999f,
	0.0f,          1.88554597f,
	0.3741f,       0.0f,
	2.098804f,     0.532764018f,
	0.0f,          2.30174708f,
	0.653593004f,  0.0f,
	2.47307801f,   0.738066018f,
	0.0f,          2.63570189f,
	0.803866982f,  0.0f,
	2.33564496f,   1.11655998f,
	0.0f,          1.97462296f,
	1.359375f,     0.0f,
	1.60831797f,   1.53376997f,
	0.0f,          1.29241002f,
	1.64120102f,   0.0f,
	1.08257794f,   1.68312502f,
	0.0f,          1.09714603f,
	1.95706201f,   0.0f,
	1.13256395f,   2.19034505f,
	0.0f,          1.17499602f,
	2.37136102f,   0.0f,
	1.22554803f,   2.53011394f,
	0.0f,          0.630919993f,
	2.43949509f,   0.0f,
	0.15851f,      2.27047896f,
	0.0f,          -0.185983002f,
	2.07967091f,   0.0f,
	-0.396858007f, 1.92367804f,
	0.0f,          -0.468416005f,
	1.85910106f,   0.0f,
	-0.643598974f, 2.00800395f,
	0.0f,          -0.765444994f,
	2.18171692f,   0.0f,
	-0.842230976f, 2.34697795f,
	0.0f,          -0.893729985f,
	2.519099f,     0.0f,
	-1.16293001f,  2.19474006f,
	0.0f,          -1.34097803f,
	1.86118102f,   0.0f,
	-1.43403602f,  1.52734601f,
	0.0f,          -1.448264f,
	1.20216596f,   0.0f,
	-1.38982403f,  0.894567013f,
	0.0f,          -1.18971705f,
	1.14396203f,   0.0f,
	-0.945487022f, 1.35010099f,
	0.0f,          -0.664068997f,
	1.50599694f,   0.0f,
	-0.352394998f, 1.60466003f,
	0.0f,          -0.0174010005f,
	1.63910103f,   0.0f,
	0.500054002f,  1.55540705f,
	0.0f,          0.949892998f,
	1.32245898f,   0.0f,
	1.30489898f,   0.96745801f,
	0.0f,          1.53786099f,
	0.517610013f,  0.0f,
	1.62156296f,   0.000116000003f,
	0.0f,          1.53917694f,
	-0.513369024f, 0.0f,
	1.30972397f,   -0.960464001f,
	0.0f,          0.95976299f,
	-1.31465805f,  0.0f,
	0.515852988f,  -1.54944694f,
	0.0f,          0.00455000019f,
	-1.63831902f,  0.0f,
	0.0647170022f, -1.82764006f,
	0.0f,          0.0584320016f,
	-2.00699306f,  0.0f,
	0.0161879994f, -2.157938f,
	0.0f,          -0.0542000011f,
	-2.30082107f,  0.0f,
	0.565742016f,  -2.16703105f,
	0.0f,          0.993158996f,
	-1.91550803f,  0.0f,
	1.25951099f,   -1.63096702f,
	0.0f,          1.43484294f,
	-1.30167997f,  0.0f,
	1.66646302f,   -1.299119f,
	0.0f,          1.88831699f,
	-1.32290494f,  0.0f,
	2.07644701f,   -1.35822904f,
	0.0f,          2.25570297f,
	-1.40425897f,  0.0f,
	2.24537802f,   -0.717559993f,
	0.0f,          2.15709996f,
	-0.213807002f, 0.0f,
	0.339082003f,  -0.247926995f,
	0.0f,          0.248160005f,
	-0.338845998f, 0.0f,
	0.132859007f,  -0.398470998f,
	0.0f,          9.79999968e-05f,
	-0.419883013f, 0.0f,
	-0.132652f,    -0.398470998f,
	0.0f,          -0.247945994f,
	-0.338845998f, 0.0f,
	-0.339273989f, -0.247134998f,
	0.0f,          -0.398488998f,
	-0.132633001f, 0.0f,
	-0.4199f,      0.000116000003f,
	0.0f,          -0.398487985f,
	0.132865995f,  0.0f,
	-0.338862985f, 0.248160005f,
	0.0f,          -0.247944996f,
	0.339078993f,  0.0f,
	-0.132651001f, 0.398703992f,
	0.0f,          9.79999968e-05f,
	0.420116991f,  0.0f,
	0.132857993f,  0.398703992f,
	0.0f,          0.248159006f,
	0.339078993f,  0.0f,
	0.339080989f,  0.248160005f,
	0.0f,          0.398707986f,
	0.132865995f,  0.0f,
	0.420121014f,  0.000116000003f,
	0.0f,          0.398708999f,
	-0.132633001f, 0.0f,
	0.0f,          5.58661985f,
	0.0f,          -7.44884682f,
	0.0f,          0.0f,
	7.44884777f,   0.0f,
	0.0f,          0.0f,
	-5.58661985f,  0.0f,
	-3.72442293f,  5.58661985f,
	0.0f,          3.72442389f,
	5.58661985f,   0.0f,
	7.44884777f,   2.79331112f,
	0.0f,          7.44884777f,
	-2.79330993f,  0.0f,
	-7.44884586f,  2.79330993f,
	0.0f,          -7.44884682f,
	-2.79330993f,  0.0f,
	-3.72442389f,  -5.58661985f,
	0.0f,          3.72442389f,
	-5.58661985f,  0.0f,
};

// 0x80269270
void* sFade0Nrm[372] = {
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
};

// 0x80269840
void* sFade0VList[12] = {
	(void*)0x10C007C,
	(void*)0x41,
	(void*)sFade0Pos,
	(void*)0x5D0,
	(void*)0x20C007C,
	(void*)0x42,
	sFade0Nrm,
	(void*)0x5D0,
	(void*)0xFF000000,
	0,
	0,
	0,
};

// 0x80269870
void* sFade0Strip0[20] = {
	0,
	(void*)0x11400,
	0,
	(void*)0x22400,
	(void*)0x1000000,
	(void*)0x28,
	(void*)0x4000000,
	(void*)0x2500,
	(void*)0x2000000,
	(void*)0x14301,
	(void*)0x5000000,
	(void*)0xFF,
	(void*)0x9000000,
	(void*)0x884,
	0,
	0,
	0,
	0,
	0,
	0,
};

// 0x802698C0
void* sFade0Strip1[120] = {
	(void*)0x98000568,
	(void*)0x68676764,
	(void*)0x64666665,
	(void*)0x65980005,
	(void*)0x6B6B6A6A,
	(void*)0x64646969,
	(void*)0x68689800,
	(void*)0x5595958,
	(void*)0x58555557,
	(void*)0x57565698,
	(void*)0x53030,
	(void*)0x2F2F2B2B,
	(void*)0x2E2E2D2D,
	(void*)0x9800053C,
	(void*)0x3C3B3B78,
	(void*)0x783A3A39,
	(void*)0x39980006,
	(void*)0x72727676,
	(void*)0x27270000,
	(void*)0x28282929,
	(void*)0x98000551,
	(void*)0x5150504D,
	(void*)0x4D4F4F4E,
	(void*)0x4E980007,
	(void*)0x4042121,
	(void*)0x4D4D0303,
	(void*)0x51517A7A,
	(void*)0x73739800,
	(void*)0x5080807,
	(void*)0x74D4D06,
	(void*)0x6050598,
	(void*)0x50B0B,
	(void*)0xA0A4D4D,
	(void*)0x9090808,
	(void*)0x9800050E,
	(void*)0xE0D0D4D,
	(void*)0x4D0C0C0B,
	(void*)0xB980018,
	(void*)0x41411818,
	(void*)0x42421717,
	(void*)0x43431616,
	(void*)0x44441515,
	(void*)0x45451414,
	(void*)0x46461313,
	(void*)0x47471212,
	(void*)0x48481111,
	(void*)0x49491010,
	(void*)0x4A4A0F0F,
	(void*)0x4B4B0E0E,
	(void*)0x4C4C4D4D,
	(void*)0x98000571,
	(void*)0x713E3E78,
	(void*)0x783D3D3C,
	(void*)0x3C980005,
	(void*)0x62626161,
	(void*)0x5D5D6060,
	(void*)0x5F5F9800,
	(void*)0x55C5C64,
	(void*)0x645D5D63,
	(void*)0x63626298,
	(void*)0x73939,
	(void*)0x35357070,
	(void*)0x34347575,
	(void*)0x33333232,
	(void*)0x98000732,
	(void*)0x32313175,
	(void*)0x75303000,
	(void*)0x2B2B2A,
	(void*)0x2A980009,
	(void*)0x77775959,
	(void*)0x1015555,
	(void*)0x7B7B5454,
	(void*)0x73735353,
	(void*)0x52529800,
	(void*)0x5707074,
	(void*)0x74393902,
	(void*)0x2787898,
	(void*)0x72727,
	(void*)0x23237272,
	(void*)0x22227777,
	(void*)0x5B5B5A5A,
	(void*)0x98000F20,
	(void*)0x201F1F79,
	(void*)0x791E1E71,
	(void*)0x711D1D3E,
	(void*)0x3E1C1C3F,
	(void*)0x3F1B1B40,
	(void*)0x401A1A41,
	(void*)0x41191918,
	(void*)0x18980007,
	(void*)0x5C5C6F6F,
	(void*)0x64646E6E,
	(void*)0x6B6B6D6D,
	(void*)0x6C6C9000,
	(void*)0x2A373735,
	(void*)0x35383835,
	(void*)0x35373736,
	(void*)0x36353539,
	(void*)0x3938382D,
	(void*)0x2D2C2C2B,
	(void*)0x2B2A2A29,
	(void*)0x29000059,
	(void*)0x5977775A,
	(void*)0x5A252523,
	(void*)0x23262623,
	(void*)0x23252524,
	(void*)0x24232327,
	(void*)0x27262605,
	(void*)0x504044D,
	(void*)0x4D030320,
	(void*)0x20797920,
	(void*)0x20030321,
	(void*)0x21737352,
	(void*)0x5251515F,
	(void*)0x5F5E5E5D,
	(void*)0x5D000000,
	0,
	0,
	0,
	0,
};

// 0x80269AA0
void* sFade0PList[4] = {
	sFade0Strip0,
	(void*)0x7,
	sFade0Strip1,
	(void*)0x1E0,
};

// 0x80269AB0
void* sFade0Model[9] = {
	sFade0VList, 0, sFade0PList, 0, (void*)0x10000, 0, 0, 0, (void*)0x40EE5CF6,
};

// 0x80269AD4
void* sFade0Object[14] = {
	(void*)0x17, sFade0Model, 0, 0, 0, 0, 0, 0, (void*)0x3F800000, (void*)0x3F800000, (void*)0x3F800000, 0, 0, 0,
};

// 0x80269B0C
void* sFade1Pos[288] = {
	(void*)0x40EE5CF2,
	(void*)0x40B2C55D,
	0,
	(void*)0x40EE5CF4,
	(void*)0xC0B2C55A,
	0,
	(void*)0xC0EE5CF2,
	(void*)0x40B2C55A,
	0,
	(void*)0xC0EE5CF6,
	(void*)0xC0B2C55D,
	0,
	(void*)0xB7B88CA4,
	(void*)0x3FFF6380,
	0,
	(void*)0xBF2173FB,
	(void*)0x3FF25E70,
	0,
	(void*)0xBF96D573,
	(void*)0x3FCE1D21,
	0,
	(void*)0xBFCE1E36,
	(void*)0x3F96D45F,
	0,
	(void*)0xBFF25F85,
	(void*)0x3F2171E3,
	0,
	(void*)0xBFFF6495,
	(void*)0xB749539C,
	0,
	(void*)0xBFF25F85,
	(void*)0xBF21714C,
	0,
	(void*)0xBFCE1E36,
	(void*)0xBF96D3E9,
	0,
	(void*)0xBF96D56B,
	(void*)0xBFCE1D08,
	0,
	(void*)0xBF2173EB,
	(void*)0xBFF25ED5,
	0,
	(void*)0xB7B02928,
	(void*)0xBFFF6428,
	0,
	(void*)0x3E004CCF,
	(void*)0xBFFED613,
	0,
	(void*)0xBE56D1E1,
	(void*)0xBFD3F0FE,
	0,
	(void*)0xBD683D36,
	(void*)0xBFDB3E92,
	0,
	(void*)0x3E0F1173,
	(void*)0xBFE2BEF9,
	0,
	(void*)0x3EAFCCC0,
	(void*)0xBFE9A483,
	0,
	(void*)0x3F04A47F,
	(void*)0xBFEF2168,
	0,
	(void*)0x3F20C122,
	(void*)0xBFF2680A,
	0,
	(void*)0x3F4A7B1C,
	(void*)0xBFEA73E7,
	0,
	(void*)0x3F725CF6,
	(void*)0xBFE0C5C1,
	0,
	(void*)0x3F8C1C26,
	(void*)0xBFD575A7,
	0,
	(void*)0x3F9DEF7C,
	(void*)0xBFC89B9D,
	0,
	(void*)0x3FAE913A,
	(void*)0xBFBA4FA9,
	0,
	(void*)0x3F763ED1,
	(void*)0xBFA8F89C,
	0,
	(void*)0x3F84A401,
	(void*)0xBFA90F49,
	0,
	(void*)0x3F94A151,
	(void*)0xBFA8136E,
	0,
	(void*)0x3FA723C4,
	(void*)0xBFA68598,
	0,
	(void*)0x3FC3E99F,
	(void*)0xBFA3B624,
	0,
	(void*)0x3FD858A3,
	(void*)0xBF87A701,
	0,
	(void*)0x3FE8E07E,
	(void*)0xBF51C8EB,
	0,
	(void*)0x3FF52370,
	(void*)0xBF0F950C,
	0,
	(void*)0x3FFCC3A0,
	(void*)0xBE92D4B2,
	0,
	(void*)0x3FFF632C,
	(void*)0xB749539C,
	0,
	(void*)0x3FFDC4A0,
	(void*)0x3E6731D3,
	0,
	(void*)0x3FF90718,
	(void*)0x3EE36A83,
	0,
	(void*)0x3FF157CD,
	(void*)0x3F2760F2,
	0,
	(void*)0x3FE6E400,
	(void*)0x3F5A76B8,
	0,
	(void*)0x3FD9D8E0,
	(void*)0x3F854EEC,
	0,
	(void*)0x3FDABE16,
	(void*)0x3F8035AD,
	0,
	(void*)0x3FDBF6CB,
	(void*)0x3F6BA8F8,
	0,
	(void*)0x3FDC61FA,
	(void*)0x3F567514,
	0,
	(void*)0x3FD56D8F,
	(void*)0x3F0075F7,
	0,
	(void*)0x3FC21227,
	(void*)0x3E56CEFF,
	0,
	(void*)0x3FA49281,
	(void*)0xBCA97BFA,
	0,
	(void*)0x3F7E6289,
	(void*)0xBE300C52,
	0,
	(void*)0x3F286280,
	(void*)0xBE67B070,
	0,
	(void*)0x3EA4C469,
	(void*)0xBE300C95,
	0,
	(void*)0x3CF3E682,
	(void*)0xBCA97BFA,
	0,
	(void*)0xBE4D8341,
	(void*)0x3E56CEFF,
	0,
	(void*)0xBEB4306E,
	(void*)0x3F0075F7,
	0,
	(void*)0xBED0027E,
	(void*)0x3F567514,
	0,
	(void*)0xBEB5B318,
	(void*)0x3F951455,
	0,
	(void*)0xBE589B0F,
	(void*)0x3FB9AE36,
	0,
	(void*)0x3BF86A0A,
	(void*)0x3FD6F5B2,
	0,
	(void*)0x3E928D65,
	(void*)0x3FEAD845,
	0,
	(void*)0x3F1BB481,
	(void*)0x3FF3437C,
	0,
	(void*)0x3EFBBD9D,
	(void*)0x3FF78A01,
	0,
	(void*)0x3EBEA63B,
	(void*)0x3FFAEC42,
	0,
	(void*)0x3E80403A,
	(void*)0x3FFD61A2,
	0,
	(void*)0x3E0151A4,
	(void*)0x3FFEE19C,
	0,
	(void*)0xBF804D7F,
	(void*)0x3F37C43B,
	0,
	(void*)0xBF68954A,
	(void*)0x3F2B57D6,
	0,
	(void*)0xBF55A3D3,
	(void*)0x3F18664D,
	0,
	(void*)0xBF49378F,
	(void*)0x3F00609A,
	0,
	(void*)0xBF44C198,
	(void*)0x3EC96F65,
	0,
	(void*)0xBF4937A0,
	(void*)0x3E921D75,
	0,
	(void*)0xBF55A3E4,
	(void*)0x3E44241C,
	0,
	(void*)0xBF68955B,
	(void*)0x3DF0BC7B,
	0,
	(void*)0xBF804D87,
	(void*)0x3D8D59D5,
	0,
	(void*)0xBF8E2203,
	(void*)0x3D535222,
	0,
	(void*)0xBF9BF66F,
	(void*)0x3D8D59D5,
	0,
	(void*)0xBFA7F948,
	(void*)0x3DF0BC7B,
	0,
	(void*)0xBFB17204,
	(void*)0x3E44241C,
	0,
	(void*)0xBFB7A837,
	(void*)0x3E921D75,
	0,
	(void*)0xBFB9E34C,
	(void*)0x3EC96F65,
	0,
	(void*)0xBFB7A837,
	(void*)0x3F00609A,
	0,
	(void*)0xBFB17204,
	(void*)0x3F18664D,
	0,
	(void*)0xBFA7F948,
	(void*)0x3F2B57D6,
	0,
	(void*)0xBF9BF66F,
	(void*)0x3F37C43B,
	0,
	(void*)0xBF8E21FB,
	(void*)0x3F3C3A54,
	0,
	0,
	(void*)0x40B2C55A,
	0,
	(void*)0xC06E5CF2,
	(void*)0x40B2C55A,
	0,
	(void*)0x406E5CF6,
	(void*)0x40B2C55D,
	0,
	(void*)0x40EE5CF4,
	0,
	0,
	(void*)0x40EE5CF4,
	(void*)0x4032C55D,
	0,
	(void*)0x40EE5CF4,
	(void*)0xC032C558,
	0,
	(void*)0x80000000,
	(void*)0xC0B2C55A,
	0,
	(void*)0x406E5CF2,
	(void*)0xC0B2C55A,
	0,
	(void*)0xC06E5CF6,
	(void*)0xC0B2C55D,
	0,
	(void*)0xC0EE5CF4,
	(void*)0x80000000,
	0,
	(void*)0xC0EE5CF4,
	(void*)0xC032C55D,
	0,
	(void*)0xC0EE5CF2,
	(void*)0x4032C558,
	0,
};

// 0x80269F8C
void* sFade1Nrm[288] = {
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000, 0, 0, (void*)0xBF800000,
	0, 0, (void*)0xBF800000,
};

// 0x8026A40C
void* sFade1VList[12] = {
	(void*)0x10C0060,  (void*)0x41, sFade1Pos, (void*)0x480,
	(void*)0x20C0060,  (void*)0x42, sFade1Nrm, (void*)0x480,
	(void*)0xFF000000, 0,           0,         0,
};

// 0x8026A43C
void* sFade1Strip0[17] = {
	0,
	(void*)0x11400,
	0,
	(void*)0x22400,
	(void*)0x1000000,
	(void*)0x28,
	(void*)0x4000000,
	(void*)0x2500,
	(void*)0x2000000,
	(void*)0x14301,
	(void*)0x5000000,
	(void*)0xFF,
	(void*)0x9000000,
	(void*)0x884,
	0,
	0,
	0,
};

// 0x8026A480
void* sFade1Strip1[96] = {
	(void*)0x98000549,
	(void*)0x49484844,
	(void*)0x44474746,
	(void*)0x46980005,
	(void*)0x4C4C4B4B,
	(void*)0x44444A4A,
	(void*)0x49499800,
	(void*)0x5131312,
	(void*)0x120F0F11,
	(void*)0x11101098,
	(void*)0x55353,
	(void*)0x52524E4E,
	(void*)0x51515050,
	(void*)0x98000542,
	(void*)0x4241414E,
	(void*)0x4E404053,
	(void*)0x5398000B,
	(void*)0x8080707,
	(void*)0x5F5F0606,
	(void*)0x2020505,
	(void*)0x55550404,
	(void*)0x54543F3F,
	(void*)0x3E3E9800,
	(void*)0x51F1F1E,
	(void*)0x1E1A1A1D,
	(void*)0x1D1C1C98,
	(void*)0x70F0F,
	(void*)0xE0E5A5A,
	(void*)0xD0D5C5C,
	(void*)0xC0C0B0B,
	(void*)0x9800052D,
	(void*)0x2D2C2C29,
	(void*)0x292B2B2A,
	(void*)0x2A980005,
	(void*)0x30302F2F,
	(void*)0x29292E2E,
	(void*)0x2D2D9800,
	(void*)0x5333332,
	(void*)0x32292931,
	(void*)0x31303098,
	(void*)0x53939,
	(void*)0x38383333,
	(void*)0x37373636,
	(void*)0x98000720,
	(void*)0x201F1F59,
	(void*)0x591A1A01,
	(void*)0x1191918,
	(void*)0x18980005,
	(void*)0x3E3E3D3D,
	(void*)0x54543C3C,
	(void*)0x3B3B9800,
	(void*)0x9262625,
	(void*)0x25585824,
	(void*)0x24575723,
	(void*)0x23595922,
	(void*)0x22212198,
	(void*)0x92727,
	(void*)0x58582828,
	(void*)0x2929,
	(void*)0x3B3B3333,
	(void*)0x3A3A3939,
	(void*)0x98000717,
	(void*)0x17161601,
	(void*)0x115150F,
	(void*)0xF141413,
	(void*)0x13980005,
	(void*)0x43434E4E,
	(void*)0x44444D4D,
	(void*)0x4C4C9800,
	(void*)0x95C5C0B,
	(void*)0xB03030A,
	(void*)0xA5E5E09,
	(void*)0x95D5D08,
	(void*)0x85F5F90,
	(void*)0x274646,
	(void*)0x45454444,
	(void*)0x58582727,
	(void*)0x26265050,
	(void*)0x4F4F4E4E,
	(void*)0x4E4E4343,
	(void*)0x42421C1C,
	(void*)0x1B1B1A1A,
	(void*)0x18181717,
	(void*)0x1013535,
	(void*)0x33333636,
	(void*)0x33333535,
	(void*)0x34342121,
	(void*)0x20205959,
	(void*)0x5B5B0F0F,
	(void*)0x5A5A0F0F,
	(void*)0x5B5B0101,
	(void*)0x3B3B5656,
	(void*)0x54545656,
	(void*)0x3B3B0000,
	0,
	0,
};

// 0x8026A600
void* sFade1PList[4] = {
	sFade1Strip0,
	(void*)0x7,
	sFade1Strip1,
	(void*)0x180,
};

// 0x8026A610
void* sFade1Model[9] = {
	sFade1VList, 0, sFade1PList, 0, (void*)0x10000, (void*)0x80000000, 0, 0, (void*)0x40EE5CF4,
};

// 0x8026A634
void* sFade1Object[14] = {
	(void*)0x17, sFade1Model, 0, 0, 0, 0, 0, 0, (void*)0x3F800000, (void*)0x3F800000, (void*)0x3F800000, 0, 0, 0,
};

extern "C" {
void* sFadeShapes[2] = { sFade0Object, sFade1Object };
}

const char* gTFadeModelName = "TFadeModel";
struct NJS_OBJECT;

extern "C" {
extern const f32 sFadeColor[4] = {
	0.5f,
	0.0f,
	0.0f,
	0.0f,
};
}

typedef char TFadeModel_size_check[(sizeof(TFadeModel) == 0x3C) ? 1 : -1];

static inline int fadeClamp(int value)
{
	if (value < 0) {
		return 0;
	}
	return value > 1 ? 1 : value;
}

// 0x801CA0D0
TFadeModel::TFadeModel(TObject* parent, int shape, int open)
    : TObject(parent)
{
	_00    = (u32)gTFadeModelName;
	mSize  = 0x3C;
	mTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3E, 0);
	mShape = fadeClamp(shape);
	mOpen  = fadeClamp(open);
	if (mOpen == 0) {
		mTime = 0;
	} else {
		mTime = 0x3C;
	}
	setTime(mTime);
}

// 0x801CA1B4
TFadeModel::~TFadeModel()
{
	removeDrawTask(mTask);
}

// 0x801CA21C
void TFadeModel::setTime(int frame)
{
	f32 t = (f32)frame / 60.0f;
	if (t < 0.0f) {
		t = 0.0f;
	} else if (t > 1.0f) {
		t = 1.0f;
	}

	if (t >= 0.8f) {
		mAlpha = (int)(255.0f * (1.0f - (t - 0.8f) / (1.0f - 0.8f)));
	} else {
		mAlpha = 0xFF;
	}

	if (t >= 0.6f) {
		f32 power = (f32)pow((t - 0.6f) / (1.0f - 0.6f), 3.0);
		power *= power;
		mZoom = 4000.0f * power;
		mZoom += 1000.0f;
		mRotZ = 0;
	} else if (t >= 0.4f) {
		mZoom = 1000.0f;
		mRotZ = 0;
	} else {
		t /= 0.4f;
		f32 power = (f32)pow(1.0f - t, 2.0);
		mZoom     = 950.0f * (1.0f - power);
		mZoom += 50.0f;
		mRotZ = (int)(65536.0f * (1.0f - power));
	}
}

// 0x801CA3A0
void TFadeModel::draw()
{
	if (mOpen == 0) {
		if (mTime < 0x3C) {
			mTime++;
		}
	} else if (mTime > 0) {
		mTime--;
	}
	setTime(mTime);
}

// 0x801CA3FC
extern "C" void fadeModelFill(f32 size, f32 depth, u32 alpha)
{
	f32 zero = 0.0f;
	f32 vhalf;
	f32 half   = size / 2.0f;
	vhalf      = 0.75f * half;
	f32 inL    = (320.0f - half) * depth;
	f32 inR    = (320.0f + half) * depth;
	f32 right  = 640.0f * depth;
	f32 inT    = (240.0f - vhalf) * depth;
	f32 inB    = (240.0f + vhalf) * depth;
	f32 bottom = 480.0f * depth;

	gjStartVertex2D();
	gjStartVertexNonTexTrans();
	GXBegin(GX_QUADS, GX_VTXFMT4, 0x10);

#define FADE_VERT(X, Y)              \
	GXPosition3f32((X), (Y), depth); \
	GXColor1u32(alpha & 0xFF)

	FADE_VERT(zero, zero);
	FADE_VERT(inL, zero);
	FADE_VERT(inL, bottom);
	FADE_VERT(zero, bottom);

	FADE_VERT(inR, zero);
	FADE_VERT(right, zero);
	FADE_VERT(right, bottom);
	FADE_VERT(inR, bottom);

	FADE_VERT(inL, zero);
	FADE_VERT(inR, zero);
	FADE_VERT(inR, inT);
	FADE_VERT(inL, inT);

	FADE_VERT(inL, inB);
	FADE_VERT(inR, inB);
	FADE_VERT(inR, bottom);
	FADE_VERT(inL, bottom);
#undef FADE_VERT
}

// The constant material alpha then red green and blue
struct FadeArgb {
	f32 a;
	f32 r;
	f32 g;
	f32 b;
};

// 0x801CA604
void TFadeModel::render()
{
	if (mTime >= 0x3C) {
		return;
	}
	pushCullMode(0);
	njZBufferMode(0);
	if (mTime <= 0) {
		fadeModelFill(0.0f, -9.9f, mAlpha);
	} else {
		FadeArgb colour = *(const FadeArgb*)sFadeColor;
		if (mAlpha < 0xFF) {
			colour.a = (f32)mAlpha / 255.0f;
			gjSetBlendOverride(1, 4, 5, 3);
			_nj_control_3d_flag_ |= 0x8010;
			njSetConstantMaterial((f32*)&colour);
		}
		gjPushUnitMatrix();
		f32 scale = mZoom / 15.0f;
		Vec3f at;
		f32 dist = 10.0f;
		scale *= *(f32*)((u8*)&gProjection + 0x54) * dist;
		at.x = 0.0f;
		at.y = 0.0f;
		at.z = -dist;
		njTranslateEx(&at);
		njScale(0, scale, scale, 1.0f);
		gjRotateZ(mRotZ);
		gjDrawObject((NJS_OBJECT*)sFadeShapes[mShape]);
		njPopMatrixEx();
		fadeModelFill(0.5f * mZoom, -9.9f, mAlpha);
		if (mAlpha < 0xFF) {
			_nj_control_3d_flag_ &= 0xFFFF7FEF;
		}
	}
	popCullMode();
	njZBufferMode(1);
}
