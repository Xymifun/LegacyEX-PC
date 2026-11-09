#ifndef GAME_CLIENT_RENDER_SHEEPRENDERER_HPP
#define GAME_CLIENT_RENDER_SHEEPRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;
class Sheep;
class SheepFurModel;
class TexturePack;

class SheepRenderer : public MobRenderer
{
public:
    SheepRenderer(EntityRenderDispatcher* dispatcher, Model* model, Model* furModel, float shadowRadius);

    virtual Model* getModel(boost::shared_ptr<LivingEntity>& entity);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    void setColourState(boost::shared_ptr<Sheep> sheep, float partialTicks);
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual void renderLayers(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                              float partialTick, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                              bool overlay);
    virtual void setupForTexturePack(TexturePack* pack);
    virtual Model* createModel(const ModelDefinition& definition);

    SheepFurModel* mFurModel;
    SheepFurModel* mOriginalFurModel;
};

#endif
