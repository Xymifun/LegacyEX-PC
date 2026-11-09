#ifndef GAME_CLIENT_RENDER_ARMORSTANDRENDERER_HPP
#define GAME_CLIENT_RENDER_ARMORSTANDRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/ArmorStandModel.hpp"
#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/world/generation/HumanoidArmorLayer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class ResourceLocation;

class ArmorStandRenderer : public LivingEntityRenderer
{
public:
    ArmorStandRenderer(EntityRenderDispatcher* dispatcher);
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual void setupRotations(boost::shared_ptr<LivingEntity>& entity, float ageInTicks, float rotationYaw,
                                float partialTicks);
    virtual bool shouldShowName(boost::shared_ptr<Entity>& entity);
    virtual Model* createModel(const ModelDefinition& definition);

    class ArmorStandArmorLayer : public HumanoidArmorLayer
    {
    public:
        ArmorStandArmorLayer(LivingEntityRenderer* renderer);
        virtual ~ArmorStandArmorLayer();
        virtual void createArmorModels();
    };
};

#endif
