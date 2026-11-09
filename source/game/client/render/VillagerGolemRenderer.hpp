#ifndef GAME_CLIENT_RENDER_VILLAGERGOLEMRENDERER_HPP
#define GAME_CLIENT_RENDER_VILLAGERGOLEMRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class VillagerGolemRenderer : public MobRenderer
{
public:
    VillagerGolemRenderer(EntityRenderDispatcher* dispatcher);

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
    virtual void setupRotations(boost::shared_ptr<LivingEntity>& entity, float ageInTicks, float rotationYaw,
                                float partialTicks);
};

#endif
