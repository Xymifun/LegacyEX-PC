#include "game/client/render/MemTexture.hpp"
#include "game/core/BufferedImage.hpp"

MemTexture::MemTexture(const std::wstring& name, unsigned char* data, unsigned int length,
                       MemTextureProcessor* processor)
{
    m_referenceCount = 1;
    m_textureId = -1;
    m_loaded = false;
    m_textureFormat = 0;
    m_textureReady = false;
    m_image = new BufferedImage(data, length);
    (void)name;
    (void)processor;
}

MemTexture::~MemTexture()
{
    delete m_image;
}
