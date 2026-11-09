#ifndef GAME_CLIENT_RENDER_FISHRENDERER_HPP
#define GAME_CLIENT_RENDER_FISHRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class FishRenderer : public MobRenderer
{
public:
    enum FishType
    {
        FISH_TYPE_COD = 0,
        FISH_TYPE_PUFFERFISH = 1,
        FISH_TYPE_SALMON = 2,
        FISH_TYPE_TROPICAL_FISH = 3
    };

    FishRenderer(EntityRenderDispatcher* dispatcher, FishType fishType, float shadowRadius);
    virtual ~FishRenderer() {}

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
    virtual Model* getModel(boost::shared_ptr<LivingEntity>& entity);

protected:
    FishType mFishType;
};

#endif
