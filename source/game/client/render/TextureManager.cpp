#include "game/client/render/TextureManager.hpp"

#include <wchar.h>
#include <utility>

#include "game/core/BufferedImage.hpp"
#include "game/core/File.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/Minecraft.hpp"
#include "game/client/render/Texture.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcGoldenAge.hpp"
#endif

static TextureManager* g_textureManager = NULL;

TextureManager::TextureManager()
{
    m_nextTextureId = 0;
}

void TextureManager::unregisterTexture(const std::wstring& name, Texture* texture)
{
    const unsigned int managerId = texture->getManagerId();
    boost::unordered_map<int, Texture*>::iterator textureIt = m_textures.find(static_cast<int>(managerId));
    if (textureIt != m_textures.end())
    {
        m_textures.erase(textureIt);
    }

    boost::unordered_map<std::wstring, int>::iterator nameIt = m_textureNames.find(name);
    if (nameIt != m_textureNames.end())
    {
        m_textureNames.erase(nameIt);
    }
}

TextureManager* TextureManager::getInstance()
{
    return g_textureManager;
}

void TextureManager::createInstance()
{
    g_textureManager = new TextureManager();
}

unsigned int TextureManager::createTextureID()
{
    return m_nextTextureId++;
}

void TextureManager::registerName(const std::wstring& name, Texture* texture)
{
    m_textureNames.insert(std::pair<const std::wstring, int>(name, static_cast<int>(texture->getManagerId())));

    if (m_textures.find(static_cast<int>(texture->getManagerId())) == m_textures.end())
    {
        m_textures.insert(std::pair<const int, Texture*>(texture->getManagerId(), texture));
    }
}

void TextureManager::registerTexture(Texture* texture)
{
    for (boost::unordered_map<int, Texture*>::iterator it = m_textures.begin(); it != m_textures.end(); ++it)
    {
        if (it->second == texture)
        {
            return;
        }
    }
    m_textures.insert(std::pair<const int, Texture*>(texture->getManagerId(), texture));
}

Texture* TextureManager::createTexture(const std::wstring& name, int type, int width, int height, int wrap, int format,
                                       int minFilter, int magFilter, bool mipmap, BufferedImage* image)
{
    Texture* texture = new Texture(name, type, width, height, wrap, format, minFilter, magFilter, image, mipmap);
    registerTexture(texture);
    return texture;
}

std::wstring TextureManager::getTextureNameFromPath(const std::wstring& path)
{
    File file(path);
    return file.getName().substr(0, file.getName().rfind(L'.'));
}

bool TextureManager::isAnimation(const std::wstring& name, TexturePack* texturePack)
{
    std::wstring textPath = L"/" + name.substr(0, name.rfind(L'.')) + L".txt";
    bool hasPlainFile = texturePack->hasFile(L"/" + name, false);
    TexturePack* selected = Minecraft::GetInstance()->mTexturePackRepository->getSelected();
    return selected->hasFile(textPath, !hasPlainFile);
}

std::vector<Texture*>* TextureManager::createTextures(const std::wstring& path, bool singleImage)
{
    std::vector<Texture*>* textures = new std::vector<Texture*>;
    TexturePack* texturePack = Minecraft::GetInstance()->mTexturePackRepository->getSelected();
    MemSect(32);
    std::wstring packPath = L"";
    const bool hasResource = texturePack->hasFile(L"res/" + path, false);
    if (hasResource)
    {
        packPath = texturePack->getPath(true, 0);
    }
    else
    {
        packPath = Minecraft::GetInstance()->mTexturePackRepository->defaultTexturePack->getPath(true, 0);
        texturePack = Minecraft::GetInstance()->mTexturePackRepository->defaultTexturePack;
    }
    BufferedImage* image = texturePack->getImageResource(path, false, true, packPath);
    MemSect(0);
#ifdef MINECRAFT_PC
    PcGoldenAgeRecolourAnimation(path, image);
    image = PcGoldenAgeReplaceAnimation(path, image);
#endif

    const int height = image->getHeight();
    const int width = image->getWidth();
    const std::wstring textureName = getTextureNameFromPath(path);
    const bool animation = isAnimation(path, texturePack);

    if (animation)
    {
        const int frameWidth = width;
        const int frameHeight = width;
        const int frameCount = height / frameWidth;
        for (int frame = 0; frame < frameCount; ++frame)
        {
            BufferedImage* frameImage = image->getSubimage(0, frameHeight * frame, frameWidth, frameHeight);
            bool mipmap = singleImage || image->getData(1) != NULL;
            Texture* frameTexture = createTexture(textureName, 2, frameWidth, frameHeight, 1, 0, 0, 0, mipmap,
                                                  frameImage);
            delete frameImage;
            textures->push_back(frameTexture);
        }
    }
    else if (width == height)
    {
        bool mipmap = singleImage || image->getData(1) != NULL;
        textures->push_back(createTexture(textureName, 2, width, height, 1, 0, 0, 0, mipmap, image));
    }

    delete image;
    return textures;
}

Texture* TextureManager::createTexture(const std::wstring& name, int type, int width, int height, int format,
                                       bool mipmap)
{
    return createTexture(name, type, width, height, 1, format, 0, 0, mipmap, NULL);
}
