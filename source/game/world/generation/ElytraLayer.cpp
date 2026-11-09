#include "game/world/generation/ElytraLayer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/PlayerRenderer.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/AbstractArmorLayer.hpp"
#include "game/world/generation/Player.hpp"

static ResourceLocation ELYTRA_LOCATION(TN_ITEM_ELYTRA);

ElytraLayer::ElytraLayer(LivingEntityRenderer* renderer) : mRenderer(renderer)
{
    mElytraModel = new ElytraModel();
}

ElytraLayer::~ElytraLayer()
{
    delete mElytraModel;
}

void ElytraLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                         float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                         bool bUseDisplayList)
{
    boost::shared_ptr<LivingEntity> player = boost::static_pointer_cast<Player>(entity);
    boost::shared_ptr<Entity> renderEntity = player;
    not_null_ptr<ItemInstance> item = player->getItemSlot(EquipmentSlot::CHEST);

    if (item->getItem() != Items::elytra)
    {
        return;
    }

    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(1, 0);
    mRenderer->bindTexture(&ELYTRA_LOCATION);
    GlStateManager::pushMatrix();

    PlayerRenderer* playerRenderer = dynamic_cast<PlayerRenderer*>(mRenderer);
    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, player->GetType()) && playerRenderer->mSpecialRenderActive)
    {
        GlStateManager::enableBlend();
        GlStateManager::blendFunc(4, 5);
        GlStateManager::color4f(1.0f, 1.0f, 1.0f, playerRenderer->mSpecialRenderAlpha);
    }

    GlStateManager::translatef(0.0f, 0.0f, 0.125f);

    mElytraModel->setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, player,
                            _SkinAdjustments());
    mElytraModel->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale,
                         bUseDisplayList);

    if (item->isEnchanted())
    {
        AbstractArmorLayer::renderFoil(mRenderer, entity, mElytraModel, limbSwing, limbSwingAmount, partialTicks,
                                       ageInTicks, netHeadYaw, headPitch, scale, bUseDisplayList);
    }

    GlStateManager::disableBlend();
    GlStateManager::popMatrix();
}

bool ElytraLayer::colorsOnDamage()
{
    return false;
}

ElytraModel* ElytraLayer::getElytraModel()
{
    return mElytraModel;
}
