#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <cmath>

#include "game/client/render/Model.hpp"
#include "game/client/render/PlayerModelPart.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/RenderLayer.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Options.hpp"
#include "game/core/Team.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/generation/CustomHeadLayer.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "vendor/_SkinAdjustments.hpp"
#include "game/core/PIXNamedEvents.hpp"

double lerp(double from, double to, double amount)
{
    return (to - from) * amount + from;
}

float hermiteBlend(float amount)
{
    return 3.0F * amount * amount - (amount + amount) * amount * amount;
}

LivingEntityRenderer::LivingEntityRenderer(EntityRenderDispatcher* dispatcher, Model* model, float shadowRadius)
    : EntityRenderer(dispatcher),
      mLayers()
{
    mModel = model;
    mShadowRadius = shadowRadius;
    mItemFrame = false;
}

bool LivingEntityRenderer::addLayer(RenderLayer* layer)
{
    mLayers.push_back(layer);
    return true;
}

bool LivingEntityRenderer::removeLayer(RenderLayer* layer)
{
    std::vector<RenderLayer*>::iterator it = std::find(mLayers.begin(), mLayers.end(), layer);
    if (it != mLayers.end())
    {
        mLayers.erase(it);
        return true;
    }
    return false;
}

Model* LivingEntityRenderer::getModel()
{
    return mModel;
}

Model* LivingEntityRenderer::getModel(boost::shared_ptr<LivingEntity>&)
{
    return mModel;
}

float LivingEntityRenderer::rotlerp(float from, float to, float delta)
{
    float difference = to - from;
    while (difference < -180.0F)
        difference += 360.0F;
    while (difference >= 180.0F)
        difference -= 360.0F;
    return from + delta * difference;
}

void LivingEntityRenderer::translateWeaponItem()
{
    return;
}

float LivingEntityRenderer::setupScale(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    GlStateManager::enableRescaleNormal();
    GlStateManager::scalef(-1.0F, -1.0F, 1.0F);
    scale(entity, partialTicks);
    const float scaleFactor = 0.0625F;
    GlStateManager::translatef(0.0F, -1.501F, 0.0F);
    return scaleFactor;
}

bool LivingEntityRenderer::setupSolidState(boost::shared_ptr<LivingEntity>&)
{
    GlStateManager::disableLighting();
    GlStateManager::activeTexture(1);
    GlStateManager::disableTexture();
    GlStateManager::activeTexture(0);
    return true;
}

bool LivingEntityRenderer::setupOverlayColor(boost::shared_ptr<LivingEntity>& entity, float partialTicks,
                                             bool combineTextures)
{
    float brightness = entity->getBrightness();
    int overlayColor = getOverlayColor(entity, brightness, partialTicks);
    int alpha = overlayColor >> 24 & 0xff;
    bool hasOverlay = alpha > 0;
    bool hurt = entity->mHurtTime > 0 || entity->mDeathTime > 0;

    bool fireTransition = false;
    float fireRed = 0.0F;
    float fireGreen = 0.0F;
    float fireBlue = 0.0F;
    float fireAlpha = 0.0F;
    if (entity->getFireTransitionStartTick() >= 0)
    {
        int elapsed = entity->mTickCount - entity->getFireTransitionStartTick();
        float progress = 0.0F;
        if (entity->isOnFire())
        {
            progress = Mth::clamp((float)elapsed / 5.0F, 0.0F, 1.0F);
        }
        else if (elapsed < 5)
        {
            progress = 1.0F - Mth::clamp((float)elapsed / 5.0F, 0.0F, 1.0F);
        }

        if (progress > 0.0F)
        {
            float phase = ((float)elapsed / 20.0F + (float)elapsed / 20.0F) * 3.1415927F;
            float pulse = Mth::sin((phase + phase) * 0.5F);
            fireRed = 0.8F;
            fireGreen = (float)lerp(0.3F, 0.15F, pulse * pulse);
            fireBlue = 0.0F;
            fireAlpha = hermiteBlend(progress) * 0.7F;
            fireTransition = true;
        }
    }

    if (hasOverlay || hurt || fireTransition)
    {
        if (hurt && !combineTextures)
            return false;

        if (fireTransition)
        {
            gRenderer->StateSetLerpColour(fireRed, fireGreen, fireBlue, fireAlpha);
        }
        else if (hurt)
        {
            gRenderer->StateSetLerpColour(1.0F, 0.0F, 0.0F, 0.3F);
        }
        else
        {
            gRenderer->StateSetLerpColour((float)(overlayColor >> 16 & 0xff) / 255.0F,
                                          (float)(overlayColor >> 8 & 0xff) / 255.0F,
                                          (float)(overlayColor & 0xff) / 255.0F, (float)alpha / 255.0F);
        }
        return true;
    }
    return false;
}

