#ifndef GAME_CLIENT_RENDER_DRAGONFIREBALLRENDERER_HPP
#define GAME_CLIENT_RENDER_DRAGONFIREBALLRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class ResourceLocation;

class DragonFireballRenderer : public EntityRenderer
{
public:
    DragonFireballRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~DragonFireballRenderer();
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
};

#endif
