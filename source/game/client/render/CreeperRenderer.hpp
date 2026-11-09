#ifndef GAME_CLIENT_RENDER_CREEPERRENDERER_HPP
#define GAME_CLIENT_RENDER_CREEPERRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class CreeperRenderer : public MobRenderer
{
public:
    CreeperRenderer(EntityRenderDispatcher* dispatcher);

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
    virtual int getOverlayColor(boost::shared_ptr<LivingEntity>& entity, float u, float v);
    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks);
};

#endif
