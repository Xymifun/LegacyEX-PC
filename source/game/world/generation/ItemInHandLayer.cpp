#include "game/world/generation/ItemInHandLayer.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/ItemInHandRenderer.hpp"
#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/client/render/HumanoidModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"

ItemInHandLayer::ItemInHandLayer(LivingEntityRenderer* renderer) : mLivingEntityRenderer(renderer)
{
}

void ItemInHandLayer::render(boost::shared_ptr<LivingEntity>& entity, float, float, float, float, float, float, float,
                             bool)
{
    const bool rightMain = entity->getMainArm() == HumanoidArm::RIGHT;
    not_null_ptr<ItemInstance> leftHandItem(rightMain ? entity->getOffhandItem() : entity->getMainHandItem());
    not_null_ptr<ItemInstance> rightHandItem(rightMain ? entity->getMainHandItem() : entity->getOffhandItem());

    if (leftHandItem->isEmpty() && rightHandItem->isEmpty())
        return;

    GlStateManager::pushMatrix();

    if (mLivingEntityRenderer->getModel()->mYoung)
    {
        GlStateManager::translatef(0.0f, 0.75f, 0.0f);
        GlStateManager::scalef(0.5f, 0.5f, 0.5f);
    }

    renderArmWithItem(entity, rightHandItem, ItemTransforms::THIRD_PERSON_RIGHT_HAND, HumanoidArm::RIGHT);
    renderArmWithItem(entity, leftHandItem, ItemTransforms::THIRD_PERSON_LEFT_HAND, HumanoidArm::LEFT);

    GlStateManager::popMatrix();
}

void ItemInHandLayer::renderArmWithItem(boost::shared_ptr<LivingEntity> entity, not_null_ptr<ItemInstance> item,
                                        ItemTransforms::TransformType transformType, const HumanoidArm* arm)
{
    if (item->isEmpty())
        return;

    GlStateManager::pushMatrix();
    translateToHand(arm);

    bool fallFlying = false;
    if (entity != NULL)
    {
        fallFlying = eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType())
                     && boost::static_pointer_cast<LivingEntity>(entity)->getFallFlyingTicks() > 4;
    }
    if (entity->isSneaking() && !fallFlying)
    {
        GlStateManager::translatef(0.0f, 0.175f, 0.0f);
        GlStateManager::rotatef(6.0f, 1.0f, 0.0f, 0.0f);
    }

    GlStateManager::rotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(180.0f, 0.0f, 1.0f, 0.0f);
    bool leftHand = arm == HumanoidArm::LEFT;
    GlStateManager::translatef((leftHand ? -1.0f : 1.0f) * 0.0625f, 0.125f, -0.625f);

    Minecraft::GetInstance()->getItemInHandRenderer()->renderItem(entity, item, transformType, leftHand, 0, true);
    GlStateManager::popMatrix();
}

void ItemInHandLayer::translateToHand(const HumanoidArm* arm)
{
    static_cast<HumanoidModel*>(mLivingEntityRenderer->getModel())->translateToHandItem(0.0625f, arm);
}

bool ItemInHandLayer::colorsOnDamage()
{
    return false;
}
