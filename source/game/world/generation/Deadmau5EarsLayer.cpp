#include "game/world/generation/Deadmau5EarsLayer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/GlStateManager.hpp"
#include "game/client/render/PlayerModel.hpp"
#include "game/client/render/PlayerRenderer.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/generation/Player.hpp"

Deadmau5EarsLayer::Deadmau5EarsLayer(PlayerRenderer* renderer) : mRenderer(renderer)
{
}

Deadmau5EarsLayer::~Deadmau5EarsLayer()
{
}

void Deadmau5EarsLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                               float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                               bool bUseDisplayList)
{
    boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(entity);

    if (!gApp.isXuidDeadmau5(player->mXuid) || player->isInvisible())
        return;

    mRenderer->bindTexture(player->mSkinLocation, L"");

    for (int i = 0; i < 2; i++)
    {
        float yaw = player->mYRotO + (player->mYRot - player->mYRotO) * partialTicks
                    - (player->mYBodyRotO + (player->mYBodyRot - player->mYBodyRotO) * partialTicks);
        float xRot = player->mXRotO + (player->mXRot - player->mXRotO) * partialTicks;

        GlStateManager::pushMatrix();
        GlStateManager::rotatef(yaw, 0.0f, 1.0f, 0.0f);
        GlStateManager::rotatef(xRot, 1.0f, 0.0f, 0.0f);
        GlStateManager::translatef(0.375f * (float)(i * 2 - 1), 0.0f, 0.0f);
        GlStateManager::translatef(0.0f, -0.375f, 0.0f);
        GlStateManager::rotatef(-xRot, 1.0f, 0.0f, 0.0f);
        GlStateManager::rotatef(-yaw, 0.0f, 1.0f, 0.0f);
        float earScale = 1.3333334f;
        GlStateManager::scalef(earScale, earScale, earScale);
        static_cast<PlayerModel*>(mRenderer->getModel())->renderEars(0.0625f);
        GlStateManager::popMatrix();
    }
}

bool Deadmau5EarsLayer::colorsOnDamage()
{
    return true;
}
