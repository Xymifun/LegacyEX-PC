#include "game/world/generation/CustomHeadLayer.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/ItemInHandRenderer.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/SkullBlockRenderer.hpp"
#include "game/core/Direction.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemTransforms.hpp"
#include "game/item/Items.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/world/generation/Player.hpp"
#include "vendor/_SkinAdjustments.hpp"

CustomHeadLayer::CustomHeadLayer(ModelPart* head) : mHead(head)
{
}

CustomHeadLayer::~CustomHeadLayer()
{
}

void CustomHeadLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                             float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                             bool bUseDisplayList)
{
    not_null_ptr<ItemInstance> headItem = entity->getItemSlot(EquipmentSlot::HEAD);

    if (headItem->isEmpty())
    {
        return;
    }

    bool bSpectating = eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType())
                       && boost::static_pointer_cast<Player>(entity)->isSpectator();

    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
    {
        if (bSpectating && boost::static_pointer_cast<Player>(entity)->SpectateAsSkull())
        {
        }
        else
        {
            boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
            if ((player->getSkinAdjustments().mAnimFlags & 0x100) != 0)
            {
                return;
            }
        }
    }

    Item* item = headItem->getItem();
    Minecraft* minecraft = Minecraft::GetInstance();

    GlStateManager::pushMatrix();

    if (bSpectating && boost::static_pointer_cast<Player>(entity)->SpectateAsFloatingHead())
    {
        GlStateManager::translatef(0.0f, 1.5f, 0.0f);
    }

    bool bFallFlying = entity->getFallFlyingTicks() > 4;

    if (entity->isSneaking() && !bFallFlying)
    {
        GlStateManager::translatef(0.0f, 0.2f, 0.0f);
    }

    bool bVillagerHead = eTYPE_DERIVED_FROM(EntityType::VILLAGER, entity->GetType())
                         || eTYPE_DERIVED_FROM(EntityType::ZOMBIE_VILLAGER, entity->GetType());

    if (entity->isBaby() && !eTYPE_DERIVED_FROM(EntityType::VILLAGER, entity->GetType()))
    {
        GlStateManager::translatef(0.0f, 0.5f * scale, 0.0f);
        GlStateManager::scalef(0.7f, 0.7f, 0.7f);
        GlStateManager::translatef(0.0f, 16.0f * scale, 0.0f);
    }

    mHead->translateTo(0.0625f);

    if (!Minecraft::InMiniGame(eMiniGameId_Glide, true))
    {
        GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    }

    if (item == Items::skull)
    {
        float headScale = 1.0f;

        if (!eTYPE_DERIVED_FROM(EntityType::ARMOR_STAND, entity->GetType()))
        {
            headScale = 1.1875f;
        }

        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
        {
            headScale = 1.25f;
        }

        GlStateManager::scalef(headScale, -headScale, -headScale);

        if (bVillagerHead)
        {
            GlStateManager::translatef(0.0f, 0.0625f, 0.0f);
        }

        if (!eTYPE_DERIVED_FROM(EntityType::ARMOR_STAND, entity->GetType()))
        {
            GlStateManager::translatef(0.0f, (-0.5f - entity->getSkinAdjustments().mHeadY) * 0.0625f, 0.0f);

            SkullBlockRenderer::instance->renderSkull(-0.5f, 0.0f, -0.5f, Direction::UP, netHeadYaw + 180.0f, headPitch,
                                                      headItem->getAuxValue(), -1, limbSwing, true);

            GlStateManager::popMatrix();
        }
        else
        {
            SkullBlockRenderer::instance->renderSkull(-0.5f, 0.0f, -0.5f, Direction::UP, 180.0f, headPitch,
                                                      headItem->getAuxValue(), -1, limbSwing, true);

            GlStateManager::popMatrix();
        }
    }
    else
    {
        if (dynamic_cast<ArmorItem*>(item) == NULL || static_cast<ArmorItem*>(item)->getSlot() != EquipmentSlot::HEAD)
        {
            GlStateManager::translatef(0.0f, -0.25f, 0.0f);
            GlStateManager::rotatef(180.0f, 0.0f, 1.0f, 0.0f);
            GlStateManager::scalef(0.625f, -0.625f, -0.625f);

            if (bVillagerHead)
            {
                GlStateManager::translatef(0.0f, 0.1875f, 0.0f);
            }

            minecraft->getItemInHandRenderer()->renderItem(entity, headItem, ItemTransforms::HEAD, false, 0,
                                                           !Minecraft::InMiniGame(eMiniGameId_Glide, true));
        }

        GlStateManager::popMatrix();
    }
}

bool CustomHeadLayer::colorsOnDamage()
{
    return true;
}
