#ifndef GAME_CLIENT_RENDER_TNTRENDERER_HPP
#define GAME_CLIENT_RENDER_TNTRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class ResourceLocation;

class TntRenderer : public EntityRenderer
{
public:
    TntRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~TntRenderer();

    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
};

#endif
