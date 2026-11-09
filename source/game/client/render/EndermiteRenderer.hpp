#ifndef GAME_CLIENT_RENDER_ENDERMITERENDERER_HPP
#define GAME_CLIENT_RENDER_ENDERMITERENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class EndermiteRenderer : public MobRenderer
{
public:
    EndermiteRenderer(EntityRenderDispatcher* dispatcher);

    virtual float getFlipDegrees(boost::shared_ptr<LivingEntity>& entity);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
