#ifndef GAME_CLIENT_RENDER_PIGRENDERER_HPP
#define GAME_CLIENT_RENDER_PIGRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class LivingEntity;
class EntityRenderDispatcher;
class Model;
class ModelDefinition;
class PigSaddleLayer;
class ResourceLocation;
class TexturePack;

class PigRenderer : public MobRenderer
{
public:
    PigRenderer(EntityRenderDispatcher* dispatcher);

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual void setupForTexturePack(TexturePack* pack);
    virtual Model* createModel(const ModelDefinition& definition);
#ifdef MINECRAFT_PC
    using MobRenderer::getModel;
    virtual Model* getModel(boost::shared_ptr<LivingEntity>& entity);
#endif

private:
    PigSaddleLayer* mSaddleLayer;
#ifdef MINECRAFT_PC
    Model* mFlatFaceModel;
#endif
};

#endif
