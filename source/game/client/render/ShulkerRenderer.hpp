#ifndef GAME_CLIENT_RENDER_SHULKERRENDERER_HPP
#define GAME_CLIENT_RENDER_SHULKERRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class ShulkerRenderer : public MobRenderer
{
public:
    ShulkerRenderer(EntityRenderDispatcher* dispatcher);
    virtual bool shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double x, double y, double z);
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
    virtual void renderModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                             float ageInTicks, float netHeadYaw, float headPitch, float scale);
    virtual void setupRotations(boost::shared_ptr<LivingEntity>& entity, float ageInTicks, float bodyRot,
                                float partialTick);
    virtual void renderLayers(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                              float partialTick, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                              bool overlay);
    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTick);
};

#endif
