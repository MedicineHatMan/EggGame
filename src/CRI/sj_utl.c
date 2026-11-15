#include <CRI/sj.h>
#include <CRI/cri_std.h>

static int sj_hex[112] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
	0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

#define HEXLEN(n, p) n = sj_hex[(p)[8]]; n = (n << 4) + sj_hex[(p)[9]]; n = (n << 4) + sj_hex[(p)[10]]; n = (n << 4) + sj_hex[(p)[11]]; n = (n << 4) + sj_hex[(p)[12]]; n = (n << 4) + sj_hex[(p)[13]]; n = (n << 4) + sj_hex[(p)[14]]

// 0x8022D1A8
void SJ_SplitChunk(SJCK* ck, int nbyte, SJCK* ck1, SJCK* ck2)
{
	*ck1 = *ck;
	ck2->len = ck1->len;
	if (ck1->len > nbyte) {
		ck1->len = nbyte;
	}
	ck2->len -= ck1->len;
	if (ck2->len == 0) {
		ck2->data = 0;
	} else {
		ck2->data = ck1->data + ck1->len;
	}
}

// 0x8022CF94
signed char* SJ_SearchTag(SJCK* cki, const char* kw, const char* endkw, SJCK* cko)
{
	signed char* p;
	signed char* end;
	int n;

	cko->data = 0;
	cko->len = 0;
	p = (signed char*)cki->data;
	end = (signed char*)cki->data + cki->len;
	while (p < end) {
		if (strncmp((char*)p, kw, 7) == 0) {
			cko->data = (char*)p + 0x10;
			HEXLEN(n, p);
			cko->len = n;
			break;
		}
		if (endkw != 0 && strncmp((char*)p, endkw, 7) == 0) {
			return 0;
		}
		HEXLEN(n, p);
		p = (signed char*)(n + (int)p);
		p += 0x10;
	}
	if (p < end) {
		return p;
	}
	return 0;
}
