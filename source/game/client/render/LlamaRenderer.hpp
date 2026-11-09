#ifndef GAME_CLIENT_RENDER_LLAMARENDERER_HPP
#define GAME_CLIENT_RENDER_LLAMARENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class Model;
class ModelDefinition;
class ResourceLocation;

class LlamaRenderer : public MobRenderer
{
public:
    LlamaRenderer(EntityRenderDispatcher* dispatcher);

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
