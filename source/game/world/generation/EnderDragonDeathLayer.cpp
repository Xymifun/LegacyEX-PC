#include "game/world/generation/EnderDragonDeathLayer.hpp"

#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/entity/mob/EnderDragon.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/Random.hpp"

EnderDragonDeathLayer::~EnderDragonDeathLayer()
{
}

void EnderDragonDeathLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                   float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                                   bool bUseDisplayList)
{
    boost::shared_ptr<EnderDragon> pDragon = boost::dynamic_pointer_cast<EnderDragon>(entity);

    if (pDragon->mDeathTime <= 0)
    {
        return;
    }

    BufferBuilder* pBuilder = Tesselator::getInstance()->getBuilder();

    Lighting::turnOff();

    float deathProgress = ((float)pDragon->mDeathTime + partialTicks) / 200.0f;
    float fadeProgress = 0.0f;

    if (deathProgress > 0.8f)
    {
        fadeProgress = (deathProgress - 0.8f) / 0.2f;
    }

    Random random(432);

    GlStateManager::disableTexture();
    GlStateManager::shadeModel(0);
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 1);
    GlStateManager::disableAlphaTest();
    GlStateManager::enableCull();
    GlStateManager::depthMask(false);
    GlStateManager::pushMatrix();
    GlStateManager::translatef(0.0f, -1.0f, -2.0f);

    for (int i = 0; (float)i < (deathProgress + deathProgress * deathProgress) / 2.0f * 60.0f; ++i)
    {
        int alpha = (int)(255.0f * (1.0f - fadeProgress));

        GlStateManager::rotatef(random.nextFloat() * 360.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::rotatef(random.nextFloat() * 360.0f, 0.0f, 1.0f, 0.0f);
        GlStateManager::rotatef(random.nextFloat() * 360.0f, 0.0f, 0.0f, 1.0f);
        GlStateManager::rotatef(random.nextFloat() * 360.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::rotatef(random.nextFloat() * 360.0f, 0.0f, 1.0f, 0.0f);
        GlStateManager::rotatef(random.nextFloat() * 360.0f + deathProgress * 90.0f, 0.0f, 0.0f, 1.0f);

        float rayHeight = random.nextFloat() * 20.0f + 5.0f + fadeProgress * 10.0f;
        float rayWidth = random.nextFloat() * 2.0f + 1.0f + fadeProgress * 2.0f;

        pBuilder->begin(2);
        pBuilder->color(0xFFFFFF, alpha);
        pBuilder->vertex(0.0f, 0.0f, 0.0f);
        pBuilder->color(0xFF00FF, 0);
        pBuilder->vertex(-0.866 * rayWidth, rayHeight, -0.5f * rayWidth);
        pBuilder->vertex(0.866 * rayWidth, rayHeight, -0.5f * rayWidth);
        pBuilder->vertex(0.0f, rayHeight, rayWidth);
        pBuilder->vertex(-0.866 * rayWidth, rayHeight, -0.5f * rayWidth);
        pBuilder->end();
    }

    GlStateManager::popMatrix();
    GlStateManager::depthMask(true);
    GlStateManager::disableCull();
    GlStateManager::disableBlend();
    GlStateManager::shadeModel(0);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::enableTexture();
    GlStateManager::enableAlphaTest();
    Lighting::turnOn();
}

bool EnderDragonDeathLayer::colorsOnDamage()
{
    return false;
}