bool LivingEntityRenderer::setupOverlayColor(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    return setupOverlayColor(entity, partialTicks, true);
}

void LivingEntityRenderer::teardownOverlayColor()
{
    gRenderer->StateSetLerpColour(0.0F, 0.0F, 0.0F, 0.0F);
}

void LivingEntityRenderer::tearDownSolidState()
{
    GlStateManager::enableLighting();
    GlStateManager::activeTexture(1);
    GlStateManager::enableTexture();
    GlStateManager::activeTexture(0);
}

void LivingEntityRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                                  float partialTick)
{
    bool isPlayer = eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType());
    if (isPlayer)
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
        if (player->isSpectator() && player->IsSpectatingOtherPlayer())
            return;
    }

    entity->GetType();
    PIXBeginNamedEvent(16711935.0F, "LivingEntityRenderer %d");
    boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(entity);

    GlStateManager::pushMatrix();
    GlStateManager::disableCull();

    Model* model = getModel(living);
    model->mAttackTime = getAttackAnim(living, partialTick);
    model->mRiding = living->isPassenger() && living->getVehicle()->GetType() != EntityType::DOLPHIN;
    model->mYoung = living->isBaby();
    model->mRidingDolphin = living->isPassenger()
                            && eTYPE_DERIVED_FROM(EntityType::DOLPHIN, living->getVehicle()->GetType());

    float bodyRot = rotlerp(living->mYBodyRotO, living->mYBodyRot, partialTick);
    float headRot = rotlerp(living->mYHeadRotO, living->mYHeadRot, partialTick);
    float netHeadYaw = headRot - bodyRot;

    if (living->isPassenger() && eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, living->getVehicle()->GetType()))
    {
        boost::shared_ptr<LivingEntity> vehicle = boost::static_pointer_cast<LivingEntity>(living->getVehicle());
        bodyRot = rotlerp(vehicle->mYBodyRotO, vehicle->mYBodyRot, partialTick);
        float headDiff = Mth::wrapDegrees(headRot - bodyRot);
        if (headDiff < -85.0F)
            headDiff = -85.0F;
        if (headDiff >= 85.0F)
            headDiff = 85.0F;
        bodyRot = headRot - headDiff;
        if (headDiff * headDiff > 2500.0F)
            bodyRot += headDiff * 0.2F;
        netHeadYaw = headRot - bodyRot;
    }

    float headPitch = living->mXRotO + (living->mXRot - living->mXRotO) * partialTick;
    setupPosition(living, x, y, z);
    float bob = getBob(living, partialTick);
    setupRotations(living, bob, bodyRot, partialTick);
    float scale = setupScale(living, partialTick);

    float limbSwingAmount = 0.0F;
    float limbSwing = 0.0F;
    if (!living->isPassenger())
    {
        limbSwingAmount = living->mAnimationSpeedOld
                          + (living->mAnimationSpeed - living->mAnimationSpeedOld) * partialTick;
        limbSwing = living->mAnimationPosition - living->mAnimationSpeed * (1.0F - partialTick);
        if (living->isBaby())
            limbSwing *= 3.0F;
        if (limbSwingAmount > 1.0F)
            limbSwingAmount = 1.0F;
    }

    GlStateManager::enableAlphaTest();
    PIXBeginNamedEvent(0.0F, "prepareMobModel");
    model->prepareMobModel(living, limbSwing, limbSwingAmount, partialTick);
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0F, "setupAnim");
    model->setupAnim(limbSwing, limbSwingAmount, bob, netHeadYaw, headPitch, scale, living, _SkinAdjustments());
    PIXEndNamedEvent();

    if (mSolidRender)
    {
        bool solidState = setupSolidState(living);
        PIXBeginNamedEvent(0.0F, "solidRender");
        GlStateManager::enableColorMaterial();
        GlStateManager::setupSolidRenderingTextureCombine(getTeamColor(entity));
        if (!mItemFrame)
        {
            bool overlay = setupOverlayColor(living, partialTick);
            renderModel(living, limbSwing, limbSwingAmount, bob, netHeadYaw, headPitch, scale);
            if (overlay)
                teardownOverlayColor();
        }

        bool showLayers = true;
        const bool isPlayer = eTYPE_DERIVED_FROM(EntityType::PLAYER, living->GetType());
        if (isPlayer && boost::static_pointer_cast<Player>(living)->isSpectator()
            && !boost::static_pointer_cast<Player>(living)->SpectateAsFloatingHead())
        {
            showLayers = false;
        }
        if (showLayers)
            renderLayers(living, limbSwing, limbSwingAmount, partialTick, bob, netHeadYaw, headPitch, scale, true);

        GlStateManager::tearDownSolidRenderingTextureCombine();
        GlStateManager::disableColorMaterial();
        PIXEndNamedEvent();
        if (solidState)
            tearDownSolidState();
    }
    else
    {
        bool overlay = setupOverlayColor(living, partialTick);
        renderModel(living, limbSwing, limbSwingAmount, bob, netHeadYaw, headPitch, scale);
        if (overlay)
            teardownOverlayColor();
        GlStateManager::depthMask(true);

        PIXBeginNamedEvent(0.0F, "renderLayers");
        bool showLayers = true;
        const bool isPlayer = eTYPE_DERIVED_FROM(EntityType::PLAYER, living->GetType());
        if (isPlayer && boost::static_pointer_cast<Player>(living)->isSpectator()
            && !boost::static_pointer_cast<Player>(living)->SpectateAsFloatingHead())
        {
            showLayers = false;
        }
        if (showLayers)
            renderLayers(living, limbSwing, limbSwingAmount, partialTick, bob, netHeadYaw, headPitch, scale, true);
        PIXEndNamedEvent();
    }

    GlStateManager::disableRescaleNormal();
    GlStateManager::activeTexture(1);
    GlStateManager::enableTexture();
    GlStateManager::activeTexture(0);
    GlStateManager::enableCull();
    GlStateManager::popMatrix();
    PIXBeginNamedEvent(0.0F, "EntityRender");
    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
    PIXEndNamedEvent();
    model->clearAllOffsets();
    PIXEndNamedEvent();
}

