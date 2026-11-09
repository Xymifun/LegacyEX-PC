#ifndef GAME_CLIENT_RENDER_CHICKENRENDERER_HPP
#define GAME_CLIENT_RENDER_CHICKENRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class ChickenRenderer : public MobRenderer
{
public:
    ChickenRenderer(EntityRenderDispatcher* dispatcher, Model* model, float shadowRadius);

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual float getBob(boost::shared_ptr<LivingEntity>& entity, float partialTicks);
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
