#ifndef GAME_CLIENT_RENDER_THROWNTRIDENTRENDERER_HPP
#define GAME_CLIENT_RENDER_THROWNTRIDENTRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class Model;
class ModelDefinition;
class ResourceLocation;
class ThrownTrident;

class ThrownTridentRenderer : public EntityRenderer
{
public:
    ThrownTridentRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~ThrownTridentRenderer();

    void renderFoil(boost::shared_ptr<Entity>& entity);
    double lerp(double first, double second, double amount);
    void renderLeash(boost::shared_ptr<ThrownTrident>& entity, double x, double y, double z, float yaw,
                     float partialTick);
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
