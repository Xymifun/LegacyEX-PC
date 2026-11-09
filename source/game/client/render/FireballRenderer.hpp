#ifndef GAME_CLIENT_RENDER_FIREBALLRENDERER_HPP
#define GAME_CLIENT_RENDER_FIREBALLRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"

class Entity;
class ResourceLocation;

class FireballRenderer : public EntityRenderer
{
public:
    FireballRenderer(EntityRenderDispatcher* dispatcher, float scale);
    virtual ~FireballRenderer();
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual void renderFlame(boost::shared_ptr<Entity>& entity, double x, double y, double z, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);

private:
    float mScale;
};

#endif
