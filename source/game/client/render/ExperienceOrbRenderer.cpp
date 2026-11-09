#include "game/client/render/ExperienceOrbRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/ExperienceOrb.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Options.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation sExperienceOrbTexture(TN_ITEM_XPORB);

ExperienceOrbRenderer::ExperienceOrbRenderer(EntityRenderDispatcher* dispatcher) : EntityRenderer(dispatcher)
{
    mShadowRadius = 0.15F;
    mShadowStrength = 0.75F;
}

ExperienceOrbRenderer::~ExperienceOrbRenderer()
{
}

void ExperienceOrbRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                                   float partialTick)
{
    if (mSolidRender)
        return;

    boost::shared_ptr<ExperienceOrb> orb = boost::static_pointer_cast<ExperienceOrb>(entity);
    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x, (float)y, (float)z);

    int icon = (int)orb->getIcon();
    bindTexture(entity);
    Lighting::turnOn();

    int iconX = (icon % 4) * 16;
    int iconY = (icon / 4) * 16;
    float u0 = (float)iconX * 0.015625F;
    float u1 = (float)(iconX + 16) * 0.015625F;
    float v0 = (float)iconY * 0.015625F;
    float v1 = (float)(iconY + 16) * 0.015625F;

    int brightness = orb->getLightColor();
    GlStateManager::multiTexCoord2f((float)(brightness % 65536), (float)(brightness / 65536));
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);

    float br = 255.0F;
    float angle = ((float)orb->mTickCount + partialTick) / 2.0F;
    int red = (int)((Mth::sin(angle + 0.0F) + 1.0F) * 0.5F * br);
    int green = 255;
    int blue = (int)((Mth::sin(angle + 4.1887903F) + 1.0F) * 0.1F * br);
    int color = red << 16 | green << 8 | blue;
    float scale = 0.3F;
    GlStateManager::translatef(0.0F, 0.1F, 0.0F);
    GlStateManager::rotatef(180.0F - mDispatcher->mCameraYRot, 0.0F, 1.0F, 0.0F);
    float pitchSign = mDispatcher->mOptions->mThirdPersonView == 2 ? -1.0F : 1.0F;
    GlStateManager::rotatef(-(pitchSign * mDispatcher->mCameraXRot), 1.0F, 0.0F, 0.0F);
    GlStateManager::scalef(scale, scale, scale);

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->begin();
    builder->color(color, 128);
    builder->normal(0.0F, 1.0F, 0.0F);
    builder->vertexUV(-0.5F, -0.25F, 0.0F, u0, v1);
    builder->vertexUV(0.5F, -0.25F, 0.0F, u1, v1);
    builder->vertexUV(0.5F, 0.75F, 0.0F, u1, v0);
    builder->vertexUV(-0.5F, 0.75F, 0.0F, u0, v0);
    builder->end();

    GlStateManager::disableBlend();
    GlStateManager::disableRescaleNormal();
    GlStateManager::popMatrix();
    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* ExperienceOrbRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &sExperienceOrbTexture;
}
