#ifndef GAME_CLIENT_RENDER_BOATRENDERER_HPP
#define GAME_CLIENT_RENDER_BOATRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"

class Boat;
class Culler;
class Entity;
class EntityRenderDispatcher;
class Model;
class ModelDefinition;
class ResourceLocation;

class BoatRenderer : public EntityRenderer
{
public:
    BoatRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~BoatRenderer();
    void setupTranslation(double x, double y, double z);
    void setupRotation(boost::shared_ptr<Entity> entity, float yaw, float partialTick);
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual bool hasSecondPass();
    virtual void renderSecondPass(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                                  float partialTick);
    double lerp(double oldValue, double newValue, double partialTick);
    virtual void renderLeash(boost::shared_ptr<Boat>& boat, double x, double y, double z, float yaw, float partialTick);
    virtual bool shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double x, double y, double z);
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
