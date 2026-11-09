#ifndef GAME_CLIENT_RENDER_SNOWMANRENDERER_HPP
#define GAME_CLIENT_RENDER_SNOWMANRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class Model;
class ModelDefinition;
class ResourceLocation;

class SnowManRenderer : public MobRenderer
{
public:
    SnowManRenderer(EntityRenderDispatcher* dispatcher);

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* getModel();
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
