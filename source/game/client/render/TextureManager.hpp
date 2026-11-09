#ifndef GAME_CLIENT_RENDER_TEXTUREMANAGER_HPP
#define GAME_CLIENT_RENDER_TEXTUREMANAGER_HPP

#include <string>
#include <vector>

#include <boost/unordered_map.hpp>

class BufferedImage;
class LayeredColorMaskTexture;
class ResourceLocation;
class Stitcher;
struct Texture;
class TexturePack;

class TextureManager
{
public:
    TextureManager();

    void unregisterTexture(const std::wstring& name, Texture* texture);
    static TextureManager* getInstance();
    static void createInstance();
    unsigned int createTextureID();
    void registerName(const std::wstring& name, Texture* texture);
    void registerTexture(Texture* texture);
    Texture* createTexture(const std::wstring& name, int type, int width, int height, int wrap, int format,
                           int minFilter, int magFilter, bool mipmap, BufferedImage* image);
    std::wstring getTextureNameFromPath(const std::wstring& path);
    bool isAnimation(const std::wstring& name, TexturePack* texturePack);
    std::vector<Texture*>* createTextures(const std::wstring& path, bool singleImage);
    Texture* createTexture(const std::wstring& name, int type, int width, int height, int format, bool mipmap);
    Texture* getTexture(const std::wstring& name);
    void registerTexture(const ResourceLocation* location, LayeredColorMaskTexture* texture);
    void registerTexture(const ResourceLocation* location, Texture* texture);
    Stitcher* createStitcher(const std::wstring& name);

private:
    int m_nextTextureId;
    boost::unordered_map<int, Texture*> m_textures;
    boost::unordered_map<std::wstring, int> m_textureNames;
};

#endif
