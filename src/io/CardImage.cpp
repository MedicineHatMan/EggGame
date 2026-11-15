#include "io/MemCard.h"
#include "Ninja/ninja.h"
#include "stl/string.h"
#include "core/SdkCalls.h"

extern "C" CardImage* createCardImage(int bannerKind, int iconKind, int iconCount, int anim, int speed, u8* images,
                                      const char* title, const char* comment)
{
	u8* banner;
	u8* bannerTlut;
	switch (bannerKind) {
	case 0:
		banner     = images;
		bannerTlut = 0;
		images += 0x1800;
		break;
	case 1:
		banner     = images;
		bannerTlut = images + 0xC00;
		images += 0xE00;
		break;
	case 2:
	default:
		banner     = 0;
		bannerTlut = 0;
		break;
	}
	u8* icons = images;
	if (iconKind != 1) {
		images = 0;
	} else {
		images += iconCount * 0x400;
	}
	return newCardImage(banner, bannerTlut, icons, images, iconCount, anim, speed, title, comment);
}

// 0x801A272C
extern "C" CardImage* newCardImage(void* banner, void* bannerTlut, void* icons, void* iconTlut, int iconCount, int anim,
                                   int speed, const char* title, const char* comment)
{
	return new CardImage(banner, bannerTlut, icons, iconTlut, iconCount, anim, speed, title, comment);
}

// 0x801A27B4
CardImage::CardImage(void* banner, void* bannerTlut, void* icons, void* iconTlut, int iconCount, int anim, int speed,
                     const char* title, const char* comment)
{
	mBanner     = banner;
	mBannerTlut = bannerTlut;
	mIcons      = icons;
	mIconTlut   = iconTlut;
	mIconCount  = iconCount;
	mAnim       = anim;
	mSpeed      = speed;
	mComment    = new char[0x40];
	memset(mComment, 0, 0x40);
	char* text = mComment;
	strncpy(text, title, 0x20);
	strncpy(text + 0x20, comment, 0x20);
	mCommentAddr = 0;
	mSize        = 0x40;
	mIconAddr    = mSize;
	if (mBanner != 0) {
		if (mBannerTlut == 0) {
			mSize += 0x1800;
		} else {
			mSize += 0xE00;
		}
	}
	mIconData = mSize;
	if (mIconCount <= 0 || mIcons == 0) {
		mIconCount = 1;
		mIcons     = 0;
		mIconTlut  = 0;
	}
	if (mIconCount > 8) {
		mIconCount = 8;
	}
	if (mIconTlut == 0) {
		mSize += mIconCount * 0x800;
	} else {
		mSize += iconCount * 0x400 + 0x200;
	}
}

// 0x801A2914
CardImage::~CardImage()
{
	if (mComment != 0) {
		delete mComment;
		mComment = 0;
	}
}

// 0x801A2970
int CardImage::getSize()
{
	return mSize;
}

// 0x801A2978
char* CardImage::getComment()
{
	if (mComment != 0) {
		return mComment + 0x20;
	}
	return 0;
}

// 0x801A2994
// With no icons it draws a 32x32 gradient instead
int CardImage::build(u8* dst)
{
	memcpy(dst, mComment, 0x40);
	dst += 0x40;
	if (mBanner != 0) {
		if (mBannerTlut == 0) {
			memcpy(dst, mBanner, 0x1800);
			dst += 0x1800;
		} else {
			memcpy(dst, mBanner, 0xC00);
			memcpy(dst + 0xC00, mBannerTlut, 0x200);
			dst += 0xE00;
		}
	}
	if (mIcons == 0) {
		u16* pixel = (u16*)dst;
		u16 color  = 0xFFFF;
		for (int y = 0; y < 32; y++) {
			for (int x = 0; x < 32; x++) {
				*pixel++ = color--;
			}
		}
	} else if (mIconTlut == 0) {
		memcpy(dst, mIcons, mIconCount << 11);
	} else {
		memcpy(dst, mIcons, mIconCount << 10);
		memcpy(dst + 1, mIconTlut, mIconCount << 9); // sic one byte in not after the icons
	}
	return mSize;
}

// 0x801A2AAC
int CardImage::bannerFormat()
{
	if (mBanner == 0) {
		return 0;
	}
	return (mBannerTlut ? 0 : 1) + 1;
}

// 0x801A2AD4
int CardImage::iconFormat()
{
	return (mIconTlut ? 0 : 1) + 1;
}

// 0x801A2AE8
int CardImage::iconSpeed()
{
	switch (mSpeed) {
	default:
		return 3;
	case 1:
		return 2;
	case 0:
		return 1;
	}
}

// 0x801A2B18
int CardImage::animMode()
{
	switch (mAnim) {
	default:
		return 0;
	case 1:
		return 4;
	}
}

typedef char CardImage_size_check[(sizeof(CardImage) == 0x30) ? 1 : -1];
