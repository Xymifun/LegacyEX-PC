#include "game/world/generation/DolphinCarryingItemLayer.hpp"

#include <cmath>

#include "game/client/render/ItemRenderer.hpp"
#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/block/Block.hpp"

DolphinCarryingItemLayer::DolphinCarryingItemLayer(LivingEntityRenderer* renderer)
    : mLivingEntityRenderer(renderer),
      mItemRenderer(Minecraft::GetInstance()->getItemRenderer())
{
}

DolphinCarryingItemLayer::~DolphinCarryingItemLayer()
{
}

void DolphinCarryingItemLayer::renderItemOnNose(const boost::shared_ptr<LivingEntity>& entity,
                                                const not_null_ptr<ItemInstance>& item)
{
    if (item->isEmpty())
        return;
    if (item->isEmpty())
        return;

    Block* block = Block::byItem(item->getItem());
    GlStateManager::pushMatrix();

    bool depthMask = mItemRenderer->isGui3d(item) && block->getRenderLayer() == 1;
    if (depthMask)
        GlStateManager::depthMask(false);

    float rotation = fabsf(entity->mXRot) / 60.0f;
    if (entity->mXRot < 0.0f)
        GlStateManager::translatef(0.0f, 1.0f - rotation * 0.5f, -1.0f + rotation * 0.5f);
    else
        GlStateManager::translatef(0.0f, 1.0f + rotation * 0.8f, -1.0f + rotation * 0.2f);

    mItemRenderer->renderStatic(item, entity, ItemTransforms::GROUND, 0.0f, false, NULL);

    if (depthMask)
        GlStateManager::depthMask(true);
    GlStateManager::popMatrix();
}

void DolphinCarryingItemLayer::render(boost::shared_ptr<LivingEntity>& entity, float, float, float, float, float, float,
                                      float, bool)
{
    const bool rightHanded = entity->getMainArm() == HumanoidArm::RIGHT;
    not_null_ptr<ItemInstance> leftHandItem = rightHanded ? entity->getOffhandItem() : entity->getMainHandItem();
    not_null_ptr<ItemInstance> rightHandItem = rightHanded ? entity->getMainHandItem() : entity->getOffhandItem();

    if (leftHandItem->isEmpty() && rightHandItem->isEmpty())
        return;

    renderItemOnNose(entity, rightHandItem);
}

bool DolphinCarryingItemLayer::colorsOnDamage()
{
    return false;
}
