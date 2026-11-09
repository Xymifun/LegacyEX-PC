#ifndef GAME_CLIENT_RENDER_GIANTMOBRENDERER_HPP
#define GAME_CLIENT_RENDER_GIANTMOBRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class GiantMobRenderer : public MobRenderer
{
public:
    GiantMobRenderer(EntityRenderDispatcher* dispatcher, float shadowRadius);

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
    virtual void translateWeaponItem();
    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks);

protected:
    float mScale;
};

#endif
