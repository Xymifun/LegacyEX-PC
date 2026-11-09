#ifndef GAME_CLIENT_RENDER_ITEMENTITYRENDERER_HPP
#define GAME_CLIENT_RENDER_ITEMENTITYRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"
#include "game/util/Random.hpp"
#include "game/item/not_null_ptr.hpp"

class Entity;
class EntityRenderDispatcher;
class ItemEntity;
class ItemInstance;
class ItemRenderer;
class ResourceLocation;
class TextureAtlasSprite;

class ItemEntityRenderer : public EntityRenderer
{
public:
    ItemEntityRenderer(EntityRenderDispatcher* dispatcher, ItemRenderer* itemRenderer);
    virtual ~ItemEntityRenderer();

    void renderItemBillboard(boost::shared_ptr<ItemEntity> entity, TextureAtlasSprite* sprite, int count,
                             float partialTick, float red, float green, float blue);
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual void SetItemFrame(bool itemFrame);
    int getRenderAmount(not_null_ptr<ItemInstance> item);

private:
    ItemRenderer* mItemRenderer;
    Random mRandom;
    bool mItemFrame;
    bool mIsGui3d;
};

#endif
