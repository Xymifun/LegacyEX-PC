#ifndef GAME_CLIENT_RENDER_PUFFERFISHRENDERER_HPP
#define GAME_CLIENT_RENDER_PUFFERFISHRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/FishRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class TexturePack;

class PufferfishRenderer : public FishRenderer
{
public:
    PufferfishRenderer(EntityRenderDispatcher* dispatcher);

    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual Model* getModel(boost::shared_ptr<LivingEntity>& entity);
    virtual void setupForTexturePack(TexturePack* pack);
    virtual Model* createModel(const ModelDefinition& definition);

private:
    int mPuffState;
    Model* mModelMedium;
    Model* mModelLarge;
    Model* mOriginalModelMedium;
    Model* mOriginalModelLarge;
};

#endif
