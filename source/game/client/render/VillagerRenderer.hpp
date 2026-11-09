#ifndef GAME_CLIENT_RENDER_VILLAGERRENDERER_HPP
#define GAME_CLIENT_RENDER_VILLAGERRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class VillagerRenderer : public MobRenderer
{
public:
    VillagerRenderer(EntityRenderDispatcher* dispatcher);

    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* getModel();
    virtual Model* createModel(const ModelDefinition& definition);
    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks);
};

#endif
