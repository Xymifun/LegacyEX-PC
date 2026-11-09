#ifndef GAME_CLIENT_RENDER_SPIDERRENDERER_HPP
#define GAME_CLIENT_RENDER_SPIDERRENDERER_HPP

#include <boost/shared_ptr.hpp>
#include <vector>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;
class TexturePack;
class RenderLayer;

class SpiderRenderer : public MobRenderer
{
public:
    SpiderRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~SpiderRenderer() {}

    virtual float getFlipDegrees(boost::shared_ptr<LivingEntity>& entity);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual void setupForTexturePack(TexturePack* pack);
    virtual Model* createModel(const ModelDefinition& definition);
    virtual void renderModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                             float ageInTicks, float netHeadYaw, float headPitch, float scale);

private:
    RenderLayer* mSpiderEyesLayer;
};

#endif
