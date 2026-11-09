#ifndef GAME_CLIENT_RENDER_TEXTUREHOLDER_HPP
#define GAME_CLIENT_RENDER_TEXTUREHOLDER_HPP

#include <string>

struct Texture;

class TextureHolder
{
public:
    TextureHolder(Texture* texture);

    Texture* getTexture();
    int getWidth() const;
    int getHeight() const;
    void rotate();
    bool isRotated();

private:
    int smallestFittingMinTexel(int input) const;

public:
    void setForcedScale(int targetSize);
    std::wstring toString();
    int compareTo(const TextureHolder* other) const;

private:
    Texture* texture;
    int width;
    int height;
    bool rotated;
    float scale;
};

struct TextureHolderLessThan
{
    inline bool operator()(const TextureHolder* first, const TextureHolder* second) const
    {
        return first->compareTo(second) >= 0;
    }
};

#endif
