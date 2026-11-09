#ifndef GAME_CLIENT_RENDER_STRAYRENDERER_HPP
#define GAME_CLIENT_RENDER_STRAYRENDERER_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/SkeletonRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;
class StrayClothingLayer;

class StrayRenderer : public SkeletonRenderer
{
public:
    StrayRenderer(EntityRenderDispatcher* dispatcher);

    virtual std::wstring getModelName();
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual void renderLayers(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                              float partialTick, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                              bool overlay);
    virtual Model* createModel(const ModelDefinition& definition);

private:
    StrayClothingLayer* mStrayClothingLayer;
};

#endif
