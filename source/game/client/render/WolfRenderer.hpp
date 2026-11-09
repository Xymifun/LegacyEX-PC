#ifndef GAME_CLIENT_RENDER_WOLFRENDERER_HPP
#define GAME_CLIENT_RENDER_WOLFRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class RenderLayer;
class ResourceLocation;
class TexturePack;

class WolfRenderer : public MobRenderer
{
public:
    WolfRenderer(EntityRenderDispatcher* dispatcher, Model* model, float shadowRadius);

    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual void setupForTexturePack(TexturePack* pack);
    virtual Model* createModel(const ModelDefinition& definition);
    virtual float getBob(boost::shared_ptr<LivingEntity>& entity, float partialTicks);

private:
    RenderLayer* mCollarLayer;
};

#endif
