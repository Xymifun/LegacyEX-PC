#ifndef GAME_CLIENT_RENDER_WITHERSKULLRENDERER_HPP
#define GAME_CLIENT_RENDER_WITHERSKULLRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"
#include "game/client/render/SkeletonHeadModel.hpp"

class Entity;
class EntityRenderDispatcher;
class Model;
class ModelDefinition;
class ResourceLocation;

class WitherSkullRenderer : public EntityRenderer
{
public:
    WitherSkullRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~WitherSkullRenderer();

    float rotlerp(float from, float to, float partialTick);
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);

private:
    SkeletonHeadModel* mWitherSkullModel;
};

#endif
