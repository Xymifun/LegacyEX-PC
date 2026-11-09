#include "game/client/render/EnderChestRenderer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/EnderChestBlockEntity.hpp"

#include <boost/shared_ptr.hpp>

static ResourceLocation ENDER_CHEST_LOCATION(TN_ITEM_ENDERCHEST);

void EnderChestRenderer::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float a,
                                int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList)
{
    boost::shared_ptr<EnderChestBlockEntity> enderChest = boost::static_pointer_cast<EnderChestBlockEntity>(
        blockEntity);
    int data = 0;

    if (enderChest->hasLevel())
    {
        data = enderChest->getData();
    }

    if (breakProgress < 0)
    {
        bindTexture(&ENDER_CHEST_LOCATION);
        GlStateManager::pushMatrix();
        GlStateManager::enableRescaleNormal();
    }
    else
    {
        bindTexture(&BREAKING_LOCATIONS[breakProgress]);
        GlStateManager::matrixMode(2);
        GlStateManager::pushMatrix();
        GlStateManager::scalef(4.0F, 4.0F, 1.0F);
        GlStateManager::translatef(0.0625F, 0.0625F, 0.0625F);
        GlStateManager::matrixMode(0);
        GlStateManager::pushMatrix();
        GlStateManager::enableRescaleNormal();
    }

    if (bTranslucent)
    {
        GlStateManager::color4f(1.0F, 1.0F, 1.0F, alpha);
    }

    GlStateManager::translatef((float)x, (float)y + 1.0F, (float)z + 1.0F);
    GlStateManager::scalef(1.0F, -1.0F, -1.0F);
    GlStateManager::translatef(0.5F, 0.5F, 0.5F);

    if (data == 2)
    {
        GlStateManager::rotatef(180.0F, 0.0F, 1.0F, 0.0F);
        GlStateManager::translatef(-0.5F, -0.5F, -0.5F);
    }
    else if (data == 3)
    {
        GlStateManager::rotatef(0.0F, 0.0F, 1.0F, 0.0F);
        GlStateManager::translatef(-0.5F, -0.5F, -0.5F);
    }
    else if (data == 4)
    {
        GlStateManager::rotatef(90.0F, 0.0F, 1.0F, 0.0F);
        GlStateManager::translatef(-0.5F, -0.5F, -0.5F);
    }
    else
    {
        float yRot = 0.0F;
        if (data == 5)
        {
            yRot = -90.0F;
        }
        GlStateManager::rotatef(yRot, 0.0F, 1.0F, 0.0F);
        GlStateManager::translatef(-0.5F, -0.5F, -0.5F);
    }

    float openness = enderChest->mOpenOld + (enderChest->mOpen - enderChest->mOpenOld) * a;
    openness = 1.0F - openness;
    openness = 1.0F - openness * openness * openness;

    mChestModel.mLid->mXRot = -(openness * 1.5707964F);
    mChestModel.render(bUseDisplayList);

    GlStateManager::disableRescaleNormal();
    GlStateManager::popMatrix();

    if (bTranslucent)
    {
        GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    }

    if (breakProgress >= 0)
    {
        GlStateManager::matrixMode(2);
        GlStateManager::popMatrix();
        GlStateManager::matrixMode(0);
    }
}
