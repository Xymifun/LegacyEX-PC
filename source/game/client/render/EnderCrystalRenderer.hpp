#ifndef GAME_CLIENT_RENDER_ENDERCRYSTALRENDERER_HPP
#define GAME_CLIENT_RENDER_ENDERCRYSTALRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"

class Culler;
class EnderCrystalModel;
class Entity;
class EntityRenderDispatcher;
class Model;
class ModelDefinition;
class ResourceLocation;

extern ResourceLocation END_CRYSTAL_BEAM_LOCATION;

class EnderCrystalRenderer : public EntityRenderer
{
public:
    EnderCrystalRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~EnderCrystalRenderer();

    virtual bool shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double x, double y, double z);
    virtual bool shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double distance);
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);

private:
    EnderCrystalModel* mModelBase;
    EnderCrystalModel* mModelNoBase;
};

#endif
