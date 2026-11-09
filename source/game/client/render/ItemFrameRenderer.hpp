#ifndef GAME_CLIENT_RENDER_ITEMFRAMERENDERER_HPP
#define GAME_CLIENT_RENDER_ITEMFRAMERENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"

class EntityRenderDispatcher;
class IconRegister;
class ItemFrame;
class ItemRenderer;
class Minecraft;
class ResourceLocation;
class TextureAtlasSprite;

class ItemFrameRenderer : public EntityRenderer
{
public:
    ItemFrameRenderer(EntityRenderDispatcher* dispatcher, ItemRenderer* itemRenderer);
    virtual ~ItemFrameRenderer();

    void registerTerrainTextures(IconRegister* iconRegister);
    void drawBlockFrame(const boost::shared_ptr<ItemFrame>& itemFrame);
    void drawFrame(const boost::shared_ptr<ItemFrame>& itemFrame);
    void drawItem(const boost::shared_ptr<ItemFrame>& itemFrame);
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual void renderName(boost::shared_ptr<Entity>& entity, double x, double y, double z);

private:
    static ResourceLocation MAP_BACKGROUND_LOCATION;

    TextureAtlasSprite* mFrameIcon;
    Minecraft* mMinecraft;
    ItemRenderer* mItemRenderer;
    int mBlockFrameDisplayList;
    int mFrameDisplayList;
};

#endif
