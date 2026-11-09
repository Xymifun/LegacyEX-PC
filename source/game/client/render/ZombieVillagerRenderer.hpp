#ifndef GAME_CLIENT_RENDER_ZOMBIEVILLAGERRENDERER_HPP
#define GAME_CLIENT_RENDER_ZOMBIEVILLAGERRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/HumanoidMobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class ZombieVillagerRenderer : public HumanoidMobRenderer
{
public:
    ZombieVillagerRenderer(EntityRenderDispatcher* dispatcher);

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
    virtual void setupRotations(boost::shared_ptr<LivingEntity>& entity, float ageInTicks, float rotationYaw,
                                float partialTicks);
};

#endif
