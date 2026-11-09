#ifndef GAME_CLIENT_RENDER_ELDERGUARDIANRENDERER_HPP
#define GAME_CLIENT_RENDER_ELDERGUARDIANRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/GuardianRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class ResourceLocation;

class ElderGuardianRenderer : public GuardianRenderer
{
public:
    ElderGuardianRenderer(EntityRenderDispatcher* dispatcher);

    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
};

#endif
