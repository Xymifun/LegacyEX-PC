#ifndef GAME_CLIENT_RENDER_MEMTEXTURE_HPP
#define GAME_CLIENT_RENDER_MEMTEXTURE_HPP

#include <string>

class BufferedImage;
class MemTextureProcessor;
class Textures;

class MemTexture
{
    friend class Textures;

private:
    BufferedImage* m_image;
    int m_referenceCount;
    int m_textureId;
    bool m_loaded;
    int m_textureFormat;
    bool m_textureReady;

public:
    MemTexture(const std::wstring& name, unsigned char* data, unsigned int length, MemTextureProcessor* processor);
    ~MemTexture();
};

#endif
