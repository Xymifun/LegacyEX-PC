#ifndef GAME_CLIENT_RENDER_COWRENDERER_HPP
#define GAME_CLIENT_RENDER_COWRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class Model;
class ModelDefinition;
class ResourceLocation;

class CowRenderer : public MobRenderer
{
public:
    CowRenderer(EntityRenderDispatcher* dispatcher, Model* model, float shadowRadius);

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
