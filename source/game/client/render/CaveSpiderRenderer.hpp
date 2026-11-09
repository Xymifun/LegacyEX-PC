#ifndef GAME_CLIENT_RENDER_CAVESPIDERRENDERER_HPP
#define GAME_CLIENT_RENDER_CAVESPIDERRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/SpiderRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class ResourceLocation;

class CaveSpiderRenderer : public SpiderRenderer
{
public:
    CaveSpiderRenderer(EntityRenderDispatcher* dispatcher);

    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
};

#endif
