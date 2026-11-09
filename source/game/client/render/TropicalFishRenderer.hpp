#ifndef GAME_CLIENT_RENDER_TROPICALFISHRENDERER_HPP
#define GAME_CLIENT_RENDER_TROPICALFISHRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/FishRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class FishModel;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class TropicalFishRenderer : public FishRenderer
{
public:
    TropicalFishRenderer(EntityRenderDispatcher* dispatcher);

    virtual void bindTexture(const ResourceLocation* location);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
    virtual Model* getModel(boost::shared_ptr<LivingEntity>& entity);
    virtual void renderModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                             float ageInTicks, float netHeadYaw, float headPitch, float scale);

    void getRGB(int color, float& red, float& green, float& blue);

private:
    FishModel* mModel2;
};

#endif
