#ifndef GAME_CLIENT_RENDER_LEASHKNOTRENDERER_HPP
#define GAME_CLIENT_RENDER_LEASHKNOTRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"
#include "game/client/render/LeashKnotModel.hpp"

class Entity;
class EntityRenderDispatcher;
class Model;
class ModelDefinition;
class ResourceLocation;

class LeashKnotRenderer : public EntityRenderer
{
public:
    LeashKnotRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~LeashKnotRenderer();
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);

private:
    LeashKnotModel* mKnotModel;
};

#endif
