#ifndef GAME_CLIENT_RENDER_TEXTUREUVCOORDINATESET_HPP
#define GAME_CLIENT_RENDER_TEXTUREUVCOORDINATESET_HPP
class TextureUVCoordinateSet
{
public:
    float scale;
    float u0;
    float v0;
    float u1;
    float v1;
    unsigned short textureWidth;
    unsigned short textureHeight;
    TextureUVCoordinateSet(float, float, float, float, unsigned short, unsigned short, float = 1.0f);
    TextureUVCoordinateSet();
    static TextureUVCoordinateSet fromOldSystem(int);
    float width() const;
    float height() const;
    float offsetWidth(float, float) const;
    float offsetHeight(float, float) const;
    float asPixelU1() const;
    float asPixelU0() const;
    float pixelWidth() const;
    float offsetWidthPixel(float, float) const;
    float asPixelV1() const;
    float asPixelV0() const;
    float pixelHeight() const;
    float offsetHeightPixel(float, float) const;
    float inverseOffsetWidth(float, float) const;
    float inverseOffsetHeight(float, float) const;
    TextureUVCoordinateSet getFlippedHorizontal() const;
    TextureUVCoordinateSet getFlippedVertically() const;
    TextureUVCoordinateSet offset(float) const;
    float pixelSizeU() const;
    float pixelSizeV() const;
    TextureUVCoordinateSet subTexture(float, float, int, int) const;
    TextureUVCoordinateSet topLeftPixel() const;
    int hashCode() const;
    bool operator==(const TextureUVCoordinateSet&) const;
};
#endif