void LivingEntityRenderer::setupForTexturePack(TexturePack* pack)
{
    EntityRenderer::setupForTexturePack(pack);
    std::vector<RenderLayer*>::iterator it = mLayers.begin();
    while (it != mLayers.end())
    {
        (*it)->setupForTexturePack(pack);
        ++it;
    }
}

void LivingEntityRenderer::renderModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                       float ageInTicks, float netHeadYaw, float headPitch, float scale)
{
    bool visible = isVisible(entity);
    bool solid = !visible && !entity->isInvisibleTo(Minecraft::GetInstance()->mPlayer);

    if (visible || solid)
    {
        boost::shared_ptr<Entity> baseEntity = entity;
        if (!bindTexture(baseEntity))
            return;

        if (solid)
            GlStateManager::setProfile();

        Model* model = getModel(entity);
        model->render(baseEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, true);

        if (solid)
            GlStateManager::unsetProfile();
    }
}

bool LivingEntityRenderer::isVisible(boost::shared_ptr<LivingEntity>& entity)
{
    return !entity->isInvisible() || mSolidRender;
}

void LivingEntityRenderer::setupPosition(boost::shared_ptr<LivingEntity>&, double x, double y, double z)
{
    GlStateManager::translatef((float)x, (float)y, (float)z);
}

