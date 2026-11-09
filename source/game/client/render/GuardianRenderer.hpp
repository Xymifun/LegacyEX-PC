#ifndef GAME_CLIENT_RENDER_GUARDIANRENDERER_HPP
#define GAME_CLIENT_RENDER_GUARDIANRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Culler;
class Entity;
class EntityRenderDispatcher;
class Model;
class ModelDefinition;
class ResourceLocation;
class Vec3;

class GuardianRenderer : public MobRenderer
{
public:
    GuardianRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~GuardianRenderer() {}
    Vec3* getPosition(boost::shared_ptr<Entity>& entity, double yOffset, float partialTicks);
    virtual bool shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double x, double y, double z);
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
