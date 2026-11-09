#ifndef GAME_CLIENT_RENDER_VEXRENDERER_HPP
#define GAME_CLIENT_RENDER_VEXRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/HumanoidMobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class VexRenderer : public HumanoidMobRenderer
{
public:
    VexRenderer(EntityRenderDispatcher* dispatcher);

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual Model* createModel(const ModelDefinition& definition);
    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTick);

protected:
    unsigned int mModelVersion;
};

#endif