void LivingEntityRenderer::setupRotations(boost::shared_ptr<LivingEntity>& entity, float ageInTicks, float rotationYaw,
                                          float partialTicks)
{
    GlStateManager::rotatef(180.0F - rotationYaw, 0.0F, 1.0F, 0.0F);

    if (entity->mDeathTime > 0)
    {
        float fall = std::sqrt(((float)entity->mDeathTime + partialTicks - 1.0F) / 20.0F * 1.6F);
        if (fall > 1.0F)
            fall = 1.0F;
        GlStateManager::rotatef(fall * getFlipDegrees(entity), 0.0F, 0.0F, 1.0F);
    }
    else
    {
        std::wstring name = entity->getName();
        if (name == L"Dinnerbone" || name == L"Grumm")
        {
            bool flip = !eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType())
                        || !boost::static_pointer_cast<Player>(entity)->isModelPartShown(PlayerModelPart::CAPE);
            if (flip)

            {
                GlStateManager::translatef(0.0F, entity->mBbHeight + 0.1F, 0.0F);
                GlStateManager::rotatef(180.0F, 0.0F, 0.0F, 1.0F);
            }
        }
        else if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
        {
            unsigned int animFlags = boost::static_pointer_cast<Player>(entity)->getSkinAdjustments().mAnimFlags;
            if ((animFlags & 0x80000000) != 0)

            {
                GlStateManager::translatef(0.0F, entity->mBbHeight + 0.1F, 0.0F);
                GlStateManager::rotatef(180.0F, 0.0F, 0.0F, 1.0F);
            }
        }
    }

    if (entity->getSwimAmount(partialTicks) > 0.0F)
    {
        float swimAmount = entity->getSwimAmount(partialTicks);
        if (swimAmount > 0.0F)
        {
            float xRot = entity->mXRotO + (entity->mXRot - entity->mXRotO) * partialTicks;
            GlStateManager::rotatef(entity->getSwimAmount(partialTicks) * (-90.0F - xRot), 1.0F, 0.0F, 0.0F);
            GlStateManager::translatef(0.0F, -swimAmount, 0.5F * swimAmount);
        }
    }
}

float LivingEntityRenderer::getAttackAnim(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    return entity->getAttackAnim(partialTicks);
}

float LivingEntityRenderer::getBob(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    return (float)entity->mTickCount + partialTicks;
}

void LivingEntityRenderer::renderLayers(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                        float partialTick, float ageInTicks, float netHeadYaw, float headPitch,
                                        float scale, bool overlay)
{
    for (std::vector<RenderLayer*>::iterator it = mLayers.begin(); it != mLayers.end(); ++it)
    {
        RenderLayer* layer = *it;
        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType())
            && boost::static_pointer_cast<Player>(entity)->isSpectator()
            && boost::static_pointer_cast<Player>(entity)->SpectateAsFloatingHead()
            && (layer == NULL || dynamic_cast<CustomHeadLayer*>(layer) == NULL))
        {
            continue;
        }

        bool damageOverlay = setupOverlayColor(entity, partialTick, layer->colorsOnDamage());
        layer->render(entity, limbSwing, limbSwingAmount, partialTick, ageInTicks, netHeadYaw, headPitch, scale,
                      overlay);
        if (damageOverlay)
            teardownOverlayColor();
    }
}

float LivingEntityRenderer::getFlipDegrees(boost::shared_ptr<LivingEntity>&)
{
    float flipDegrees = 90.0F;
    return flipDegrees;
}

