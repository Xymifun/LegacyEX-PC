#ifndef GAME_CLIENT_RENDER_HUSKRENDERER_HPP
#define GAME_CLIENT_RENDER_HUSKRENDERER_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/ZombieRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class ResourceLocation;

class HuskRenderer : public ZombieRenderer
{
public:
    HuskRenderer(EntityRenderDispatcher* dispatcher);

    virtual std::wstring getModelName();
    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
};

#endif
