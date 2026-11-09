#ifndef GAME_CLIENT_RENDER_WITHERSKELETONRENDERER_HPP
#define GAME_CLIENT_RENDER_WITHERSKELETONRENDERER_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/SkeletonRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class WitherSkeletonRenderer : public SkeletonRenderer
{
public:
    WitherSkeletonRenderer(EntityRenderDispatcher* dispatcher);

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual std::wstring getModelName();
    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks);
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
