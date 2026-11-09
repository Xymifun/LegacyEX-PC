#include <nn/ffl/FFLiMipMapUtil.h>

s32 FFLiGetMipMapNum(s32 width, s32 height)
{
    if (width <= 32u || height <= 32u)
        return 1;

    return FFLiGetMipMapNum(width / 2, height / 2) + 1;
}

u32 FFLiGetMipMapLevelSize(u32 size, u32 mipLevel)
{
    if (mipLevel == 0)
        return size;

    return FFLiGetMipMapLevelSize(size / 2, mipLevel - 1);
}
