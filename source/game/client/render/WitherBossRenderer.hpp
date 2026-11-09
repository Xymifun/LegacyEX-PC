#ifndef GAME_CLIENT_RENDER_WITHERBOSSRENDERER_HPP
#define GAME_CLIENT_RENDER_WITHERBOSSRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class WitherBossRenderer : public MobRenderer
{
public:
    WitherBossRenderer(EntityRenderDispatcher* dispatcher);

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
    virtual int getOverlayColor(boost::shared_ptr<LivingEntity>& entity, float u, float v);
    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks);
};

#endif
