#ifndef GAME_CLIENT_RENDER_EVOCATIONILLAGERRENDERER_HPP
#define GAME_CLIENT_RENDER_EVOCATIONILLAGERRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"
#include "game/world/generation/ItemInHandLayer.hpp"

class Entity;
class EntityRenderDispatcher;
class HumanoidArm;
class Model;
class ModelDefinition;
class ResourceLocation;
class LivingEntity;

class EvocationIllagerRenderer : public MobRenderer
{
public:
    class EvocationIllagerRenderer_ItemInHandLayer : public ItemInHandLayer
    {
    public:
        EvocationIllagerRenderer_ItemInHandLayer(EvocationIllagerRenderer* renderer);
        virtual ~EvocationIllagerRenderer_ItemInHandLayer();

        virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                            float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                            bool bUseDisplayList);
        virtual void translateToHand(const HumanoidArm* arm);
    };

    EvocationIllagerRenderer(EntityRenderDispatcher* dispatcher);

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks);
    virtual std::wstring getModelName();
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
