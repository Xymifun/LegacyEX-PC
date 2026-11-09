#ifndef GAME_CLIENT_RENDER_GHASTRENDERER_HPP
#define GAME_CLIENT_RENDER_GHASTRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class GhastRenderer : public MobRenderer
{
public:
    GhastRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~GhastRenderer() {}

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTick);
};

#endif
