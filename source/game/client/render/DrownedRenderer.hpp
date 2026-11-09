#ifndef GAME_CLIENT_RENDER_DROWNEDRENDERER_HPP
#define GAME_CLIENT_RENDER_DROWNEDRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/HumanoidMobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;
class ZombieArmorLayer;

class DrownedRenderer : public HumanoidMobRenderer
{
public:
    DrownedRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~DrownedRenderer();

    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks);
    virtual Model* createModel(const ModelDefinition& definition);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual void setupRotations(boost::shared_ptr<LivingEntity>& entity, float ageInTicks, float rotationYaw,
                                float partialTicks);

private:
    ZombieArmorLayer* mArmorLayer;
};

#endif
