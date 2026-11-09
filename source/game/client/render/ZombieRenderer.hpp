#ifndef GAME_CLIENT_RENDER_ZOMBIERENDERER_HPP
#define GAME_CLIENT_RENDER_ZOMBIERENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/HumanoidMobRenderer.hpp"

class Entity;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class ZombieRenderer : public HumanoidMobRenderer
{
public:
    ZombieRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~ZombieRenderer() {}

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
    virtual void setupRotations(boost::shared_ptr<LivingEntity>& entity, float ageInTicks, float rotationYaw,
                                float partialTicks);
};

#endif
