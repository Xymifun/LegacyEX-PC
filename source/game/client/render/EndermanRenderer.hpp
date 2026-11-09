#ifndef GAME_CLIENT_RENDER_ENDERMANRENDERER_HPP
#define GAME_CLIENT_RENDER_ENDERMANRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"
#include "game/util/Random.hpp"

class Entity;
class EntityRenderDispatcher;
class EnderEyesLayer;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;
class TexturePack;

class EndermanRenderer : public MobRenderer
{
public:
    EndermanRenderer(EntityRenderDispatcher* dispatcher);

    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual void setupForTexturePack(TexturePack* pack);
    virtual void renderModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                             float ageInTicks, float netHeadYaw, float headPitch, float scale);
    virtual Model* createModel(const ModelDefinition& definition);

private:
    Random mRandom;
    EnderEyesLayer* mEyesLayer;
};

#endif
