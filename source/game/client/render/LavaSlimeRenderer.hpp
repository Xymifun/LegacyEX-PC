#ifndef GAME_CLIENT_RENDER_LAVASLIMERENDERER_HPP
#define GAME_CLIENT_RENDER_LAVASLIMERENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class LavaSlimeRenderer : public MobRenderer
{
public:
    LavaSlimeRenderer(EntityRenderDispatcher* dispatcher);

    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTick);
};

#endif
