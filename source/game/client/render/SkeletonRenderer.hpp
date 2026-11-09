#ifndef GAME_CLIENT_RENDER_SKELETONRENDERER_HPP
#define GAME_CLIENT_RENDER_SKELETONRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/HumanoidMobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class Model;
class ModelDefinition;
class ResourceLocation;
class SkeletonModel;

class SkeletonRenderer : public HumanoidMobRenderer
{
public:
    SkeletonRenderer(EntityRenderDispatcher* dispatcher);
    SkeletonRenderer(EntityRenderDispatcher* dispatcher, SkeletonModel* model);
    virtual ~SkeletonRenderer() {}

    virtual void translateWeaponItem();
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
