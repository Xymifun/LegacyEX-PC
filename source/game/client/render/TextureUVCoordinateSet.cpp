#include "game/client/render/TextureUVCoordinateSet.hpp"
TextureUVCoordinateSet::TextureUVCoordinateSet(float a, float b, float c, float d, unsigned short w, unsigned short h,
                                               float s)
    : scale(s),
      u0(a),
      v0(b),
      u1(c),
      v1(d),
      textureWidth(w),
      textureHeight(h)
{
}
TextureUVCoordinateSet::TextureUVCoordinateSet()
    : scale(1.0f),
      u0(0.0f),
      v0(0.0f),
      u1(0.0f),
      v1(0.0f),
      textureWidth(1),
      textureHeight(1)
{
}
TextureUVCoordinateSet TextureUVCoordinateSet::fromOldSystem(int i)
{
    float x = (float)(i % 16), y = (float)(i / 16);
    return TextureUVCoordinateSet(x * .0625f, y * .0625f, (x + 1) * .0625f, (y + 1) * .0625f, 16, 16);
}
