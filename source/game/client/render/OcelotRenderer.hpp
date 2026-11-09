#ifndef GAME_CLIENT_RENDER_OCELOTRENDERER_HPP
#define GAME_CLIENT_RENDER_OCELOTRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class OcelotRenderer : public MobRenderer
{
public:
    OcelotRenderer(EntityRenderDispatcher* dispatcher);

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks);
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
