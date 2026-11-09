#ifndef GAME_CLIENT_RENDER_RABBITRENDERER_HPP
#define GAME_CLIENT_RENDER_RABBITRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class Model;
class ModelDefinition;
class ResourceLocation;

class RabbitRenderer : public MobRenderer
{
public:
    RabbitRenderer(EntityRenderDispatcher* dispatcher);

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
