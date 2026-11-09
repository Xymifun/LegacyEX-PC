#ifndef GAME_CLIENT_RENDER_BATRENDERER_HPP
#define GAME_CLIENT_RENDER_BATRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class BatRenderer : public MobRenderer
{
public:
    BatRenderer(EntityRenderDispatcher* dispatcher);

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
    virtual void setupPosition(boost::shared_ptr<LivingEntity>& entity, double x, double y, double z);
    virtual void setupRotations(boost::shared_ptr<LivingEntity>& entity, float ageInTicks, float rotationYaw,
                                float partialTicks);
    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks);
};

#endif
