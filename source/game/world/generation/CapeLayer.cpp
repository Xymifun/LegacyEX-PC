#include "game/world/generation/CapeLayer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/PlayerModel.hpp"
#include "game/client/render/PlayerModelPart.hpp"
#include "game/client/render/PlayerRenderer.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/generation/Player.hpp"

CapeLayer::CapeLayer(PlayerRenderer* renderer) : mRenderer(renderer)
{
}

CapeLayer::~CapeLayer()
{
}

void CapeLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                       float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                       bool bUseDisplayList)
{
    boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(entity);

    if (player->isInvisible() || !player->isModelPartShown(PlayerModelPart::CAPE) || player->mCapeLocation.empty())
    {
        return;
    }

    const boost::shared_ptr<ItemInstance>& chestItem = player->getItemSlot(EquipmentSlot::CHEST);

    if (chestItem->getItem() == Items::elytra)
    {
        return;
    }

    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);

    if (!mRenderer->bindTexture(player->mCapeLocation, L""))
    {
        return;
    }

    GlStateManager::pushMatrix();
    GlStateManager::translatef(0.0f, 0.0f, 0.125f);

    double capeX = (player->mPrevChasingPosX + (player->mChasingPosX - player->mPrevChasingPosX) * partialTicks)
                   - (player->mXo + (player->mX - player->mXo) * partialTicks);
    double capeY = (player->mPrevChasingPosY + (player->mChasingPosY - player->mPrevChasingPosY) * partialTicks)
                   - (player->mYo + (player->mY - player->mYo) * partialTicks);
    double capeZ = (player->mPrevChasingPosZ + (player->mChasingPosZ - player->mPrevChasingPosZ) * partialTicks)
                   - (player->mZo + (player->mZ - player->mZo) * partialTicks);
    float bodyRot = player->mYBodyRotO + (player->mYBodyRot - player->mYBodyRotO) * partialTicks;
    double sinBody = Mth::sin(bodyRot * Mth::DEG_TO_RAD);
    double cosBody = -Mth::cos(bodyRot * Mth::DEG_TO_RAD);
    float vertical = Mth::clamp((float)capeY * 10.0f, -6.0f, 32.0f);
    float horizontal = (float)(capeX * cosBody - capeZ * sinBody) * 100.0f;
    float forward = (float)(capeX * sinBody + capeZ * cosBody) * 100.0f;

    if (forward < 0.0f)
    {
        forward = 0.0f;
    }

    float cameraYaw = player->mPrevCameraYaw + (player->mCameraYaw - player->mPrevCameraYaw) * partialTicks;
    float walkDist = player->mWalkDistO + (player->mWalkDist - player->mWalkDistO) * partialTicks;
    vertical += Mth::sin(walkDist * 6.0f) * 32.0f * cameraYaw;

    if (player->isSneaking())
    {
        vertical += 25.0f;
    }

    float capeRotation = forward * 0.5f + 6.0f + vertical;

    if (capeRotation > 64.0f)
    {
        capeRotation = 64.0f;
    }

    GlStateManager::rotatef(capeRotation, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(horizontal * 0.5f, 0.0f, 0.0f, 1.0f);
    GlStateManager::rotatef(-(horizontal * 0.5f), 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(180.0f, 0.0f, 1.0f, 0.0f);
    static_cast<PlayerModel*>(mRenderer->getModel())->renderCloak(0.0625f);
    GlStateManager::popMatrix();
}

bool CapeLayer::colorsOnDamage()
{
    return false;
}
