#include "game/client/render/FishingHookRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/projectile/FishingHook.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/util/Vec3.hpp"

static ResourceLocation FISHING_HOOK_LOCATION(TN_PARTICLES);

FishingHookRenderer::FishingHookRenderer(EntityRenderDispatcher* dispatcher) : EntityRenderer(dispatcher)
{
}

FishingHookRenderer::~FishingHookRenderer()
{
}

void FishingHookRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                                 float partialTick)
{
    boost::shared_ptr<FishingHook> hook = boost::static_pointer_cast<FishingHook>(entity);
    boost::shared_ptr<Player> owner = hook->getOwner();
    if (!owner || mSolidRender)
        return;

    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x, (float)y, (float)z);
    GlStateManager::enableRescaleNormal();
    GlStateManager::scalef(0.5f, 0.5f, 0.5f);
    bindTexture(entity);
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();

    float u0 = 0.0625f;
    float u1 = 0.125f;
    float v0 = 0.125f;
    float v1 = 0.1875f;
    float r = 1.0f;
    float xo = 0.5f;
    float yo = 0.5f;
    GlStateManager::rotatef(180.0f - mDispatcher->mCameraYRot, 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(
        -((hook->getOwner()->GetThirdPersonView(false) == 2 ? -1.0f : 1.0f) * mDispatcher->mCameraXRot), 1.0f, 0.0f,
        0.0f);
    if (mSolidRender)
        GlStateManager::enableColorMaterial();
    builder->begin();
    builder->normal(0.0f, 1.0f, 0.0f);
    builder->vertexUV(0.0f - xo, 0.0f - yo, 0.0f, u0, v1);
    builder->vertexUV(r - xo, 0.0f - yo, 0.0f, u1, v1);
    builder->vertexUV(r - xo, r - yo, 0.0f, u1, v0);
    builder->vertexUV(0.0f - xo, r - yo, 0.0f, u0, v0);
    tesselator->end();
    if (mSolidRender)
        GlStateManager::disableColorMaterial();
    GlStateManager::disableRescaleNormal();
    GlStateManager::popMatrix();

    float attackAnim = owner->getAttackAnim(partialTick);
    float swing = Mth::sin(Mth::sqrt(attackAnim) * 3.1415927f);
    float bodyRot = (owner->mYBodyRotO + (owner->mYBodyRot - owner->mYBodyRotO) * partialTick) * Mth::DEG_TO_RAD;
    double sinRot = Mth::sin(bodyRot);
    double cosRot = Mth::cos(bodyRot);
    double handX;
    double handY;
    double handZ;
    double eyeOffset;
    if (owner->GetThirdPersonView(false) > 0 || owner != Minecraft::GetInstance()->mPlayer)
    {
        handX = owner->mXo + (owner->mX - owner->mXo) * (double)partialTick - cosRot * 0.35 - sinRot * 0.8;
        handY = owner->mYo + (owner->mY - owner->mYo) * (double)partialTick + owner->getEyeHeight() - 0.45;
        handZ = owner->mZo + (owner->mZ - owner->mZo) * (double)partialTick - sinRot * 0.35 + cosRot * 0.8;
        eyeOffset = owner->isSneaking() ? -0.1875 : 0.0;
    }
    else
    {
        float fov = (float)Minecraft::GetInstance()->mGameRenderer->GetFovVal() / 100.0f;
        float sideOffset = -0.37f;
        boost::shared_ptr<LocalPlayer> localPlayer = boost::dynamic_pointer_cast<LocalPlayer>(owner);
        if (localPlayer && (localPlayer->mViewport == 3 || localPlayer->mViewport == 4 || !gRenderer->IsWidescreen()))
            sideOffset = -0.165f;
        Vec3* offset = Vec3::newTemp((double)(sideOffset * fov), -0.045 * (double)fov, 0.4);
        offset = offset->xRot(-(owner->mXRotO + (owner->mXRot - owner->mXRotO) * partialTick) * Mth::DEG_TO_RAD);
        offset = offset->yRot(-(owner->mYRotO + (owner->mYRot - owner->mYRotO) * partialTick) * Mth::DEG_TO_RAD);
        offset = offset->yRot(swing * 0.5f);
        offset = offset->xRot(swing * 0.7f);
        handX = owner->mXo + (owner->mX - owner->mXo) * (double)partialTick + offset->x;
        handY = owner->mYo + (owner->mY - owner->mYo) * (double)partialTick + offset->y;
        handZ = owner->mZo + (owner->mZ - owner->mZo) * (double)partialTick + offset->z;
        eyeOffset = owner->getEyeHeight();
    }

    double hookX = hook->mXo + (hook->mX - hook->mXo) * (double)partialTick;
    double hookY = hook->mYo + (hook->mY - hook->mYo) * (double)partialTick + 0.25;
    double hookZ = hook->mZo + (hook->mZ - hook->mZo) * (double)partialTick;
    double lineX = (double)(float)(handX - hookX);
    double lineY = (double)(float)(handY - hookY) + eyeOffset;
    double lineZ = (double)(float)(handZ - hookZ);

    GlStateManager::disableTexture();
    GlStateManager::disableLighting();
    builder->begin(5);
    builder->color(0);
    for (int i = 0; i <= 16; ++i)
    {
        float fraction = (float)i / 16.0f;
        builder->vertex((float)(x + lineX * (double)fraction),
                        (float)(y + lineY * (double)(fraction * fraction + fraction) * 0.5 + 0.25),
                        (float)(z + lineZ * (double)fraction));
    }
    tesselator->end();
    GlStateManager::enableLighting();
    GlStateManager::enableTexture();
    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* FishingHookRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &FISHING_HOOK_LOCATION;
}
