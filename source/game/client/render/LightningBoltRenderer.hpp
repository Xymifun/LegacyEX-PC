#ifndef GAME_CLIENT_RENDER_LIGHTNINGBOLTRENDERER_HPP
#define GAME_CLIENT_RENDER_LIGHTNINGBOLTRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"

class Entity;
class EntityRenderDispatcher;

class LightningBoltRenderer : public EntityRenderer
{
public:
    LightningBoltRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~LightningBoltRenderer();
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
};

#endif
