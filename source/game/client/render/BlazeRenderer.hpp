#ifndef GAME_CLIENT_RENDER_BLAZERENDERER_HPP
#define GAME_CLIENT_RENDER_BLAZERENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class Model;
class ModelDefinition;
class ResourceLocation;

class BlazeRenderer : public MobRenderer
{
public:
    BlazeRenderer(EntityRenderDispatcher* dispatcher);

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
