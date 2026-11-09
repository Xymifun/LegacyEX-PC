#ifndef GAME_CLIENT_RENDER_SQUIDRENDERER_HPP
#define GAME_CLIENT_RENDER_SQUIDRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class SquidRenderer : public MobRenderer
{
public:
    SquidRenderer(EntityRenderDispatcher* dispatcher);

    virtual void setupRotations(boost::shared_ptr<LivingEntity>& entity, float ageInTicks, float rotationYaw,
                                float partialTicks);
    virtual float getBob(boost::shared_ptr<LivingEntity>& entity, float partialTicks);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
