#ifndef GAME_WORLD_GENERATION_ITEMINHANDLAYER_HPP
#define GAME_WORLD_GENERATION_ITEMINHANDLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"
#include "game/item/ItemTransforms.hpp"
#include "game/item/not_null_ptr.hpp"

class HumanoidArm;
class ItemInstance;
class LivingEntity;
class LivingEntityRenderer;

class ItemInHandLayer : public RenderLayer
{
public:
    ItemInHandLayer(LivingEntityRenderer* renderer);
    virtual ~ItemInHandLayer() {}

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();
    virtual void translateToHand(const HumanoidArm* arm);

    void renderArmWithItem(boost::shared_ptr<LivingEntity> entity, not_null_ptr<ItemInstance> item,
                           ItemTransforms::TransformType transformType, const HumanoidArm* arm);

protected:
    LivingEntityRenderer* mLivingEntityRenderer;
};

#endif
