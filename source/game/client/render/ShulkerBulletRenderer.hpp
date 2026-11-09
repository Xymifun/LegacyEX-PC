#ifndef GAME_CLIENT_RENDER_SHULKERBULLETRENDERER_HPP
#define GAME_CLIENT_RENDER_SHULKERBULLETRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class Model;
class ModelDefinition;
class ShulkerBulletModel;
class ResourceLocation;

class ShulkerBulletRenderer : public EntityRenderer
{
public:
    ShulkerBulletRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~ShulkerBulletRenderer();

    float rotlerp(float previousRotation, float rotation, float partialTick);
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);

private:
    ShulkerBulletModel* mBulletModel;
};

#endif
