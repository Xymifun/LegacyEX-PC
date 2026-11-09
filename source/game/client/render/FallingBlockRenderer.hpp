#ifndef GAME_CLIENT_RENDER_FALLINGBLOCKRENDERER_HPP
#define GAME_CLIENT_RENDER_FALLINGBLOCKRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class ResourceLocation;

class FallingBlockRenderer : public EntityRenderer
{
public:
    FallingBlockRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~FallingBlockRenderer();

    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
};

#endif
