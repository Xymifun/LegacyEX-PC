#ifndef GAME_CLIENT_RENDER_ARROWRENDERER_HPP
#define GAME_CLIENT_RENDER_ARROWRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class ResourceLocation;

class ArrowRenderer : public EntityRenderer
{
public:
    ArrowRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~ArrowRenderer() {}
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
};

#endif
