#ifndef GAME_CLIENT_RENDER_BLOCKENTITYRENDERER_HPP
#define GAME_CLIENT_RENDER_BLOCKENTITYRENDERER_HPP

#include <string>

#include <boost/shared_ptr.hpp>

class BlockEntity;
class BlockEntityRenderDispatcher;
class Font;
class Level;
class ResourceLocation;
class TexturePack;

class BlockEntityRenderer
{
public:
    virtual ~BlockEntityRenderer() {}
    virtual void render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float a,
                        int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList);
    virtual void onNewLevel(Level* pLevel);
    virtual void init(BlockEntityRenderDispatcher* pDispatcher);
    virtual bool shouldRenderOffScreen(boost::shared_ptr<BlockEntity> blockEntity);
    virtual void setupForTexturePack(TexturePack* pTexturePack);

    void setOverlayRenderState(unsigned char overlayRenderState);

    static ResourceLocation BREAKING_LOCATIONS[10];

protected:
    Font* getFont();
    Level* getLevel();

    void renderNameTag(boost::shared_ptr<BlockEntity> blockEntity, std::wstring name, double x, double y, double z,
                       int maxDistance);

    void bindTexture(ResourceLocation* pResourceLocation);
    void bindTexture(const std::wstring& texturePath, ResourceLocation* pResourceLocation);
    void bindTextureLayers(ResourceLocation* pResourceLocation);

    BlockEntityRenderDispatcher* mDispatcher;
};

#endif
