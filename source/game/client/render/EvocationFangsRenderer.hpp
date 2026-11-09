#ifndef GAME_CLIENT_RENDER_EVOCATIONFANGSRENDERER_HPP
#define GAME_CLIENT_RENDER_EVOCATIONFANGSRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class Model;
class ModelDefinition;
class EvocationFangsModel;
class ResourceLocation;

class EvocationFangsRenderer : public EntityRenderer
{
public:
    EvocationFangsRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~EvocationFangsRenderer();

    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);

private:
    EvocationFangsModel* mFangsModel;
};

#endif
