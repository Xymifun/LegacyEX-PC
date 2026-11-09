#ifndef GAME_CLIENT_RENDER_THROWNITEMRENDERER_HPP
#define GAME_CLIENT_RENDER_THROWNITEMRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"
#include "game/item/not_null_ptr.hpp"

class Entity;
class EntityRenderDispatcher;
class Item;
class ItemInstance;
class Tesselator;
class TextureAtlasSprite;

class ThrownItemRenderer : public EntityRenderer
{
public:
    ThrownItemRenderer(EntityRenderDispatcher* dispatcher, Item* item);
    virtual ~ThrownItemRenderer() {}
    void renderIcon(Tesselator* tesselator, TextureAtlasSprite* sprite);
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual not_null_ptr<ItemInstance> getItem(boost::shared_ptr<Entity> entity);

private:
    Item* mItem;
};

#endif