int LivingEntityRenderer::getOverlayColor(boost::shared_ptr<LivingEntity>&, float, float)
{
    return 0;
}

void LivingEntityRenderer::scale(boost::shared_ptr<LivingEntity>&, float)
{
    return;
}

void LivingEntityRenderer::renderName(boost::shared_ptr<Entity>& entity, double x, double y, double z)
{
    if (shouldShowName(entity) || Minecraft::renderDebug())
    {
        double distanceSqr = entity->distanceToSqr(Minecraft::GetInstance()->mPlayer->GetCamera());

        float maxDistance = entity->isSneaking() ? 32.0F : 64.0F;
        if (distanceSqr < maxDistance * maxDistance)
        {
            std::wstring name = entity->getDisplayName();
            if (!name.empty())
            {
                if (GameSettings::GetGameSettings(eGameSetting_DisplayHUD) == 0
                    || Minecraft::GetInstance()->mOptions->mHideGui)
                    return;

                GlStateManager::alphaFunc(4, 0.01f);
                renderNameTags(entity, x, y, z, name, distanceSqr, 0xFF000000);
            }
        }
    }
}

bool LivingEntityRenderer::shouldShowName(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(entity);
    boost::shared_ptr<Player> localPlayer = Minecraft::GetInstance()->mPlayer;
    bool visibleToPlayer = !living->isInvisibleTo(localPlayer);

    if (living->isInvisibleTo(localPlayer) || living.get() == mDispatcher->mCamera.get() || living->isVehicle())
        return false;

    const bool isPlayer = eTYPE_DERIVED_FROM(EntityType::PLAYER, living->GetType());
    if (!isPlayer || living.get() == localPlayer.get())
    {
        bool renderNames = Minecraft::renderNames();
        bool hasCustomName = living->hasCustomName();
        return renderNames && hasCustomName;
    }

    bool showName;
    if (!Minecraft::InMiniGame(eMiniGameId_Tumble, true))
    {
        showName = living.get() == mDispatcher->mPlayer.get();
    }
    else
    {
        showName = living->distanceTo(localPlayer) < 3.0F && !localPlayer->isSpectator();
    }

    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(living);
    if (player->isSpectator())
    {
        if (player->SpectateAsInvisible())
            return showName;
    }
    if (player->isSpectator())
    {
        if (player->SpectateAsFloatingHead())
            return false;
    }

    if (!Minecraft::InMiniGame(eMiniGameId_Lobby, true))
    {
        Team* entityTeam = living->getTeam();
        Team* playerTeam = localPlayer->getTeam();
        if (entityTeam != NULL)
        {
            switch (entityTeam->getNameTagVisibility())
            {
            case Team::ALWAYS:
                return visibleToPlayer;
            case Team::NEVER:
                return showName;
            case Team::HIDE_FOR_OTHER_TEAMS:
                if (!showName)
                {
                    if (!localPlayer->isSpectator())
                    {
                        bool result = visibleToPlayer;
                        if (playerTeam != NULL)
                            result = entityTeam->isAlliedTo(playerTeam)
                                     && (entityTeam->canSeeFriendlyInvisibles() || visibleToPlayer);
                        return result;
                    }
                }
                return true;
            case Team::HIDE_FOR_OWN_TEAM:
                if (!showName)
                {
                    if (!localPlayer->isSpectator())
                    {
                        bool result = visibleToPlayer;
                        if (playerTeam != NULL)
                            result = !entityTeam->isAlliedTo(playerTeam) && visibleToPlayer;
                        return result;
                    }
                }
                return true;
            }
        }
    }

    if (localPlayer->isSpectator())
        return true;

    if (!Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        return Minecraft::renderNames() && gApp.GetGameHostOption(eGameHostOption_Gamertags) != 0;
    }

    return Minecraft::GetInstance()->GetMiniGame()->GetShowNametags() || showName;
}
