#ifndef GAME_CLIENT_RENDER_MUSHROOMCOWRENDERER_HPP
#define GAME_CLIENT_RENDER_MUSHROOMCOWRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class Model;
class ModelDefinition;
class ResourceLocation;

class MushroomCowRenderer : public MobRenderer
{
public:
    MushroomCowRenderer(EntityRenderDispatcher* dispatcher);

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
