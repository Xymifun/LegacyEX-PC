#ifndef GAME_CLIENT_RENDER_HORSERENDERER_HPP
#define GAME_CLIENT_RENDER_HORSERENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/AbstractHorseRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class HorseRenderer : public AbstractHorseRenderer
{
public:
    HorseRenderer(EntityRenderDispatcher* dispatcher);

    virtual void bindTexture(const ResourceLocation* location);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);
    virtual void renderModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                             float ageInTicks, float netHeadYaw, float headPitch, float scale);
};

#endif
