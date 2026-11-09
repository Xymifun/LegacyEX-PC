#ifndef GAME_CLIENT_RENDER_UNDEADHORSERENDERER_HPP
#define GAME_CLIENT_RENDER_UNDEADHORSERENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/AbstractHorseRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class Model;
class ModelDefinition;
class ResourceLocation;

class UndeadHorseRenderer : public AbstractHorseRenderer
{
public:
    UndeadHorseRenderer(EntityRenderDispatcher* dispatcher);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
