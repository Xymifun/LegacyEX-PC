#include "game/client/render/MobRenderer.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Culler.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"

MobRenderer::MobRenderer(EntityRenderDispatcher* dispatcher, Model* model, float shadowRadius)
    : LivingEntityRenderer(dispatcher, model, shadowRadius)
{
}

bool MobRenderer::shouldShowName(boost::shared_ptr<Entity>& entity)
{
    return LivingEntityRenderer::shouldShowName(entity)
           && (entity->shouldShowName() || boost::static_pointer_cast<Mob>(entity)->hasCustomName())
           && entity.get() == mDispatcher->mPlayer.get();
}

bool MobRenderer::shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double x, double y, double z)
{
    if (EntityRenderer::shouldRender(entity, culler, x, y, z))
        return true;
    boost::shared_ptr<Mob> mob = boost::static_pointer_cast<Mob>(entity);
    if (mob && mob->isLeashed() && mob->getLeashHolder())
    {
        boost::shared_ptr<Entity> leashHolder = mob->getLeashHolder();
        return culler->isVisible(leashHolder->getBoundingBoxForCulling());
    }
    return false;
}

void MobRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick)
{
    LivingEntityRenderer::render(entity, x, y, z, yaw, partialTick);
    if (!mSolidRender)
    {
        bool isMob = eTYPE_DERIVED_FROM(EntityType::MOB, entity->GetType());
        if (isMob)
        {
            boost::shared_ptr<Mob> mob = boost::static_pointer_cast<Mob>(entity);
            renderLeash(mob, x, y, z, yaw, partialTick);
        }
    }
}

void MobRenderer::setLightColor(boost::shared_ptr<Entity>& entity)
{
    int lightColor = entity->getLightColor();
    GlStateManager::multiTexCoord2f((float)(lightColor & 65535), (float)(lightColor >> 16));
}

double MobRenderer::lerp(double first, double second, double fraction)
{
    return (second - first) * fraction + first;
}

void MobRenderer::renderLeash(boost::shared_ptr<Mob>& mob, double x, double y, double z, float yaw, float partialTick)
{
    boost::shared_ptr<Entity> holder = mob->getLeashHolder();
    if (!holder)
        return;

    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    y -= (1.6 - (double)mob->mBbHeight) * 0.5;
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    double holderYaw = lerp((double)holder->mYRotO, (double)holder->mYRot, (double)(partialTick * 0.5F));
    double holderPitch = lerp((double)holder->mXRotO, (double)holder->mXRot, (double)(partialTick * 0.5F));
    double cosYaw = std::cos(holderYaw * Mth::DEG_TO_RAD);
    double sinYaw = std::sin(holderYaw * Mth::DEG_TO_RAD);
    double sinPitch = std::sin(holderPitch * Mth::DEG_TO_RAD);
    if (eTYPE_DERIVED_FROM(EntityType::HANGING_ENTITY, holder->GetType()))
    {
        cosYaw = 0.0;
        sinYaw = 0.0;
        sinPitch = -1.0;
    }
    double cosPitch = std::cos(holderPitch * Mth::DEG_TO_RAD);
    double holderX = lerp(holder->mXo, holder->mX, partialTick) - cosYaw * 0.7 - sinYaw * 0.5 * cosPitch;
    double holderY = lerp(holder->mYo + holder->getEyeHeight() * 0.7, holder->mY + holder->getEyeHeight() * 0.7,
                          partialTick)
                     - sinPitch * 0.5 - 0.25;
    double holderZ = lerp(holder->mZo, holder->mZ, partialTick) - sinYaw * 0.7 + cosYaw * 0.5 * cosPitch;

    double bodyYaw = lerp((double)mob->mYBodyRotO, (double)mob->mYBodyRot, partialTick) * Mth::DEG_TO_RAD
                     + 1.5707963705062866;
    double offsetX = std::cos(bodyYaw) * (double)mob->mBbWidth * 0.4;
    double offsetZ = std::sin(bodyYaw) * (double)mob->mBbWidth * 0.4;
    double mobX = lerp(mob->mXo, mob->mX, partialTick) + offsetX;
    double mobY = lerp(mob->mYo, mob->mY, partialTick);
    double mobZ = lerp(mob->mZo, mob->mZ, partialTick) + offsetZ;
    x += offsetX;
    z += offsetZ;
    double dx = (double)(float)(holderX - mobX);
    double dy = (double)(float)(holderY - mobY);
    double dz = (double)(float)(holderZ - mobZ);

    GlStateManager::disableTexture();
    GlStateManager::disableLighting();
    GlStateManager::disableCull();
    unsigned int lightColour = Minecraft::GetInstance()->getColourTable()->getColour(
        eMinecraftColour_Leash_Light_Colour);
    const float lightRed = (float)((lightColour >> 16) & 255) / 255.0F;
    const float lightGreen = (float)((double)((lightColour >> 8) & 255) / 255.0);
    const float lightBlue = (float)((double)(lightColour & 255) / 255.0);
    unsigned int darkColour = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Leash_Dark_Colour);
    const float darkBlue = (float)((double)(darkColour & 255) / 255.0);
    const float darkGreen = (float)((double)((darkColour >> 8) & 255) / 255.0);
    const float darkRed = (float)((darkColour >> 16) & 255) / 255.0F;

    const double width = 0.025;
    builder->begin(1);
    for (int i = 0; i <= 24; ++i)
    {
        const bool light = (i & 1) != 0;
        builder->color(light ? lightRed : darkRed, light ? lightGreen : darkGreen, light ? lightBlue : darkBlue, 1.0F);
        const float fraction = (float)i / 24.0F;
        const double px = x + dx * (double)fraction;
        const double py = y + dy * (double)(fraction * fraction + fraction) * 0.5
                          + (double)((24.0F - (float)i) / 18.0F + 0.125F);
        const double pz = z + dz * (double)fraction;
        builder->vertex((float)px, (float)py, (float)pz);
        builder->vertex((float)(px + width), (float)(py + width), (float)pz);
    }
    builder->end();

    builder->begin(1);
    for (int i = 0; i <= 24; ++i)
    {
        const bool light = (i & 1) != 0;
        builder->color(light ? lightRed : darkRed, light ? lightGreen : darkGreen, light ? lightBlue : darkBlue, 1.0F);
        const float fraction = (float)i / 24.0F;
        const double px = x + dx * (double)fraction;
        const double py = y + dy * (double)(fraction * fraction + fraction) * 0.5
                          + (double)((24.0F - (float)i) / 18.0F + 0.125F);
        const double pz = z + dz * (double)fraction;
        builder->vertex((float)px, (float)(py + width), (float)pz);
        builder->vertex((float)(px + width), (float)py, (float)(pz + width));
    }
    builder->end();
    GlStateManager::enableLighting();
    GlStateManager::enableTexture();
    GlStateManager::enableCull();
}
