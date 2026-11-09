#ifndef GAME_CLIENT_RENDER_LLAMASPITRENDERER_HPP
#define GAME_CLIENT_RENDER_LLAMASPITRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class ModelDefinition;
class LlamaSpitModel;
class ResourceLocation;

class LlamaSpitRenderer : public EntityRenderer
{
public:
    LlamaSpitRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~LlamaSpitRenderer();
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);

private:
    LlamaSpitModel* mSpitModel;
};

#endif
