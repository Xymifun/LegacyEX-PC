#ifndef GAME_CLIENT_RENDER_EXPERIENCEORBRENDERER_HPP
#define GAME_CLIENT_RENDER_EXPERIENCEORBRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class ResourceLocation;

class ExperienceOrbRenderer : public EntityRenderer
{
public:
    ExperienceOrbRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~ExperienceOrbRenderer();
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    void blit(int x, int y, int u, int v, int width, int height);
};

#endif
