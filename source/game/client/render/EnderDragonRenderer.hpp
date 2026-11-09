#ifndef GAME_CLIENT_RENDER_ENDERDRAGONRENDERER_HPP
#define GAME_CLIENT_RENDER_ENDERDRAGONRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class EnderDragonEyesLayer;
class DragonModel;
class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;
class TexturePack;

class EnderDragonRenderer : public MobRenderer
{
public:
    EnderDragonRenderer(EntityRenderDispatcher* dispatcher);
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual void setupForTexturePack(TexturePack* pack);
    virtual Model* createModel(const ModelDefinition& definition);
    virtual void renderModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                             float ageInTicks, float netHeadYaw, float headPitch, float scale);
    virtual void setupRotations(boost::shared_ptr<LivingEntity>& entity, float ageInTicks, float rotationYaw,
                                float partialTicks);

    static void renderCrystalBeams(double x, double y, double z, float partialTick, double targetX, double targetY,
                                   double targetZ, int time, double crystalX, double crystalY, double crystalZ);

private:
    DragonModel* mDragonModel;
    EnderDragonEyesLayer* mEyesLayer;
};

#endif
