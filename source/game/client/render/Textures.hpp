#ifndef GAME_CLIENT_RENDER_TEXTURES_HPP
#define GAME_CLIENT_RENDER_TEXTURES_HPP

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/CriticalSection.hpp"
#include "game/client/render/C4JRender.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class BufferedImage;
class Entity;
class HttpTexture;
class HttpTextureProcessor;
class ItemEntity;
class MemTexture;
class MemTextureProcessor;
class Options;
class TextureAtlas;
class TextureAtlasSprite;
class TexturePackRepository;

extern ResourceLocation BLOCKS_LOCATION;
extern ResourceLocation ITEM_LOCATION;

class Textures : public boost::unordered_map<std::wstring, int>
{
public:
    Textures(TexturePackRepository* texturePackRepository, Options* options);

    static bool IsTUImage(_TEXTURE_NAME textureName, const std::wstring& name);
    static bool IsOriginalImage(_TEXTURE_NAME textureName, const std::wstring& name);
    BufferedImage* readImage(_TEXTURE_NAME textureName, const std::wstring& name);
    void setTextureFormat(const std::wstring& name);
    arrayWithLength<int> anaglyph(arrayWithLength<int> pixels);
    void loadTexture(BufferedImage* image, int texture, bool blur, bool clamp);
    int loadTexture(_TEXTURE_NAME textureName, const std::wstring& name);
    void loadIndexedTextures();
    arrayWithLength<int> loadTexturePixels(BufferedImage* image, arrayWithLength<int> pixels);
    arrayWithLength<int> loadTexturePixels(BufferedImage* image);
    arrayWithLength<int> loadTexturePixels(_TEXTURE_NAME textureName, const std::wstring& name);
    void bind(int texture);
    void bindTexture(const std::wstring& name);
    int loadTexture(int textureName);
    void bindTexture(const ResourceLocation* location);
    void bindTextureLayers(const ResourceLocation* location);
    ResourceLocation* getTextureLocation(int textureName);
    ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    void clearLastBoundId();
    void loadTexture(BufferedImage* image, int texture);
    int getTexture(BufferedImage* image, C4JRender::eTextureFormat format, bool mipmap);
    void replaceTexture(arrayWithLength<int> pixels, int width, int height, int texture);
    void replaceTextureDirect(arrayWithLength<int> pixels, int width, int height, int texture);
    void replaceTextureDirect(arrayWithLength<short> pixels, int width, int height, int texture);
    void releaseTexture(int texture);
    intptr_t addMemTexture(const std::wstring& name, MemTextureProcessor* processor);
    int loadMemTexture(const std::wstring& name, const std::wstring& path);
    int loadMemTexture(const std::wstring& name, int texture);
    void removeMemTexture(const std::wstring& name);
    void tick(bool force, bool unused, bool reload);
    void freeStitch();
    void stitch();
    void reloadAll();
    TextureAtlasSprite* getMissingIcon(int atlas);

    int loadHttpTexture(const std::wstring& name, const std::wstring& path);
    int loadHttpTexture(const std::wstring& name, int texture);
    bool hasHttpTexture(const std::wstring& name);
    void addHttpTexture(const std::wstring& name, HttpTextureProcessor* processor);
    void removeHttpTexture(const std::wstring& name);
    void handleTextureMemoryPanic();

private:
    boost::unordered_map<std::wstring, arrayWithLength<int> > mTexturePixels;
    boost::unordered_map<int, BufferedImage*> mTextureImages;
    boost::unordered_map<std::wstring, HttpTexture*> mHttpTextures;
    boost::unordered_map<std::wstring, MemTexture*> mMemTextures;
    OSFastMutex mMutex;
    Options* mOptions;
    TexturePackRepository* mTexturePackRepository;
    BufferedImage* mMissingImage;
    TextureAtlas* mBlocks;
    TextureAtlas* mItems;
    int mLastBoundId;
    bool mReloading;
    bool mLoaded;
};

#endif
